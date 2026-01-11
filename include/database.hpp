/*
	Utility library for Dreamcast game servers.
    Copyright (C) 2026  Flyinghead

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include <sqlite3.h>
#include <stdexcept>
#include <string>
#include <vector>
#include <cstring>

class UniqueConstraintViolation : public std::runtime_error
{
public:
	UniqueConstraintViolation() : std::runtime_error("Unique constraint violation") {}
};

[[noreturn]] inline static void throwSqlError(sqlite3 *db)
{
	if (db == nullptr)
		throw std::runtime_error("Database is closed");
	if (sqlite3_extended_errcode(db) == SQLITE_CONSTRAINT_UNIQUE)
		throw UniqueConstraintViolation();
	const char *msg = sqlite3_errmsg(db);
	if (msg != nullptr)
		throw std::runtime_error(msg);
	else
		throw std::runtime_error("SQL Error");
}

class Database
{
public:
	Database() = default;

	Database(const std::string& path) {
		open(path);
	}
	Database(const Database&) = delete;
	Database& operator=(const Database&) = delete;

	~Database() {
		close();
	}

	void open(const std::string& path)
	{
		close();
		if (sqlite3_open(path.c_str(), &db) != SQLITE_OK)
		{
			std::string what = "Can't open database " + path + std::string(": ") + sqlite3_errmsg(db);
			sqlite3_close(db);
			throw std::runtime_error(what);
		}
		sqlite3_busy_timeout(db, 1000);
	}

	void close()
	{
		if (db != nullptr) {
			sqlite3_close(db);
			db = nullptr;
		}
	}

	void exec(const std::string& stmt)
	{
		if (db == nullptr)
			throwSqlError(db);
		if (sqlite3_exec(db, stmt.c_str(), nullptr, 0, nullptr) != SQLITE_OK)
			throwSqlError(db);
	}

public:
	sqlite3 *db = nullptr;
	friend class Statement;
};

class Statement
{
public:
	Statement(Database& database, const char *sql) : db(database.db)
	{
		if (db == nullptr)
			throwSqlError(db);
		if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK)
			throwSqlError(db);
	}
	Statement(const Statement&) = delete;
	Statement& operator=(const Statement&) = delete;

	~Statement() {
		if (stmt != nullptr)
			sqlite3_finalize(stmt);
	}

	void bind(int idx, int v)
	{
		checkReset();
		if (sqlite3_bind_int(stmt, idx, v) != SQLITE_OK)
			throwSqlError(db);
	}
	void bind(int idx, const std::string& s)
	{
		checkReset();
		if (sqlite3_bind_text(stmt, idx, s.c_str(), s.length(), SQLITE_TRANSIENT) != SQLITE_OK)
			throwSqlError(db);
	}
	void bind(int idx, const uint8_t *data, size_t len)
	{
		checkReset();
		if (sqlite3_bind_blob(stmt, idx, data, len, SQLITE_STATIC) != SQLITE_OK)
			throwSqlError(db);
	}

	bool step()
	{
		executed = true;
		int rc = sqlite3_step(stmt);
		if (rc == SQLITE_ROW)
			return true;
		if (rc != SQLITE_DONE && rc != SQLITE_OK)
			throwSqlError(db);
		return false;
	}

	int getIntColumn(int idx) {
		return sqlite3_column_int(stmt, idx);
	}
	std::string getStringColumn(int idx) {
		return std::string((const char *)sqlite3_column_text(stmt, idx));
	}
	std::vector<uint8_t> getBlobColumn(int idx)
	{
		std::vector<uint8_t> blob;
		blob.resize(sqlite3_column_bytes(stmt, idx));
		if (!blob.empty())
			memcpy(blob.data(), sqlite3_column_blob(stmt, idx), blob.size());
		return blob;
	}

	int changedRows()
	{
		if (!executed)
			return 0;
		else
			return sqlite3_changes(db);
	}

private:
	void checkReset()
	{
		if (executed) {
			executed = false;
			sqlite3_reset(stmt);
		}
	}
	sqlite3 *db;
	sqlite3_stmt *stmt = nullptr;
	bool executed = false;
};

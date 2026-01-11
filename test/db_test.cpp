#include "gtest/gtest.h"
#include <unistd.h>
#include "../include/database.hpp"

class DatabaseTest : public ::testing::Test {
protected:
	void createDb()
	{
		unlink("test.db");
		Database db("test.db");
		db.exec("CREATE TABLE TEST (ID INTEGER UNIQUE, NAME VARCHAR, DATA BLOB)");
	}
	void insertTestData()
	{
		createDb();
		Database db("test.db");
		Statement stmt(db,
			"INSERT INTO TEST (ID, NAME, DATA) VALUES (?, ?, ?)");
		stmt.bind(1, 1);
		std::string name{ "Alice" };
		stmt.bind(2, name);
		stmt.bind(3, (const uint8_t *)&name[0], name.size());
		stmt.step();
		stmt.bind(1, 2);
		name = "Bob";
		stmt.bind(2, name);
		stmt.bind(3, (const uint8_t *)&name[0], name.size());
		stmt.step();
		stmt.bind(1, 3);
		name = "Charlie";
		stmt.bind(2, name);
		stmt.bind(3, (const uint8_t *)&name[0], name.size());
		stmt.step();
	}
};

TEST_F(DatabaseTest, databaseNotFound)
{
	ASSERT_THROW(Database("/a/b/c/notfound.db"), std::runtime_error);
}

TEST_F(DatabaseTest, databaseClosed)
{
	Database db;
	ASSERT_THROW(db.exec("DELETE FROM WHATEVER"), std::runtime_error);
	ASSERT_THROW(Statement(db, "SELECT * FROM WHATEVER"), std::runtime_error);
}

TEST_F(DatabaseTest, exec)
{
	createDb();
}

TEST_F(DatabaseTest, statement)
{
	insertTestData();
}

TEST_F(DatabaseTest, getColumn)
{
	insertTestData();
	Database db("test.db");
	{
		Statement stmt(db, "SELECT COUNT(*) FROM TEST");
		ASSERT_TRUE(stmt.step());
		ASSERT_EQ(3, stmt.getIntColumn(0));
		ASSERT_FALSE(stmt.step());
	}
	{
		Statement stmt(db, "SELECT * FROM TEST WHERE NAME = ?");
		stmt.bind(1, "Alice");
		ASSERT_TRUE(stmt.step());
		ASSERT_EQ(1, stmt.getIntColumn(0));
		ASSERT_EQ("Alice", stmt.getStringColumn(1));
		auto data = stmt.getBlobColumn(2);
		ASSERT_EQ(5, data.size());
		ASSERT_TRUE(!memcmp(data.data(), "Alice", 5));
		ASSERT_FALSE(stmt.step());
	}
}

TEST_F(DatabaseTest, select)
{
	insertTestData();
	Database db("test.db");
	Statement stmt(db, "SELECT NAME FROM TEST ORDER BY NAME DESC");
	ASSERT_TRUE(stmt.step());
	ASSERT_EQ("Charlie", stmt.getStringColumn(0));
	ASSERT_TRUE(stmt.step());
	ASSERT_EQ("Bob", stmt.getStringColumn(0));
	ASSERT_TRUE(stmt.step());
	ASSERT_EQ("Alice", stmt.getStringColumn(0));
	ASSERT_FALSE(stmt.step());
}

TEST_F(DatabaseTest, unique)
{
	insertTestData();
	Database db("test.db");
	ASSERT_THROW(db.exec("INSERT INTO TEST (ID, NAME) VALUES (1, 'Nobody')"), UniqueConstraintViolation);
}

TEST_F(DatabaseTest, changedRows)
{
	insertTestData();
	Database db("test.db");
	Statement stmt(db, "UPDATE TEST SET NAME = 'unknown' WHERE NAME <= 'Bob'");
	stmt.step();
	ASSERT_EQ(2, stmt.changedRows());
}

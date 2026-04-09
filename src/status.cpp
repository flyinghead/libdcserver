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
#include "status.h"
#include "status.hpp"
#include "json.hpp"
#include "internal.h"
#include <string>
#include <string_view>
#include <stdio.h>
#include <time.h>
#include <fstream>
#include <stdexcept>
#include <memory>
#include <pthread.h>

#ifndef STATUSDIR
#define STATUSDIR "/var/local/lib/dcnet/status/"
#endif
#ifndef CONFDIR
#define CONFDIR "/usr/local/etc/dcnet"
#endif
#define CONF_FILE CONFDIR "/status.conf"

static bool initialized;
static std::string statusUrl;
static std::string legacyStatusUrl;
static std::string statusDir;
static int updateInterval = 5 * 60; // default 5 min
static nlohmann::json statusArray;
static std::unique_ptr<WorkerThread> worker;

static nlohmann::json jsonStatus(std::string_view gameId, int playerCount, int gameCount)
{
	nlohmann::json status = {
		{ "gameId", gameId },
		{ "timestamp", time(nullptr) },
	};
	if (playerCount >= 0)
		status["playerCount"] = playerCount;
	if (gameCount >= 0)
		status["gameCount"] = gameCount;
	return status;
}

static void prefork() {
	worker.reset();
}

static void init()
{
	if (initialized)
		return;
	initialized = true;
	// mutexes and condition variables don't like forking
	pthread_atfork(prefork, nullptr, nullptr);

	std::ifstream ifs(CONF_FILE);
	if (ifs.fail())
		return;
	Config config = loadConfig(ifs);
	if (config.count("status-url") != 0)
		legacyStatusUrl = config["status-url"][0];
	if (config.count("mgmt-status-url") != 0)
		statusUrl = config["mgmt-status-url"][0];
	if (config.count("update-interval") != 0)
	{
		int v = atoi(config["update-interval"][0].c_str());
		if (v != 0)
			updateInterval = v;
	}
	if (config.count("status-dir") != 0)
		statusDir = config["status-dir"][0];
	if (statusDir.empty())
		statusDir = STATUSDIR;
	if (statusDir.back() != '/')
		statusDir += '/';
}

//
// Legacy status
//
void statusUpdate(std::string_view gameId, int playerCount, int gameCount)
{
	init();
	nlohmann::json json = jsonStatus(gameId, playerCount, gameCount);
	statusArray.push_back(json);
}

void statusCommit(std::string_view serverId)
{
	if (statusArray.empty())
		return;
	std::string jsonstr = statusArray.dump(4);
	if (!legacyStatusUrl.empty()) {
		Http().post(legacyStatusUrl + '/' + std::string(serverId), jsonstr, "application/json");
	}
	else
	{
		std::string path = statusDir + std::string(serverId);
		FILE *f = fopen(path.c_str(), "w");
		if (f == nullptr) {
			perror(path.c_str());
			return;
		}
		fwrite(jsonstr.c_str(), 1, jsonstr.length(), f);
		fclose(f);
	}
	statusArray.clear();
}

extern "C"
{

int statusGetInterval() {
	return updateInterval;
}

int statusUpdate(const char *gameId, int playerCount, int gameCount)
{
	try {
		statusUpdate(std::string_view(gameId), playerCount, gameCount);
		return 0;
	} catch (const std::exception& e) {
		fprintf(stderr, "statusUpdate: %s\n", e.what());
	} catch (...) {
		fprintf(stderr, "statusUpdate: unknown error\n");
	}
	return -1;
}

int statusCommit(const char *serverId)
{
	try {
		statusCommit(std::string_view(serverId));
		return 0;
	} catch (const std::exception& e) {
		fprintf(stderr, "statusUpdate: %s\n", e.what());
	} catch (...) {
		fprintf(stderr, "statusUpdate: unknown error\n");
	}
	return -1;
}

} // extern "C"

//
// New status
//
namespace status
{

static void httpPost(std::string url, std::string payload)
{
	if (worker == nullptr)
		worker = std::make_unique<WorkerThread>();
	worker->run([url, payload]() {
		Http().post(url, payload, "application/json");
	});
}

static void postId(const std::string& url, std::string_view id)
{
	nlohmann::json payload = { { "id", id } };
	std::string jsonstr = payload.dump(4);
	httpPost(url, jsonstr);
}

void reset(std::string_view serverId)
{
	init();
	if (statusUrl.empty())
		return;
	postId(statusUrl + "/game/reset", serverId);
}

void ping(std::string_view serverId)
{
	init();
	if (statusUrl.empty())
		return;
	postId(statusUrl + "/game/ping", serverId);
}

static void joinLeave(const std::string& url, std::string_view gameId, std::string_view ip, int port, std::string_view playerName)
{
	nlohmann::json payload = {
		{ "id", gameId },
		{ "ip", ip },
		{ "port", port },
	};
	if (!playerName.empty())
		payload["name"] = playerName;
	std::string jsonstr = payload.dump(4);
	httpPost(url, jsonstr);
}

void join(std::string_view gameId, std::string_view ip, int port, std::string_view playerName)
{
	init();
	if (statusUrl.empty())
		return;
	joinLeave(statusUrl + "/game/join", gameId, ip, port, playerName);
}

void leave(std::string_view gameId, std::string_view ip, int port, std::string_view playerName)
{
	init();
	if (statusUrl.empty())
		return;
	joinLeave(statusUrl + "/game/leave", gameId, ip, port, playerName);
}

void createGame(std::string_view gameId)
{
	init();
	if (statusUrl.empty())
		return;
	postId(statusUrl + "/game/creategame", gameId);
}

void deleteGame(std::string_view gameId)
{
	init();
	if (statusUrl.empty())
		return;
	postId(statusUrl + "/game/deletegame", gameId);
}

int pingInterval() {
	init();
	return updateInterval;
}

}

extern "C"
{

void statusReset(const char *serverId) {
	status::reset(serverId);
}
void statusPing(const char *serverId) {
	status::ping(serverId);
}
void statusJoin(const char *gameId, const char *ip, int port, const char *playerName) {
	status::join(gameId, ip, port, playerName == nullptr ? "" : playerName);
}
void statusLeave(const char *gameId, const char *ip, int port, const char *playerName) {
	status::leave(gameId, ip, port, playerName == nullptr ? "" : playerName);
}
void statusCreateGame(const char *gameId) {
	status::createGame(gameId);
}
void statusDeleteGame(const char *gameId) {
	status::deleteGame(gameId);
}
int statusPingInterval() {
	return status::pingInterval();
}

}

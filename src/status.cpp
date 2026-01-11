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

#ifndef STATUS_PATH
#define STATUS_PATH "/var/lib/dcnet/status/"
#endif
#ifndef CONFDIR
#define CONFDIR "/usr/local/etc/dcnet"
#endif
#define CONF_FILE CONFDIR "/status.conf"

static bool initialized;
static std::string statusUrl;
static int updateInterval = 5 * 60; // default 5 min
static nlohmann::json statusArray;

static nlohmann::json jsonStatus(std::string_view gameId, int playerCount, int gameCount)
{
	nlohmann::json status = {
		{ "gameId", gameId },
		{ "timestamp", time(nullptr) },
		{ "playerCount", playerCount },
		{ "gameCount", gameCount },
	};
	return status;
}

static void init()
{
	if (initialized)
		return;
	initialized = true;
	std::ifstream ifs(CONF_FILE);
	if (ifs.fail())
		return;
	Config config = loadConfig(ifs);
	if (config.count("status-url") != 0)
		statusUrl = config["status-url"][0];
	if (config.count("update-interval") != 0)
	{
		int v = atoi(config["update-interval"][0].c_str());
		if (v != 0)
			updateInterval = v;
	}
}

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
	if (!statusUrl.empty()) {
		Http().post(statusUrl + '/' + std::string(serverId), jsonstr, "application/json");
	}
	else
	{
		std::string path = STATUS_PATH + std::string(serverId);
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

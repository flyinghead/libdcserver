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
#include "json.hpp"
#include <string>
#include <stdio.h>
#include <time.h>

#ifndef STATUS_PATH
#define STATUS_PATH "/var/lib/dcnet/status/"
#endif

struct GameStatus
{
	std::string gameId;
};

std::string jsonStatus(const std::string& gameId, int playerCount, int gameCount)
{
	nlohmann::json status = {
		{ "gameId", gameId },
		{ "timestamp", time(nullptr) },
		{ "playerCount", playerCount },
		{ "gameCount", gameCount },
	};
	return status.dump(4);
}

extern "C"
{

Handle statusInit(const char *gameId) {
	return new GameStatus { gameId };
}

void statusTerm(Handle handle) {
	GameStatus *gs = (GameStatus *)handle;
	delete gs;
}

void statusUpdate(Handle handle, int playerCount, int gameCount)
{
	const char *names[] = { "players", "games", nullptr };
	std::string splayers = std::to_string(playerCount);
	std::string sgames = std::to_string(gameCount);
	const char *values[] = { splayers.c_str(), sgames.c_str(), nullptr };
	statusUpdateEx(handle, names, values);
}

void statusUpdateEx(Handle handle, const char *names[], const char *values[])
{
	GameStatus *gs = (GameStatus *)handle;
	std::string path = STATUS_PATH + gs->gameId;
	FILE *f = fopen(path.c_str(), "w");
	if (f == nullptr) {
		perror(path.c_str());
		return;
	}
	fprintf(f, "timestamp=%ld\n", time(nullptr));
	while (*names != nullptr)
		fprintf(f, "%s=%s\n", *names++, *values++);
	fclose(f);
}

} // extern "C"

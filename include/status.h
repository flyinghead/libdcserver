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
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

int statusGetInterval();
int statusUpdate(const char *gameId, int playerCount, int gameCount);
int statusCommit(const char *serverId);

void statusReset(const char *serverId);
void statusPing(const char *serverId);
void statusJoin(const char *gameId, const char *ip, int port, const char *playerName);
void statusLeave(const char *gameId, const char *ip, int port, const char *playerName);
void statusCreateGame(const char *gameId);
void statusDeleteGame(const char *gameId);
int statusPingInterval();

#ifdef __cplusplus
}
#endif

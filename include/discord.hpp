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
#include <string>
#include <string_view>
#include <stdexcept>

struct Notif
{
	std::string content;
	struct {
		std::string title;
		std::string text;
	} embed;
};

class DiscordException : public std::runtime_error
{
public:
	DiscordException(const std::string& what)
		: std::runtime_error(what)
	{}
	DiscordException(const char *what)
		: std::runtime_error(what)
	{}
};

void discordSetWebhook(const std::string& url);
void discordNotif(const std::string& gameId, const Notif& notif);
std::string discordEscape(std::string_view str);

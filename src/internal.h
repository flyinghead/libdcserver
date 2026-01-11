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
#include <istream>
#include <string>
#include <string_view>
#include <map>
#include <vector>

using Config = std::map<std::string, std::vector<std::string>>;
Config loadConfig(std::istream& stream);

class Http
{
public:
	Http();
	~Http();
	void post(const std::string& url, std::string_view body, std::string_view contentType);

private:
	using CURL = void;
	CURL *curl = nullptr;
};

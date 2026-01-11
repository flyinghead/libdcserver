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
#include "internal.h"
#include <sstream>
#include <stdio.h>
#include <cctype>

Config loadConfig(std::istream& stream)
{
	Config config;

	std::string line;
	while (std::getline(stream, line))
	{
		std::istringstream istr(line);
		// Get key
		istr >> std::ws;
		std::string key;
		if (!std::getline(istr, key, '='))
			continue;
		if (key[0] == '#' || key[0] == ';')
			// comment
			continue;
		while (!key.empty() && std::isspace(key.back()))
			key.pop_back();

		// Get value
		istr >> std::ws;
		std::string value;
		if (!std::getline(istr, value))
			continue;
		while (!value.empty() && std::isspace(value.back()))
			value.pop_back();

		std::vector<std::string> values;
		std::istringstream ivalues(value);
		std::string item;
		while (std::getline(ivalues, item, ','))
		{
			while (!item.empty() && std::isspace(item.back()))
				item.pop_back();
			if (!item.empty())
				values.push_back(item);
			ivalues >> std::ws;
		}
		config[key] = values;
	}
	return config;
}

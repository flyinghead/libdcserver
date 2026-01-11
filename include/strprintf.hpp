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
#include <cstdio>

template<typename ... Args>
std::string strprintf(const char *format, Args ... args)
{
	int size = std::snprintf(nullptr, 0, format, args...);
	std::string out(size + 1, '\0');
	std::snprintf(out.data(), size + 1, format, args...);
	out.resize(size);
	return out;
}

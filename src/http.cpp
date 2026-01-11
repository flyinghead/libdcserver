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
#include "strprintf.hpp"
#include <string>
#include <string_view>
#include <stdexcept>
#include <curl/curl.h>

Http::Http()
{
	curl = curl_easy_init();
	if (curl == nullptr)
		throw std::runtime_error("can't create curl handle");
}

void Http::post(const std::string& url, std::string_view body, std::string_view contentType)
{
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "DCNet-DiscordWebhook");
	curl_slist *headers = nullptr;
	if (!contentType.empty()) {
		std::string ctype = "Content-Type: " + std::string(contentType);
		headers = curl_slist_append(headers, ctype.c_str());
	}
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.data());
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body.size());

	CURLcode res = curl_easy_perform(curl);
	curl_slist_free_all(headers);
	if (res != CURLE_OK)
		throw std::runtime_error(strprintf("curl error: %d", res));

	long code;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
	if (code < 200 || code >= 300)
		throw std::runtime_error(strprintf("HTTP error %ld", code));
}

Http::~Http() {
	curl_easy_cleanup(curl);
}

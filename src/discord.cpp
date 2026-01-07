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
#include "discord.hpp"
#include "strprintf.hpp"
#include "json.hpp"
#include <curl/curl.h>
#include <atomic>
#include <thread>
#include <chrono>
#include <fstream>
#include <stdio.h>
#include <set>

#ifndef DATADIR
#define DATADIR "/usr/local/share/dcnet"
#endif
#ifndef CONFDIR
#define CONFDIR "/usr/local/etc/dcnet"
#endif
#define GAMES_FILE DATADIR "/games.json"
#define CONF_FILE CONFDIR "/discord.conf"

using namespace nlohmann;
static constexpr int MAX_THREADS = 5;

static std::string webhook;
static std::atomic_int threadCount;
static json games;
static std::set<std::string> disabledGames;
static bool initialized;

static void init()
{
	if (initialized)
		return;
	initialized = true;
	std::ifstream ifs(GAMES_FILE);
	if (ifs.fail())
		throw DiscordException("Can't open " GAMES_FILE);
	games = json::parse(ifs);

	std::ifstream ifs2(CONF_FILE);
	if (ifs2.fail())
		throw DiscordException("Can't open " CONF_FILE);
	std::string line;
	while (std::getline(ifs2, line))
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

		if (key == "webhook") {
			webhook = value;
		}
		else if (key == "disabled-games")
		{
			std::istringstream ivalues(value);
			std::string gameId;
			while (std::getline(ivalues, gameId, ','))
			{
				while (!gameId.empty() && std::isspace(gameId.back()))
					gameId.pop_back();
				if (!gameId.empty())
					disabledGames.insert(gameId);
				ivalues >> std::ws;
			}
		}
		else {
			fprintf(stderr, "Unrecognized key in discord.conf: %s\n", key.c_str());
		}
	}
}

static void postWebhook(std::string body)
{
	CURL *curl = curl_easy_init();
	if (curl == nullptr) {
		threadCount.fetch_sub(1);
		fprintf(stderr, "Discord: can't create curl handle\n");
		return;
	}
	CURLcode res;
	curl_easy_setopt(curl, CURLOPT_URL, webhook.c_str());
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "DCNet-DiscordWebhook");
	curl_slist *headers = curl_slist_append(NULL, "Content-Type: application/json");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());

	res = curl_easy_perform(curl);
	if (res != CURLE_OK) {
		fprintf(stderr, "Discord: curl error: %d\n", res);
	}
	else
	{
		long code;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
		if (code < 200 || code >= 300)
			fprintf(stderr, "Discord: HTTP error %ld\n", code);
	}
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);
	threadCount.fetch_sub(1);
}


void discordNotif(const std::string& gameId, const Notif& notif)
{
	init();
	if (webhook.empty() || disabledGames.count(gameId) != 0)
		return;
	if (threadCount.fetch_add(1) >= MAX_THREADS) {
		threadCount.fetch_sub(1);
		throw DiscordException("Discord max thread count reached");
	}
	std::string gameName;
	std::string gamePic;
	try {
		json g = games.at(gameId).get<json>();
		gameName = g.at("name").get<std::string>();
		gamePic = g.at("thumbnail").get<std::string>();
	} catch (const json::exception& e) {
		gameName = gameId;
		gamePic = "https://dcnet.flyca.st/gamepic/unknown.jpg";
	}

	json embeds;
	embeds.push_back({
		{ "author",
			{
				{ "name", gameName },
				{ "icon_url", gamePic }
			},
		},
		{ "title", notif.embed.title },
		{ "description", notif.embed.text },
		{ "color", 9118205 },
	});
	json jnotif = {
		{ "content", notif.content },
		{ "embeds", embeds },
	};

	std::thread thread(postWebhook, jnotif.dump(4, ' ', false, json::error_handler_t::replace));
	thread.detach();
}

void discordSetWebhook(std::string_view) {
	init();
}

std::string discordEscape(std::string_view str)
{
	std::string ret;
	for (char c : str)
	{
		if (c == '*' || c == '_' || c == '`' || c == '~' || c == '<'
				|| c == '>' || c == ':' || c == '[' || c == '\\')
			ret += '\\';
		ret += c;
	}
	return ret;
}

//
// C bindings
//
extern "C"
{

void discordSetWebhook(const char *) {
	try {
		init();
	} catch (const std::exception& e) {
		fprintf(stderr, "Discord initialization failed: %s\n", e.what());
	}
}

int discordNotif(const char *gameId, const char *content, const char *embedTitle, const char *embedText)
{
	Notif notif;
	notif.content = content;
	notif.embed.title = embedTitle;
	notif.embed.text = embedText;
	try {
		discordNotif(gameId, notif);
		return 0;
	} catch (const std::exception& e) {
		fprintf(stderr, "discord error: %s\n", e.what());
		return -1;
	}
}

char *discordEscape(const char *s)
{
	std::string ret = discordEscape(std::string_view{ s });
	char *r = (char *)malloc(ret.length() + 1);
	strcpy(r, ret.c_str());
	return r;
}

}

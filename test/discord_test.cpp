#include "gtest/gtest.h"
#include "../include/discord.hpp"

void discordForceWebhook(std::string_view url);

class DiscordTest : public ::testing::Test {
protected:
};

TEST_F(DiscordTest, escape)
{
	ASSERT_EQ("", discordEscape(""));
	ASSERT_EQ("same", discordEscape("same"));
	ASSERT_EQ("\\*\\_\\`\\~", discordEscape("*_`~"));
	ASSERT_EQ("\\<\\>\\:\\[\\\\", discordEscape("<>:[\\"));
	ASSERT_EQ("+\\*a\\>b\\_c\\:d", discordEscape("+*a>b_c:d"));
}

TEST_F(DiscordTest, notification)
{
	const char *webhook = getenv("DISCORD_WEBHOOK");
	if (webhook == nullptr) {
		fprintf(stderr, "*** DISCORD_WEBHOOK env variable not defined. Skipping test ***\n");
		return;
	}
	discordForceWebhook(webhook);
	Notif notif;
	notif.content = "This is a unit test. *Please ignore*";
	notif.embed.title = "Title";
	notif.embed.text = "Lorem ipsum dolor sit amet";
	discordNotif("oogabooga", notif);
	sleep(1); // let the thread finish
}

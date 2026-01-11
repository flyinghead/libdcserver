#include "gtest/gtest.h"
#include "../src/internal.h"
#include <sstream>

class ConfigTest : public ::testing::Test {
protected:
	Config load(const std::string& s) {
		std::stringstream stream(s);
		return loadConfig(stream);
	}
};

TEST_F(ConfigTest, testEmpty)
{
	ASSERT_TRUE(load("").empty());
}

TEST_F(ConfigTest, testComment)
{
	ASSERT_TRUE(load("; whatever=this\n #whatever = that\n").empty());
}

TEST_F(ConfigTest, testWhitespace)
{
	Config cfg = load("a1=b1\n"
			" a2= b2\n"
			"a3 =b3 \n"
			" a4 = b4 \n"
			"\ta5\t\t=\tb5\t\n");
	ASSERT_EQ("b1", cfg["a1"][0]);
	ASSERT_EQ("b2", cfg["a2"][0]);
	ASSERT_EQ("b3", cfg["a3"][0]);
	ASSERT_EQ("b4", cfg["a4"][0]);
	ASSERT_EQ("b5", cfg["a5"][0]);
}

TEST_F(ConfigTest, testMultiValue)
{
	Config cfg = load("a1=b1,c1,d1\n"
			"a2=b2 ,c2 ,d2 \n"
			"a3= b3, c3, d3\n"
			"a4= b4 , c4 , d4 \n"
			"a5=\tb5\t,\tc5,\td5\t\n");
	ASSERT_EQ("b1", cfg["a1"][0]);
	ASSERT_EQ("c1", cfg["a1"][1]);
	ASSERT_EQ("d1", cfg["a1"][2]);
	ASSERT_EQ("b2", cfg["a2"][0]);
	ASSERT_EQ("c2", cfg["a2"][1]);
	ASSERT_EQ("d2", cfg["a2"][2]);
	ASSERT_EQ("b3", cfg["a3"][0]);
	ASSERT_EQ("c3", cfg["a3"][1]);
	ASSERT_EQ("d3", cfg["a3"][2]);
	ASSERT_EQ("b4", cfg["a4"][0]);
	ASSERT_EQ("c4", cfg["a4"][1]);
	ASSERT_EQ("d4", cfg["a4"][2]);
	ASSERT_EQ("b5", cfg["a5"][0]);
	ASSERT_EQ("c5", cfg["a5"][1]);
	ASSERT_EQ("d5", cfg["a5"][2]);
}


TEST_F(ConfigTest, testFull)
{
	Config cfg = load("# some comment\n"
			"a1=b1\n"
			";a1=b2\n"
			"a2 = b2, c2\n"
			"# some other comment\n"
			"property42=value42\n"
			"a3 =  b3");
	ASSERT_EQ("b1", cfg["a1"][0]);
	ASSERT_EQ("b2", cfg["a2"][0]);
	ASSERT_EQ("c2", cfg["a2"][1]);
	ASSERT_EQ("value42", cfg["property42"][0]);
	ASSERT_EQ("b3", cfg["a3"][0]);
}

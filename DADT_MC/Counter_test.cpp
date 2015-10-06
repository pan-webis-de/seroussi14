/*
 * Counter_test.cpp
 *
 *  Created on: Sep 17, 2015
 *      Author: thomas
 */

#include "Counter.h"

#include "gtest/gtest.h"


TEST(DynCounterTest, CanInitialize) {
	Dynamic_Counter c;
	EXPECT_EQ(0, c.sum());
}

TEST(DynCounterTest, Insert) {
	Dynamic_Counter c;
	c.inc(2);
	EXPECT_EQ(1, c.sum());
	EXPECT_EQ(1, c.at(2));
}

TEST(DynCounterTest, Increment) {
	Dynamic_Counter c;
	c.inc(10);
	c.inc(2);
	c.inc(2);
	EXPECT_EQ(3, c.sum());
	EXPECT_EQ(2, c.at(2));
}

TEST(DynCounterTest, Decrement) {
	Dynamic_Counter c;
	c.inc(10);
	c.inc(2);
	c.inc(2);
	c.dec(2);
	EXPECT_EQ(2, c.sum());
	EXPECT_EQ(1, c.at(2));
}


TEST(StatCounterTest, CanInitialize) {
	Static_Counter c(20);
	EXPECT_EQ(0, c.sum());
}

TEST(StatCounterTest, Insert) {
	Static_Counter c(20);
	c.inc(2);
	EXPECT_EQ(1, c.sum());
	EXPECT_EQ(1, c.at(2));
}

TEST(StatCounterTest, Increment) {
	Static_Counter c(20);
	c.inc(10);
	c.inc(2);
	c.inc(2);
	EXPECT_EQ(3, c.sum());
	EXPECT_EQ(2, c.at(2));
}

TEST(StatCounterTest, Decrement) {
	Static_Counter c(20);
	c.inc(10);
	c.inc(2);
	c.inc(2);
	c.dec(2);
	EXPECT_EQ(2, c.sum());
	EXPECT_EQ(1, c.at(2));
	c.print();
}

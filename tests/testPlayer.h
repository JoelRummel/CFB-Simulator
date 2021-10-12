#pragma once
#include <gtest/gtest.h>
#include "../src/players/player.h"

TEST(PlayerTestSuite, CreateRatingsVector) {
    std::vector<int> rats = createRatingsVector(OL, 99, 0.0);
    EXPECT_EQ(rats[STRENGTH], 99);
    EXPECT_EQ(rats[RUNBLOCK], 99);
    EXPECT_EQ(rats[PASSBLOCK], 95);

    rats = createRatingsVector(OL, 99, 1.0);
    EXPECT_EQ(rats[STRENGTH], 99);
    EXPECT_EQ(rats[RUNBLOCK], 95);
    EXPECT_EQ(rats[PASSBLOCK], 99);

    rats = createRatingsVector(DL, 46, 0.0);
    EXPECT_EQ(rats[STRENGTH], 46);
}

TEST(PlayerTestSuite, YearString) {
    Player p1("Hello", QB, 1, 50, {}, 1.0);
    EXPECT_EQ(p1.getYearString(), "Freshman");

    Player p2("World", QB, 3, 50, {}, 1.0);
    EXPECT_EQ(p2.getYearString(), "Junior");
}

TEST(PlayerTestSuite, PlayerAgingAndTraining) {
    Player p1("Hello", QB, 3, 70, {}, 0.4);
    p1.ageAndGraduate();
    EXPECT_EQ(p1.getYear(), 4);
    p1.train(10);
    EXPECT_EQ(p1.getOVR(), 80);
    EXPECT_EQ(p1.getRating(SPEED, true), createRatingsVector(QB, 80, 0.4)[SPEED]);
}
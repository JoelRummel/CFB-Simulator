#pragma once
#include <gtest/gtest.h>
#include "../src/players/player.h"

TEST(PlayerTestSuite, CreateRatingsVector) {
    std::vector<int> rats = createRatingsVector(OL, 99, 0.0);
    EXPECT_EQ(rats[STRENGTH], 99);
    EXPECT_EQ(rats[RUNBLOCK], 99);
    EXPECT_EQ(rats[PASSBLOCK], 90);

    rats = createRatingsVector(OL, 99, 1.0);
    EXPECT_EQ(rats[STRENGTH], 99);
    EXPECT_EQ(rats[RUNBLOCK], 90);
    EXPECT_EQ(rats[PASSBLOCK], 99);

    rats = createRatingsVector(DL, 46, 0.0);
    EXPECT_EQ(rats[STRENGTH], 46);
}

TEST(PlayerTestSuite, YearString) {
    Player p1("Hello", QB, 1, 50, 99, {}, 1.0);
    EXPECT_EQ(p1.getYearString(), "Freshman");

    Player p2("World", QB, 3, 50, 99, {}, 1.0);
    EXPECT_EQ(p2.getYearString(), "Junior");
}

TEST(PlayerTestSuite, PlayerAgingAndTraining) {
    Player p1("Hello", QB, 3, 60, 70, {}, 0.4);
    p1.ageAndGraduate();
    EXPECT_EQ(p1.getYear(), 4);
    p1.train(1);
    EXPECT_EQ(p1.getOVR(), 71);
    EXPECT_EQ(p1.getLastTrainingResult(), 11);
}

TEST(PlayerTestSuite, PlayerInjuries) {
    RNG::setRngOverride(1.0);
    Player p1("Hello", QB, 2, 50, 90, {}, 0.0);
    p1.runInjuryRisk(2.0);
    EXPECT_TRUE(p1.isInjured());
    EXPECT_EQ(p1.getWeeksInjured(), -1);

    // Advancing the week on a season-ending injury shouldn't change injury status
    p1.advanceOneWeek();
    EXPECT_TRUE(p1.isInjured());
    EXPECT_EQ(p1.getWeeksInjured(), -1);

    // Advancing the season SHOULD clear the season-ending injury
    p1.ageAndGraduate();
    EXPECT_FALSE(p1.isInjured());
    EXPECT_EQ(p1.getWeeksInjured(), 0);

    // Player shouldn't be injured on good luck
    RNG::setRngOverride(0.5);
    p1.runInjuryRisk(0.1);
    EXPECT_FALSE(p1.isInjured());

    // Player should be injured one week on moderate luck
    RNG::setRngOverride(0.0);
    p1.runInjuryRisk(1.0);
    EXPECT_TRUE(p1.isInjured());
    EXPECT_EQ(p1.getWeeksInjured(), 2);

    // Advancing the weeks should clear the one-week injury
    p1.advanceOneWeek();
    EXPECT_EQ(p1.getWeeksInjured(), 1);
    p1.advanceOneWeek();
    EXPECT_FALSE(p1.isInjured());
}
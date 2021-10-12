#pragma once
#include <gtest/gtest.h>
#include "../src/games/gameManager.h"

class GameManagerTest : public ::testing::Test {
protected:
    City* city;
    School* s1;
    School* s2;
    GameManager gameState;

    void SetUp() override {
        city = new City();
        city->name = "Sample City";
        city->state = "Sample State";
        city->population = 10000;

        s1 = new School("S1", "M1", "S", city, 10, 25000, 1000000, 20, 1);
        s2 = new School("S2", "M1", "S", city, 10, 25000, 1000000, 20, 1);
        gameState.setCompetingSchools(s1, s2);
    }

    void TearDown() override {
        delete city;
        delete s1;
        delete s2;
    }
};

TEST_F(GameManagerTest, CountdownClockFunction) {
    gameState.flipPossession();
    bool homePossession = gameState.homeHasPossession();

    // 900 seconds in each quarter, go to halftime
    for (int i = 900 * 2; i > 0; i -= 25) {
        EXPECT_FALSE(gameState.countdownClock());
    }

    EXPECT_EQ(homePossession, gameState.homeHasPossession());

    // 900 seconds in each quarter, go to halftime
    for (int i = 900 * 2; i > 25; i -= 25) {
        EXPECT_FALSE(gameState.countdownClock());
    }

    EXPECT_TRUE(gameState.countdownClock());
}

TEST_F(GameManagerTest, FlipPossessionFunction) {
    bool homePossession = gameState.homeHasPossession();
    int yardLine = gameState.getYardLine();
    gameState.flipPossession();
    ASSERT_EQ(gameState.homeHasPossession(), !homePossession);
    ASSERT_EQ(gameState.getDown(), 1);
    ASSERT_EQ(gameState.getYardsToGo(), 10);
    ASSERT_FALSE(yardLine == gameState.getYardLine());
    gameState.flipPossession();
    ASSERT_EQ(gameState.homeHasPossession(), homePossession);
    ASSERT_EQ(gameState.getDown(), 1);
    ASSERT_TRUE(yardLine == gameState.getYardLine());
}

TEST_F(GameManagerTest, ResetPossessionFunction) {
    bool homePossession = gameState.homeHasPossession();
    gameState.resetPossession();
    EXPECT_EQ(!homePossession, gameState.homeHasPossession());
    gameState.flipPossession();
    gameState.resetPossession();
    EXPECT_EQ(homePossession, gameState.homeHasPossession());
}

TEST_F(GameManagerTest, GainYardsFunction) {
    // Should start at 75 yards away
    bool homePossession = gameState.homeHasPossession();
    EXPECT_EQ(gameState.getYardLine(), 75);

    EXPECT_FALSE(gameState.gainYards(30));
    EXPECT_EQ(gameState.getDown(), 1);
    EXPECT_EQ(gameState.getYardLine(), 45);

    EXPECT_FALSE(gameState.gainYards(9));
    EXPECT_EQ(gameState.getDown(), 2);
    EXPECT_EQ(gameState.getYardLine(), 36);

    EXPECT_FALSE(gameState.gainYards(0));
    EXPECT_EQ(gameState.getDown(), 3);
    EXPECT_EQ(gameState.getYardLine(), 36);

    EXPECT_FALSE(gameState.gainYards(-10));
    EXPECT_EQ(gameState.getDown(), 4);
    EXPECT_EQ(gameState.getYardLine(), 46);
    EXPECT_EQ(gameState.getLineToGain(), 35);
    EXPECT_EQ(gameState.getYardsToGo(), 11);

    EXPECT_FALSE(gameState.gainYards(1));
    EXPECT_EQ(homePossession, !gameState.homeHasPossession());
    EXPECT_EQ(gameState.getDown(), 1);

    EXPECT_TRUE(gameState.gainYards(100));
    EXPECT_EQ(homePossession, gameState.homeHasPossession());
    EXPECT_EQ(gameState.getDown(), 1);
    EXPECT_EQ(gameState.getYardLine(), 75);
    EXPECT_EQ(gameState.getLineToGain(), 65);

    EXPECT_EQ(homePossession ? gameState.getAwayScore() : gameState.getHomeScore(), 7);
}

TEST_F(GameManagerTest, ScoreFieldGoal) {
    bool homePossession = gameState.homeHasPossession();
    gameState.gainYards(30);
    gameState.scoreFieldGoal();
    EXPECT_EQ(homePossession ? gameState.getHomeScore() : gameState.getAwayScore(), 3);
    EXPECT_EQ(!homePossession, gameState.homeHasPossession());
    EXPECT_EQ(gameState.getYardLine(), 75);
    EXPECT_EQ(gameState.getLineToGain(), 65);
}

TEST_F(GameManagerTest, PuntBall) {
    bool homePossession = gameState.homeHasPossession();
    gameState.puntBall(5);
    EXPECT_EQ(gameState.homeHasPossession(), !homePossession);
    EXPECT_EQ(gameState.getYardLine(), 30);
    EXPECT_EQ(gameState.getLineToGain(), 20);
}
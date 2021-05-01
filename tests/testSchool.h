#pragma once
#include <gtest/gtest.h>
#include "../school.h"
#include "../coachesOrg.h"

class SchoolsTest : public ::testing::Test {
protected:
    City* city;
    School* s1;

    void SetUp() override {
        city = new City();
        city->name = "Sample City";
        city->state = "Sample State";
        city->population = 10000;

        s1 = new School("S1", "M1", "S", city, 10, 25000, 1000000, 20, 1);
        for (int i = 0; i < 11; i++) {
            s1->signCoach(new Coach(false, (CoachType)i), (CoachType)i);
        }
    }

    void TearDown() override {
        delete city;
        delete s1;
    }
};

TEST_F(SchoolsTest, SchedulingStrategy) {
    s1->setDivision(BIGTENEAST);
    EXPECT_EQ(s1->strategy.easyDesired, 1);
    EXPECT_EQ(s1->strategy.mediumDesired, 1);
    EXPECT_EQ(s1->strategy.hardDesired, 1);

    s1->setDivision(MWCMOUNTAIN);
    EXPECT_EQ(s1->strategy.easyDesired, 0);
    EXPECT_EQ(s1->strategy.mediumDesired, 2);
    EXPECT_EQ(s1->strategy.hardDesired, 2);

    EXPECT_TRUE(s1->strategy.wantSchool(s1));
}

TEST_F(SchoolsTest, PlayersGraduate) {
    for (int i = 0; i < 4; i++) {
        s1->advanceRosterOneYear();
    }
    EXPECT_EQ(s1->getRoster()->getRosterSize(), 0);
}

TEST_F(SchoolsTest, PrestigeDropsAfterBadSeason) {
    s1->setRanking(130);
    s1->assessSelf();
    EXPECT_EQ(s1->getPrestige(), 5);
}

TEST_F(SchoolsTest, CoachingStaffGetsCannedAfterBadSeason) {
    for (int i = 0; i < 11; i++) EXPECT_FALSE(s1->isVacant((CoachType)i));
    s1->setRanking(130);
    s1->assessSelf();
    s1->assessSelf();
    s1->makeCoachingDecisions();
    for (int i = 0; i < 11; i++) EXPECT_TRUE(s1->isVacant((CoachType)i));
}
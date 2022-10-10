#pragma once
#include <gtest/gtest.h>
#include "../../src/recruits/recruit.h"

TEST(RecruitTestSuite, FactoryPositionAssignment) {
    RNG::setRngOverride(0);
    Recruit r1 = recruitFactory();
    EXPECT_EQ(r1.getUnderlyingPlayer()->getPosition(), QB);

    RNG::setRngOverride(69);
    Recruit r2 = recruitFactory();
    EXPECT_EQ(r2.getUnderlyingPlayer()->getPosition(), K);
}

TEST(RecruitTestSuite, FactoryOvrAssignment) {
    Recruit r1 = recruitFactory(0);
    EXPECT_EQ(r1.getUnderlyingPlayer()->getOVR(), 20);

    Recruit r2 = recruitFactory(1);
    EXPECT_EQ(r2.getUnderlyingPlayer()->getOVR(), 76);
}

TEST(RecruitTestSuite, ConstructorPotentialAssignment) {
    RNG::setRngOverride(1.0);
    Recruit r1(QB, 50);

    for (int i = 0; i < 3; i++) {
        r1.getUnderlyingPlayer()->ageAndGraduate();
        r1.getUnderlyingPlayer()->train(0.75);
    }
    EXPECT_EQ(r1.getUnderlyingPlayer()->getOVR(), 81);
}
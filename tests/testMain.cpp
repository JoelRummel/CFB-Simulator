#include <gtest/gtest.h>
#include "testPlayer.h"
#include "testSchool.h"
#include "testGameManager.h"
#include "../src/loadData.h"

int main(int argc, char** argv) {
    GlobalData::loadEverything();

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
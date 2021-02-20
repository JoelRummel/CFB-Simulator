#pragma once
#include "player.h"

#define NUM_PREFS 5

class Recruit {
private:

    Player* player = nullptr;

    double preferences[NUM_PREFS];

public:

    enum Preference {
        PROX_TO_HOME,
        ACADEMICS,
        PRESTIGE,
        NFL,
        PLAY_TIME
    };

    Recruit(Position pos, int ovr) {
        player = new Player(playerFactory(pos, 0, 0, ovr));

        // Determine preferences
        double remainder = 1.0;
        bool tripped[NUM_PREFS] = { false, false, false, false, false }; // also gotta update this lol
        while (remainder > 0.01) {
            int nextPref = -1;
            while (nextPref == -1) {
                int i = std::rand() % NUM_PREFS;
                if (!tripped[i]) {
                    nextPref = i;
                    tripped[i] = true;
                }
            }

            double amnt = RNG::randomNumberUniformDist(0.0, remainder);
            preferences[nextPref] = amnt;
            remainder -= amnt;

            bool untripped = false;
            for (int i = 0; i < NUM_PREFS; i++) if (!tripped[i]) untripped = true;
            if (!untripped) break;
        }
    }

    Player* getUnderlyingPlayer() { return player; }

    int getStars() {
        if (player == nullptr) throw "Error: Recruit not initialized";
        int ovr = player->getOVR();
        if (ovr >= 73) return 5;
        if (ovr >= 66) return 4;
        if (ovr >= 59) return 3;
        if (ovr >= 52) return 2;
        if (ovr >= 45) return 1;
        return 0;
    }

    std::string getStarString() {
        std::string stars = "";
        int starCount = getStars();
        for (int i = 0; i < starCount; i++) {
            stars += "★";
        }
        return stars;
    }

    std::string getPaddedStarString() {
        std::string stars = "";
        int starCount = getStars();
        for (int i = 1; i <= 5; i++) {
            if (i <= starCount) stars += "★";
            else stars += " ";
        }
        return stars;
    }
};

const std::vector<std::pair<Position, int>> POSITION_DISTRIBUTION = {
    {QB, 4},
    {HB, 5},
    {OL, 11},
    {WR, 7},
    {TE, 3},
    {DL, 10},
    {LB, 8},
    {CB, 7},
    {S, 5},
    {P, 1},
    {K, 1}
};
const int TOTAL_POSITION_DISTRIBUTION = 62;

/**
 * Recruit OVR range: 40-78
 * Maximum annual training increment: 7 (brings 78 to 99 after 3 years of training)
 * 5 star: 73-78 (6 pts)
 * 4 star: 66-72 (7 pts)
 * 3 star: 59-65 (7 pts)
 * 2 star: 52-58 (7 pts)
 * 1 star: 45-51 (7 pts)
 * 0 star: 40-44 (5 pts)
 */
Recruit recruitFactory() {
    int positionSelection = std::rand() % TOTAL_POSITION_DISTRIBUTION;
    Position p;
    for (auto dist : POSITION_DISTRIBUTION) {
        positionSelection -= dist.second;
        if (positionSelection < 0) {
            p = dist.first;
            break;
        }
    }

    double x = RNG::randomNumberUniformDist();

    double ovr;
    if (x < 0.05) ovr = 40 + (100 * x);
    else if (x < 0.6929) ovr = 44 + (22 * x);
    else if (x < 0.8821) ovr = 57 + (19 * std::pow(x, 6));
    else if (x < 0.9875) ovr = 57 + std::pow(17, std::pow(x, 2));
    else ovr = 59 + std::pow(20, std::pow(x, 10));

    Recruit r(p, std::floor(ovr));
    return r;
}
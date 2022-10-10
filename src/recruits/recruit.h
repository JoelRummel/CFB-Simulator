#pragma once
#include "../players/player.h"

#define NUM_PREFS 5

const std::vector<std::pair<Position, int>> POSITION_DISTRIBUTION = {
    {QB, 4},
    {HB, 5},
    {OL, 11},
    {WR, 9},
    {TE, 4},
    {DL, 10},
    {LB, 9},
    {CB, 8},
    {S, 6},
    {P, 2},
    {K, 2}
};
const int TOTAL_POSITION_DISTRIBUTION = 70;

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

    Recruit() {}

    Recruit(Position pos, int ovr) {
        // Potential range: -10 - 30
        double potential = 0;
        double x = RNG::randomNumberUniformDist();
        if (x < 0.3) potential = 17 * std::log(5 * x + 0.5) + 3;
        else if (x < 0.9) potential = 14 * x + 10;
        else potential = 30 * std::pow(x, 3) + 1;

        player = new Player(playerFactory(pos, 1, 0, ovr, std::floor(potential) + ovr));

        // Determine preferences
        const std::vector<std::vector<double>> weights = { {0.8, 0.2, 0, 0, 0}, {0.7, 0.1, 0.1, 0.1, 0}, {0.6, 0.3, 0.1, 0, 0}, {0.5, 0.25, 0.25, 0, 0}, {0.4, 0.2, 0.2, 0.2, 0}, {0.3, 0.2, 0.2, 0.2, 0.1} };
        std::vector<double> selection = *select_randomly(weights.begin(), weights.end());
        RNG::shuffle(selection);
        for (int i = 0; i < NUM_PREFS; i++) preferences[i] = selection[i];
    }

    Player* getUnderlyingPlayer() { return player; }

    void updateUnderlyingPlayer(Player* newPlayer) { player = newPlayer; }

    int getStars() {
        if (player == nullptr) throw "Error: Recruit not initialized";
        int ovr = player->getOVR();
        if (ovr >= 57) return 5;
        if (ovr >= 47) return 4;
        if (ovr >= 40) return 3;
        if (ovr >= 33) return 2;
        if (ovr >= 27) return 1;
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

    std::string getTopPreferenceStr() {
        double bestPref = 0;
        int prefIndex = -1;
        for (int i = 0; i < NUM_PREFS; i++) {
            if (preferences[i] > bestPref) {
                prefIndex = i;
                bestPref = preferences[i];
            }
        }
        std::string str;
        Preference pref = (Preference)prefIndex;
        switch (pref) {
        case PROX_TO_HOME: str = "Proximity to home"; break;
        case ACADEMICS: str = "Academics"; break;
        case NFL: str = "NFL prospects"; break;
        case PRESTIGE: str = "School prestige"; break;
        case PLAY_TIME: str = "Early playing time"; break;
        }
        str += " (" + std::to_string((int)(bestPref * 100));
        str += "%)";
        return str;
    }

    double rateSchoolPreference(int prestige, City* city, int academics, int nfl, std::vector<Player*> others) {
        double score = 0.0;
        score += (prestige / 10.0) * preferences[PRESTIGE];
        score += ((300 - academics) / 299.0) * preferences[ACADEMICS];
        score += (nfl / 56.0) * preferences[NFL];
        double dist = std::min(City::distance(city, player->getHometown()), 2500.0);
        score += ((2500 - dist) / 2500.0) * preferences[PROX_TO_HOME];

        int betterThanMe = 0;
        for (Player* other : others) {
            if (other->getOVR() >= player->getOVR()) betterThanMe++;
        }
        int availableSpots;
        for (int i = 0; i < (int)POSITION_DISTRIBUTION.size(); i++) {
            if (POSITION_DISTRIBUTION[i].first == player->getPosition()) {
                availableSpots = POSITION_DISTRIBUTION[i].second;
                break;
            }
        }
        if ((int)others.size() >= availableSpots) return -1;
        // Todo: consider scaling up the PLAY_TIME preference?
        score += ((availableSpots - betterThanMe) / (double)availableSpots) * (preferences[PLAY_TIME] + 0.25);

        return score;
    }
};


/**
 * Recruit OVR range: 20-75
 * 5 star: 57-75 (14 pts)
 * 4 star: 47-56 (10 pts)
 * 3 star: 40-46 (7 pts)
 * 2 star: 33-39 (7 pts)
 * 1 star: 27-32 (6 pts)
 * 0 star: 20-26 (6 pts)
 */
Recruit recruitFactory(double seedOverride = -1) {
    int positionSelection = RNG::randomNumberUniformDist(1, TOTAL_POSITION_DISTRIBUTION);
    Position p;
    for (auto dist : POSITION_DISTRIBUTION) {
        positionSelection -= dist.second;
        if (positionSelection <= 0) {
            p = dist.first;
            break;
        }
    }

    double x = RNG::randomNumberUniformDist();
    if (seedOverride > -1) x = seedOverride;

    double ovr;
    if (x < 0.05) ovr = 20 + (140 * x);
    else if (x < 0.6929) ovr = 26 + (20 * x);
    else if (x < 0.8821) ovr = 38 + (19 * std::pow(x, 6));
    else if (x < 0.9875) ovr = 41 + std::pow(21, std::pow(x, 5));
    else ovr = 55 + std::pow(21, std::pow(x, 80));

    Recruit r(p, std::floor(ovr));
    return r;
}
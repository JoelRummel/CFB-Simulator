#pragma once
#include "recruit.h"
#include "../school.h"

struct SortByRecruitOvr {
    bool operator()(Recruit a, Recruit b) { return (a.getUnderlyingPlayer()->getOVR() > b.getUnderlyingPlayer()->getOVR()); }
};

struct TopRecruitingClass {
    Recruit recruits[10];
    School* schoolChoices[10];
};

class RecruitLounge {

    std::vector<Recruit> recruits;

public:
    void generateNewRecruitingClass() {
        recruits.clear();
        for (int i = 0; i < 2800; i++) {
            recruits.push_back(recruitFactory());
        }
        for (int i = 0; i < 2000; i++) {
            // recruits.push_back(recruitFactory(RNG::randomNumberUniformDist(0.0, 0.03)));
        }
        SortByRecruitOvr sbro;
        std::sort(recruits.begin(), recruits.end(), sbro);
    }

    School* pickFavoriteSchool(std::vector<School*>& allSchools, int recruitIndex) {
        double bestScore = -1;
        School* favorite = nullptr;
        Recruit& recruit = recruits[recruitIndex];
        for (School* school : allSchools) {
            if (school->getRoster()->getRosterSize() >= 70) continue;

            int prestige = school->getPrestige();
            int academics = school->getAcademicRating();
            City* city = school->getCity();
            int nfl = school->getNFLRating();
            std::vector<Player*> others = school->getRoster()->getAllPlayersAt(recruit.getUnderlyingPlayer()->getPosition(), false);
            int maxSpots;
            for (auto dist : POSITION_DISTRIBUTION) if (dist.first == recruit.getUnderlyingPlayer()->getPosition()) maxSpots = dist.second;
            if (others.size() >= maxSpots) continue;
            double thisScore = recruit.rateSchoolPreference(prestige, city, academics, nfl, others);
            double multiplier = school->getRecruitingMultiplier(recruit.getUnderlyingPlayer()->getPosition());
            multiplier += (1 - multiplier) * 0.6;
            thisScore *= multiplier;
            if (thisScore > bestScore) {
                favorite = school;
                bestScore = thisScore;
            }
        }
        if (favorite == nullptr) {
            // No one wanted him. Sad :(
            delete recruit.getUnderlyingPlayer();
        }
        return favorite;
    }

    TopRecruitingClass signRecruitingClass(std::vector<School*>& allSchools) {
        TopRecruitingClass trc;
        for (int i = 0; i < recruits.size(); i++) {
            School* winner = pickFavoriteSchool(allSchools, i);
            if (winner != nullptr) {
                Player* newPlayer = winner->signRecruit(recruits[i].getUnderlyingPlayer(), recruits[i].getStars()); // this deletes the underlying player
                if (i < 10) {
                    trc.recruits[i] = recruits[i];
                    trc.recruits[i].updateUnderlyingPlayer(newPlayer);
                    trc.schoolChoices[i] = winner;
                }
            }
        }
        int walkOnsNeeded = 0;
        for (auto& school : allSchools) {
            for (auto dist : POSITION_DISTRIBUTION) {
                walkOnsNeeded += (dist.second - school->getRoster()->getAllPlayersAt(dist.first).size());
                // if (walkOnsNeeded > 500) std::cout << "SCHOOOOL: " << school->getName() << " - " << positionToStr(dist.first) << std::endl;
            }
        }
        std::cout << "WALK ONS NEEDED: " << walkOnsNeeded << std::endl;
        return trc;
    }
};
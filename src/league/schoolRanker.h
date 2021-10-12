#pragma once
#include "../school.h"

struct SortByPublicRating {
    bool operator()(School* a, School* b) { return (a->getRankingScore() > b->getRankingScore()); }
};

class SchoolRanker {

    std::vector<School*> allSchools;

    double decideNewRankingScore(School* school, int week) {
        week--;
        School::Matchup* matchup = school->getGameResults(week);
        if (matchup != nullptr && matchup->gameResult.awayStats != nullptr) {
            std::pair<TeamStats*, TeamStats*> stats = school->getOrderedStats(matchup);

            int pointMargin = stats.first->points - stats.second->points;
            if (pointMargin > 28) pointMargin = 28;
            if (pointMargin < -28) pointMargin = -28;
            if (pointMargin > 0) pointMargin += 14;
            else pointMargin -= 14; // pointMargin is now between -42 and 42
            double baseScore = pointMargin > 0 ? 10 : -10;

            bool away = matchup->away == school;
            int theirRanking = away ? matchup->home->getRanking() : matchup->away->getRanking();
            baseScore *= pointMargin + ((262 - (theirRanking + 131)) / 2.0);

            return school->getRankingScore() + (baseScore * ((week + 30) / 45.0));
        }
        return school->getRankingScore();
    }

public:
    void resetPoll(std::vector<School*> schools) {
        allSchools = schools;
        for (auto& school : allSchools) {
            school->resetRankingScore();
        }
    }

    void rankTeams(int week) {
        for (auto& school : allSchools) {
            school->setRankingScore(decideNewRankingScore(school, week));
        }
        SortByPublicRating sbr;
        std::sort(allSchools.begin(), allSchools.end(), sbr);
        for (int i = 0; i < (int)allSchools.size(); i++) { allSchools[i]->setRanking(i + 1); }
    }

    void printAPTop25(int week) {
        //			  v1   #4  Alabama        (4 - 1)  W Texas A&M (42-21)
        std::cout << "Diff Rank School          W-L    Last Week\n";
        std::cout << "-----------------------------------------------------------\n";
        for (int i = 0; i < 25; ++i) {
            std::string rcstr = allSchools[i]->getRankingChangeString();
            std::string name = allSchools[i]->getName();
            std::string wlstr = allSchools[i]->getWinLossString();
            std::string lwstr = allSchools[i]->getWeekResultString(week - 1);
            printf("%-4s #%-2d %-15s (%s)  %s\n", rcstr.c_str(), i + 1, name.c_str(), wlstr.c_str(), lwstr.c_str());
        }
    }
};
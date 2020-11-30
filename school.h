#pragma once

#include "coach.h"
#include "roster.h"

#include <iostream>
#include <string>
// todo: delete above include

class School {
  public:
	struct Matchup {
		School* away;
		School* home;
		GameResult gameResult;
	};
	struct NonConStrategy {
		// outlines number of cupcake/quality/challenge games desired vs how many scheduled
		int easyDesired = 0;
		int mediumDesired = 0;
		int hardDesired = 0;
		int easyScheduled = 0;
		int mediumScheduled = 0;
		int hardScheduled = 0;

		bool wantSchool(School* s) {
			int diff = difficulty(s->getPrestige());
			if (diff == 0) return easyScheduled < easyDesired;
			if (diff == 1) return mediumScheduled < mediumDesired;
			return hardScheduled < hardDesired;
		}

		void registerSchedule(School* s) {
			int diff = difficulty(s->getPrestige());
			if (diff == 0) easyScheduled++;
			if (diff == 1) mediumScheduled++;
			if (diff == 2) hardScheduled++;
		}

		void decideStrategy(int prestige, Conference div) {
			bool p5 = isP5(div);
			int nonConGames = numNonConGames(div);
			easyScheduled = 0;
			mediumScheduled = 0;
			hardScheduled = 0;
			easyDesired = 0;
			mediumDesired = 0;
			hardDesired = 0;
			if (p5) {
				if (prestige >= 8) {
					easyDesired = 1;
					mediumDesired = 1;
					hardDesired = 1;
					if (nonConGames == 4) easyDesired = 2;
				} else if (prestige >= 5) {
					easyDesired = 2;
					mediumDesired = 1;
					if (nonConGames == 4) mediumDesired = 2;
				} else {
					easyDesired = 2;
					mediumDesired = 1;
					if (nonConGames == 4) easyDesired = 3;
				}
			} else {
				mediumDesired = 1;
				hardDesired = 2;
				if (nonConGames == 4) mediumDesired = 2;
			}
		}

	  private:
		int difficulty(int prestige) {
			if (prestige <= 4) return 0;
			if (prestige <= 7) return 1;
			return 2;
		}
	};

  private:
	std::string name;
	std::string mascot;
	std::string city;
	std::string state;
	int nflRating;
	int academicRating;
	int stadiumCapacity;
	int budget;
	Roster roster;
	int prestige;
	std::vector<Matchup*> schedule;
	int numGamesScheduled = 0;
	int conferenceGamesScheduled = 0;
	int crossConfGamesScheduled = 0;
	Conference division;
	double simpleRating = 0;
	double publicRealRating = 0;
	double privateRealRating = 0;
	int ranking = -1;

	Coach* coaches[11];

	std::pair<TeamStats*, TeamStats*> getOrderedStats(Matchup* m) {
		if (this == m->away) {
			return std::make_pair(m->gameResult.awayStats, m->gameResult.homeStats);
		} else
			return std::make_pair(m->gameResult.homeStats, m->gameResult.awayStats);
	}

  public:
	NonConStrategy strategy;

	School(std::string na, std::string ma, std::string st, std::string ci, int pr, int ss, int bu, int nfl, int ac) :
			name { na }, mascot { ma }, state { st }, city { ci }, prestige { pr }, stadiumCapacity { ss }, budget { bu }, nflRating { nfl },
			academicRating { ac } {
		roster.generateRoster(pr);
		schedule.resize(16, nullptr);
		for (int i = 0; i < 11; i++) coaches[i] = nullptr;
	}

	std::string getName() { return name; }
	std::string getMascot() { return mascot; }
	std::string getCityName() { return city; }
	std::string getStateName() { return state; }
	int getStadiumCapacity() { return stadiumCapacity; }
	int getBudget() { return budget; }
	int getNFLRating() { return nflRating; }
	int getAcademicRating() { return academicRating; }
	std::string getRankedName() {
		if (ranking == -1 || ranking > 25) return getName();
		return "(#" + std::to_string(ranking) + ") " + getName();
	}
	Roster* getRoster() { return &roster; }
	void assignGame(int week, Matchup* s, bool confGame = true, bool crossConfGame = false) {
		if (s == nullptr) {
			numGamesScheduled--;
			crossConfGamesScheduled--;
			// delete schedule[week];
			schedule[week] = nullptr;
			return;
		}
		assert(s->away != nullptr && s->home != nullptr);
		assert(schedule[week] == nullptr);
		schedule[week] = s;
		numGamesScheduled++;
		if (confGame) conferenceGamesScheduled++;
		else if (crossConfGame)
			crossConfGamesScheduled++;
	}
	int getPrestige() { return prestige; }
	int getNumGamesScheduled() { return numGamesScheduled; }
	int getConferenceGamesScheduled() { return conferenceGamesScheduled; }
	int getCrossConfGamesScheduled() { return crossConfGamesScheduled; }
	Conference getDivision() { return division; }
	void setDivision(Conference d) {
		division = d;
		strategy.decideStrategy(prestige, d);
	}
	bool isOnSchedule(School* school) {
		for (auto& matchup : schedule) {
			if (matchup != nullptr)
				if (matchup->away == school || matchup->home == school) return true;
		}
		return false;
	}
	School* getScheduledOpponent(int week) {
		Matchup* m = schedule[week];
		if (m == nullptr) return nullptr;
		if (m->away == this) return m->home;
		return m->away;
	}
	Matchup* getMatchupAgainst(School* s) {
		for (auto& m : schedule) {
			if (m != nullptr && (m->away == s || m->home == s)) { return m; }
		}
		return nullptr;
	}
	bool didIWinAgainst(School* s) {
		Matchup* m = getMatchupAgainst(s);
		if (m == nullptr) return false;
		if (m->gameResult.homeStats == nullptr) return false;
		if (s == m->away) return m->gameResult.homeStats->points > m->gameResult.awayStats->points;
		return m->gameResult.awayStats->points > m->gameResult.homeStats->points;
	}
	Matchup* getGameResults(int week) { return schedule[week]; }
	std::pair<int, int> getWinLossRecord(bool confRecord = false) {
		std::pair<int, int> winsLosses;
		for (auto& game : schedule) {
			if (game != nullptr && game->gameResult.homeStats != nullptr) {
				if (confRecord && game->away->getDivision() != game->home->getDivision() &&
					game->away->getDivision() != getOppositeDivision(game->home->getDivision()))
					continue;
				bool awayWon = game->gameResult.awayStats->points > game->gameResult.homeStats->points;
				if (this == game->away && awayWon) {
					winsLosses.first++;
				} else if (this == game->home && !awayWon) {
					winsLosses.first++;
				} else
					winsLosses.second++;
			}
		}
		return winsLosses;
	}
	std::string getWinLossString(bool confRecord = false) {
		std::pair<int, int> record = getWinLossRecord(confRecord);
		return std::to_string(record.first) + " - " + std::to_string(record.second);
	}

	TeamStats* getMyStats(int week) {
		if (schedule[week] == nullptr) return nullptr;
		if (this == schedule[week]->away) return schedule[week]->gameResult.awayStats;
		return schedule[week]->gameResult.homeStats;
	}

	double getAverageOffense() {
		int yards = 0;
		int games = 0;
		for (auto& game : schedule) {
			if (game != nullptr) {
				games++;
				if (this == game->away) yards += game->gameResult.awayStats->offensiveYards();
				else
					yards += game->gameResult.homeStats->offensiveYards();
			}
		}
		if (games == 0) return 0;
		return (double)yards / games;
	}

	int getRanking() { return ranking; }
	void setRanking(int r) { ranking = r; }

	double getPublicRating() { return publicRealRating; }

	void setSimpleRating() {
		int gamesCounted = 0;
		simpleRating = 0;
		for (auto& matchup : schedule) {
			if (matchup != nullptr && matchup->gameResult.homeStats != nullptr) {
				gamesCounted++;
				std::pair<TeamStats*, TeamStats*> stats = getOrderedStats(matchup);
				int margin = stats.first->points - stats.second->points;
				/*if (margin > 24) margin = 24;
				else if (margin > 0 && margin < 7)
					margin = 7;
				else if (margin > -7 && margin < 0)
					margin = -7;
				else if (margin < -24)
					margin = -24;*/
				if (margin > 0) margin += 14;
				else if (margin < 0)
					margin -= 14;
				simpleRating += margin;
			}
		}
		if (gamesCounted == 0) return;
		simpleRating /= (double)gamesCounted;
		publicRealRating = simpleRating;
	}

	void updatePrivateRating() {
		int gamesCounted = 0;
		double cumulativeRating = 0;
		for (auto& matchup : schedule) {
			if (matchup == nullptr) continue;
			gamesCounted++;
			if (matchup->away == this) cumulativeRating += matchup->home->getPublicRating();
			else
				cumulativeRating += matchup->away->getPublicRating();
		}
		if (gamesCounted == 0) return;
		privateRealRating = simpleRating + (cumulativeRating / gamesCounted);
	}

	void publishPrivateRating() { publicRealRating = privateRealRating; }

	void signCoach(Coach* newCoach, CoachType type) {
		assert(isVacant(type));
		coaches[(int)type] = newCoach;
		if (type == CoachType::HC || type == CoachType::OC || type == CoachType::DC || type == CoachType::LB) {
			std::cout << getName() << " has hired " << newCoach->getName() << " (" << newCoach->getPublicOvr() << "/" << newCoach->getActualOvr()
					  << " public OVR) as " << coachTypeToString(type) << std::endl;
		}
	}

	bool isVacant(CoachType position) { return coaches[(int)position] == nullptr; }

	void printDetails() {
		std::cout << "\n===== " << str_upper(getRankedName()) << " " << str_upper(getMascot()) << " =====\n";
		std::cout << "      Record:       " << getWinLossString() << "\n";
		std::cout << "      Conf. record: " << getWinLossString(true) << "\n";
		std::cout << "      Head Coach: " << coaches[(int)CoachType::HC]->getName() << " (" << coaches[(int)CoachType::HC]->getPublicOvr()
				  << " public OVR)\n";
	}
};
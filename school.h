#pragma once

#include "coach.h"
#include "roster.h"

#include <iostream>
#include <string>
// todo: delete above include

struct SortByAdjustedPublicOvr {
	double adj;

	bool operator()(Coach* a, Coach* b) {
		double aAdjusted = a->getPublicOvr() - ((a->getPublicOvr() - a->getActualOvr()) * adj);
		double bAdjusted = b->getPublicOvr() - ((b->getPublicOvr() - b->getActualOvr()) * adj);
		return (aAdjusted > bAdjusted);
	}
};

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
	int prestigeFraction = 0;
	std::vector<Matchup*> schedule;
	int numGamesScheduled = 0;
	int conferenceGamesScheduled = 0;
	int crossConfGamesScheduled = 0;
	Conference division;
	double simpleRating = 0;
	double publicRealRating = 0;
	double privateRealRating = 0;
	int ranking = -1;
	int offenseRanking = -1;
	int defenseRanking = -1;

	Coach* coaches[11];

	std::pair<TeamStats*, TeamStats*> getOrderedStats(Matchup* m) {
		if (this == m->away) {
			return std::make_pair(m->gameResult.awayStats, m->gameResult.homeStats);
		} else
			return std::make_pair(m->gameResult.homeStats, m->gameResult.awayStats);
	}

	void printCoachBars(Coach* c[3], std::string rating) {
		printf("%-25s|%-25s|%-25s\n", c[0]->getTypeString().c_str(), c[1]->getTypeString().c_str(),
			   c[2] != nullptr ? c[2]->getTypeString().c_str() : "");
		double cumPercent; // CUMULATIVE percent, obviously
		for (int i = 0; i < (c[2] != nullptr ? 3 : 2); i++) {
			int r;
			if (rating == "d") r = c[i]->getOvrDevelopment();
			else if (rating == "r")
				r = c[i]->getOvrRecruiting();
			else
				r = c[i]->getOvrGametime();
			double pct = ((r - 40) / 59.0) * 100.0;
			cumPercent += pct;
			for (int j = 0; j < 100; j += 4) std::cout << ((j < pct) ? "█" : " ");
			std::cout << "|";
		}
		printf("  Total: %.1f%%\n\n", cumPercent / (c[2] != nullptr ? 3 : 2));
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

	// TODO: old, outdated function. get rid of it
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

	std::pair<double, double> getAverageOffenseDefense() {
		// Make sure not to include postseason
		TeamStats ts;
		for (int i = 0; i < 13; i++) {
			if (schedule[i] != nullptr) ts += *(getOrderedStats(getGameResults(i)).first);
		}
		return std::make_pair(ts.offensiveYards() / (double)ts.games, ts.yardsAllowed / (double)ts.games);
	}

	int getRanking() { return ranking; }
	void setRanking(int r) { ranking = r; }
	void setOffenseRanking(int r) { offenseRanking = r; }
	void setDefenseRanking(int r) { defenseRanking = r; }
	int getOffenseRanking() { return offenseRanking; }
	int getDefenseRanking() { return defenseRanking; }

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

	Coach* snipeCoach(std::vector<Coach*> snipeSet, std::vector<Vacancy>& competitors, int myVacancyIndex) {
		// assume the first element of the vector is the coach who originally wanted to sign with us
		Coach* hc = coaches[(int)CoachType::HC];
		Coach* original = snipeSet[0]; // failsafe variable
		if (hc == nullptr) return original;
		double assessmentAbility = (hc->getActualOvr() - 40) / 59.0;
		// Sort into our favorite coaches (yikes on efficiency)
		SortByAdjustedPublicOvr sbao { assessmentAbility };
		std::sort(snipeSet.begin(), snipeSet.end(), sbao);
		for (Coach* candidate : snipeSet) {
			if (candidate->pickFavoriteJob(competitors) == myVacancyIndex) return candidate;
		}
		return original;
	}

	void assessSelf() {
		std::vector<int> boundaries { 4, 9, 16, 26, 41, 56, 76, 96, 116, 126, 131 };
		int performance;
		// We should never get past 11 iterations anyways
		for (int i = 0; i < 11; i++) {
			if (ranking < boundaries[i]) {
				performance = 10 - i;
				break;
			}
		}
		for (int movements = performance - prestige; movements != 0; movements += (movements > 0 ? -1 : 1)) {
			if (movements > 0) prestigeFraction += 1;
			else
				prestigeFraction -= 1;
			if (std::abs(prestigeFraction) > 1) {
				prestige += prestigeFraction / 2; // either 2/2 or -2/2
				prestigeFraction = 0;
			}
		}
	}

	void assessCoaches() {
		// Assess head coach using national ranking
		double rankingRatio = (130 - ranking) / 129.0;
		double offRatio = (((130 - offenseRanking) / 129.0) * 0.5) + (rankingRatio * 0.5);
		double defRatio = (((130 - defenseRanking) / 129.0) * 0.5) + (rankingRatio * 0.5);
		double allRatio = (0.5 * offRatio) + (0.5 * defRatio);
		for (CoachType position : { CoachType::HC, CoachType::ST }) coaches[(int)position]->givePublicAssessment(allRatio);
		for (CoachType position : { CoachType::OC, CoachType::OL, CoachType::QB, CoachType::RB, CoachType::WR })
			coaches[(int)position]->givePublicAssessment(offRatio);
		for (CoachType position : { CoachType::DC, CoachType::DB, CoachType::LB, CoachType::DL })
			coaches[(int)position]->givePublicAssessment(defRatio);

		// Take the opportunity to adjust our prestige as well
		assessSelf();
	}

	void applyGametimeBonuses() {
		for (Position p : { QB, HB, WR, TE, OL, DL, LB, CB, S, K, P }) {
			std::vector<Player*> players = roster.getAllPlayersAt(p, false);
			for (Player* player : players) {
				double bonus = coaches[(int)getPositionalCoachType(p)]->getActualOvr();
				CoachType t = getSecondLevelCoachType(p);
				if (t != CoachType::ST) bonus += coaches[(int)t]->getActualOvr();
				bonus += coaches[(int)CoachType::HC]->getActualOvr();
				bonus -= (t == CoachType::ST ? 80 : 120);
				bonus /= (59.0 * (t == CoachType::ST ? 2 : 3));
				player->setGametimeBonus(bonus);
			}
		}
	}

	bool isVacant(CoachType position) { return coaches[(int)position] == nullptr; }

	void printDetails() {
		std::cout << "\n===== " << str_upper(getRankedName()) << " " << str_upper(getMascot()) << " =====\n";
		std::cout << "      Prestige: ";
		for (int i = 1; i < 11; i++) std::cout << (i <= prestige ? "★" : "☆");
		std::cout << "\n      Record:       " << getWinLossString() << "\n";
		std::cout << "      Conf. record: " << getWinLossString(true) << "\n";
		std::cout << "      Head Coach: " << coaches[(int)CoachType::HC]->getName() << " (" << coaches[(int)CoachType::HC]->getPublicOvr()
				  << " public OVR)\n";
	}
	void printCoachingStaff() {
		// Print details of each coach
		std::cout << std::endl;
		for (int i = 0; i < 11; i++) {
			Coach* c = coaches[i];
			printf("%-27s%-20s%d/%d public OVR ( %d DVLP | %d RCRT | %d GAME )\n", coachTypeToString((CoachType)i).c_str(), c->getName().c_str(),
				   c->getPublicOvr(), c->getActualOvr(), c->getOvrDevelopment(), c->getOvrRecruiting(), c->getOvrGametime());
		}
	}
	void printCoachingImpact(Position p) {
		Coach* c[3];
		c[0] = coaches[(int)CoachType::HC];
		c[1] = coaches[(int)getSecondLevelCoachType(p)];
		c[2] = coaches[(int)getPositionalCoachType(p)];
		if (getPositionalCoachType(p) == CoachType::ST) c[2] = nullptr;
		std::cout << "PLAYER DEVELOPMENT:\n";
		printCoachBars(c, "d");
		std::cout << "RECRUITING:\n";
		printCoachBars(c, "r");
		std::cout << "GAME PLANNING + MOTIVATION:\n";
		printCoachBars(c, "g");
	}
	// Normally used in case of scheduling error and need to start again from scratch
	void nukeSchedule() {
		numGamesScheduled = 0;
		conferenceGamesScheduled = 0;
		crossConfGamesScheduled = 0;
		schedule.clear(); // this prolly causes a memory leak
		schedule.resize(16, nullptr);
	}
	void prepareNextSeason() {
		ranking = -1;
		offenseRanking = -1;
		defenseRanking = -1;
		simpleRating = 0;
		publicRealRating = 0;
		privateRealRating = 0;
		nukeSchedule();
	}
};
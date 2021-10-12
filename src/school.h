#pragma once

#include "coaches/coach.h"
#include "coaches/coachlogs.h"
#include "players/roster.h"

#include <iostream>
#include <string>
// todo: delete above include

struct SortByAdjustedPublicOvr {
	double adj;

	bool operator()(Coach* a, Coach* b) {
		double aAdjusted = a->getPublicOvr() - ((a->getPublicOvr() - a->getActualOvr()) * (adj / 2));
		double bAdjusted = b->getPublicOvr() - ((b->getPublicOvr() - b->getActualOvr()) * (adj / 2));
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
	City* city;
	std::string mascot;
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
	double rankingScore = 0;
	int ranking = -1;
	int offenseRanking = -1;
	int defenseRanking = -1;
	int lastWeekRanking = -1;

	Coach* coaches[11];
	CoachingLogs coachLogs;
	int recruitingClass[6];

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

	School(std::string na, std::string ma, std::string st, City* ci, int pr, int ss, int bu, int nfl, int ac) :
		name{ na }, mascot{ ma }, state{ st }, city{ ci }, prestige{ pr }, stadiumCapacity{ ss }, budget{ bu }, nflRating{ nfl },
		academicRating{ ac } {
		roster.generateRoster(pr);
		schedule.resize(16, nullptr);
		for (int i = 0; i < 11; i++) coaches[i] = nullptr;
		for (int i = 0; i < 6; i++) recruitingClass[i] = 0;
	}

	std::string getName() { return name; }
	std::string getMascot() { return mascot; }
	City* getCity() { return city; }
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
	void setRanking(int r) {
		lastWeekRanking = ranking;
		ranking = r;
	}
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

	void setRankingScore(double newRankingScore) {
		rankingScore = newRankingScore;
	}

	double getRankingScore() {
		return rankingScore;
	}

	void resetRankingScore() {
		rankingScore = (prestige - 10) * 35;
		lastWeekRanking = -1;
	}

	std::string getRankingChangeString() {
		if (lastWeekRanking < 1 || lastWeekRanking > 25) return "NEW";
		if (lastWeekRanking == ranking) return "-";
		std::string str = (lastWeekRanking > ranking) ? "+" : "-";
		str += std::to_string(std::abs(lastWeekRanking - ranking));
		return str;
	}

	std::string getWeekResultString(int week) {
		if (schedule[week] == nullptr) return "---";
		std::pair<TeamStats*, TeamStats*> stats = getOrderedStats(schedule[week]);
		std::string str;
		if (stats.first->points > stats.second->points) str = "W ";
		else str = "L ";
		str += pickNotMine(this, schedule[week]->away, schedule[week]->home, (School*)nullptr)->getName();
		str += " (" + std::to_string(stats.first->points) + "-" + std::to_string(stats.second->points) + ")";
		return str;
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

	Player* signRecruit(Player* player, int stars) {
		recruitingClass[stars]++;
		return roster.addPlayer(player);
	}

	double getRecruitingMultiplier(Position pos) {
		double multiplier = coaches[(int)CoachType::HC]->getOvrRecruiting() - 40;
		multiplier += coaches[(int)getPositionalCoachType(pos)]->getOvrRecruiting() - 40;
		if (pos == P || pos == K) return multiplier / 118.0;
		multiplier += coaches[(int)getSecondLevelCoachType(pos)]->getOvrRecruiting() - 40;
		return multiplier / 177.0;
	}

	double getDevelopmentMultiplier(Position pos) {
		double multiplier = coaches[(int)CoachType::HC]->getOvrDevelopment() - 40;
		multiplier += coaches[(int)getPositionalCoachType(pos)]->getOvrDevelopment() - 40;
		if (pos == P || pos == K) return multiplier / 118.0;
		multiplier += coaches[(int)getSecondLevelCoachType(pos)]->getOvrDevelopment() - 40;
		return multiplier / 177.0;
	}

	void makeCoachingDecisions() {
		Coach* coach = coaches[(int)CoachType::HC];
		coach->currentContract.yearsRemaining--;
		int minimumPrestige = coach->currentContract.prestigeTargets[0] - coach->currentContract.yearsRemaining;
		if (prestige < minimumPrestige) {
			// Clean house!!
			std::cout << getName() << " has fired their coaching staff (HC: " << coach->getPublicOvr() << "/" << coach->getActualOvr() << ")\n";
			for (int i = 0; i < 11; i++) {
				coaches[i]->resign(true);
				coachLogs.recordFire(coaches[i], (CoachType)i);
				coaches[i] = nullptr;
			}
		} else {
			if (coach->currentContract.yearsRemaining == 0) {
				// Sign extension
				if (coach->currentContract.prestigeTargets.size() > 1)
					coach->currentContract.prestigeTargets.erase(coach->currentContract.prestigeTargets.begin());
				coach->currentContract.yearsTotal = (prestige - minimumPrestige) + 3;
				coach->currentContract.yearsRemaining = coach->currentContract.yearsTotal;
				// Todo: log the extension someplace
				coachLogs.recordExtension(coach, CoachType::HC, coach->currentContract.yearsTotal);
			}
		}
	}

	void signCoach(Coach* newCoach, CoachType type) {
		coaches[(int)type] = newCoach;
		Contract contract;
		int plusOne = (newCoach->getPublicOvr() > 90 ? 1 : 0);
		contract.yearsTotal = (std::rand() % 2) + 3 + plusOne;
		contract.yearsRemaining = contract.yearsTotal;
		contract.originalPrestige = prestige;

		// Now THIS is the fun part: minimum ending prestige
		if (type == CoachType::HC) switch (prestige) {
		case 10: contract.prestigeTargets = { 9, 9, 9 }; break;
		case 9: contract.prestigeTargets = { 8, 9, 9 }; break;
		case 8: contract.prestigeTargets = { 7, 8, 9 }; break;
		case 7: contract.prestigeTargets = { 6, 7, 8 }; break;
		case 6: contract.prestigeTargets = { 5, 6, 7 }; break;
		case 5: contract.prestigeTargets = { 4, 5, 6 }; break;
		case 4: contract.prestigeTargets = { 4, 5, 5 }; break;
		case 3: contract.prestigeTargets = { 3, 4, 4 }; break;
		case 2: contract.prestigeTargets = { 2, 3, 3 }; break;
		case 1:
		case 0: contract.prestigeTargets = { 1, 2, 2 };
		}
		newCoach->currentContract = contract;
		coachLogs.recordHire(newCoach, type, contract.yearsTotal);
	}

	Coach* snipeCoach(std::vector<Coach*> snipeSet, std::vector<Vacancy>& competitors, int myVacancyIndex) {
		// assume the first element of the vector is the coach who originally wanted to sign with us
		Coach* hc = coaches[(int)CoachType::HC];
		Coach* original = snipeSet[0]; // failsafe variable
		if (hc == nullptr) return original;
		double assessmentAbility = (hc->getActualOvr() - 40) / 59.0;
		// Sort into our favorite coaches (yikes on efficiency)
		SortByAdjustedPublicOvr sbao{ assessmentAbility };
		std::sort(snipeSet.begin(), snipeSet.end(), sbao);
		for (Coach* candidate : snipeSet) {
			if (candidate->pickFavoriteJob(competitors) == myVacancyIndex) return candidate;
		}
		return original;
	}

	void loseCoach(CoachType role) {
		coachLogs.recordLoss(coaches[(int)role], role);
		coaches[(int)role] = nullptr;
	}

	std::pair<TeamStats*, TeamStats*> getOrderedStats(Matchup* m) {
		if (this == m->away) {
			return std::make_pair(m->gameResult.awayStats, m->gameResult.homeStats);
		} else
			return std::make_pair(m->gameResult.homeStats, m->gameResult.awayStats);
	}

	void assessSelf() {
		std::vector<int> boundaries{ 4, 9, 16, 26, 41, 56, 76, 96, 116, 126, 131 };
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
		// double allRatio = (0.5 * offRatio) + (0.5 * defRatio);
		for (CoachType position : { CoachType::HC, CoachType::ST }) coaches[(int)position]->givePublicAssessment(rankingRatio);
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
				double bonus = coaches[(int)getPositionalCoachType(p)]->getOvrGametime();
				CoachType t = getSecondLevelCoachType(p);
				if (t != CoachType::ST) bonus += coaches[(int)t]->getOvrGametime();
				bonus += coaches[(int)CoachType::HC]->getOvrGametime();
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
		if (offenseRanking != -1) {
			std::cout << "      Offense ranking: " << offenseRanking << "\n";
			std::cout << "      Defense ranking: " << defenseRanking << "\n";
		}
	}
	void printLatestRecruitingClass() {
		printf("%-20s%-3d%-3d%-3d%-3d%-3d%-3d\n", name.c_str(), recruitingClass[5], recruitingClass[4], recruitingClass[3], recruitingClass[2], recruitingClass[1], recruitingClass[0]);
	}
	void printCoachingStaff() {
		// Print details of each coach
		std::cout << std::endl;
		for (int i = 0; i < 11; i++) {
			Coach* c = coaches[i];
			std::cout << c << "\n";
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
	void printCoachingHistory() { coachLogs.printHiringFiringRecords(); }
	// Normally used in case of scheduling error and need to start again from scratch
	void nukeSchedule() {
		numGamesScheduled = 0;
		conferenceGamesScheduled = 0;
		crossConfGamesScheduled = 0;
		schedule.clear(); // this prolly causes a memory leak
		schedule.resize(16, nullptr);
	}
	void advanceRosterOneYear() {
		for (int i = 0; i < 6; i++) recruitingClass[i] = 0;
		roster.ageAndGraduatePlayers();
		for (Position p : {QB, HB, WR, OL, TE, DL, LB, CB, S, K, P})
			roster.trainPlayersAtPosition(p, getDevelopmentMultiplier(p));
	}
	void prepareNextSeason() {
		ranking = -1;
		offenseRanking = -1;
		defenseRanking = -1;
		simpleRating = 0;
		publicRealRating = 0;
		privateRealRating = 0;
		nukeSchedule();

		advanceRosterOneYear();

		coachLogs.advanceYear();
	}
};
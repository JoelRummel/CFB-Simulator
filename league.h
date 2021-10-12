#pragma once

#include "coachesOrg.h"
#include "loadData.h"
#include "recruitLounge.h"
#include "games/gamePlayer.h"
#include "league/schoolRanker.h"
#include "league/scheduler.h"

struct SortByPrestige {
	bool operator()(School* a, School* b) { return (a->getPrestige() > b->getPrestige()); }
};

struct SortByOffense {
	bool operator()(School* a, School* b) { return (a->getAverageOffenseDefense().first > b->getAverageOffenseDefense().first); }
};

struct SortByDefense {
	bool operator()(School* a, School* b) { return (a->getAverageOffenseDefense().second < b->getAverageOffenseDefense().second); }
};

struct SortByConferenceRecord {
	bool operator()(School* a, School* b) {
		for (bool v : { true, false }) {
			std::pair<int, int> r1 = a->getWinLossRecord(v);
			std::pair<int, int> r2 = b->getWinLossRecord(v);
			if (r1.first > r2.first) return true;
			if (r1.first < r2.first) return false;
			if (r1.second < r2.second) return true;
			if (r1.second > r2.second) return false;

			if (v) {
				// Check the direct matchup between schools
				if (a->didIWinAgainst(b)) return true;
				if (b->didIWinAgainst(a)) return false;
			}
		}
		// shrug
		return (a->getPublicRating() > b->getPublicRating());
	}
};

class League {
private:
	std::vector<std::vector<School>> conferences;        // IMPORTANT: everything actually lives here!
	std::vector<School*> allSchools;

	SchoolRanker schoolRanker;

	Scheduler scheduler;

	CoachesOrganization coachesOrg;

	TopRecruitingClass latestTRC;

	int year = 2020;
	int week = 0;

	void assembleSchoolVector() {
		for (int i = 0; i < (int)conferences.size(); ++i) {
			for (auto& school : conferences[i]) { school.setDivision((Conference)i); }
		}
		for (auto& conference : conferences) {
			for (auto& school : conference) { allSchools.push_back(&school); }
		}
	}

	void sortSchoolVectorByPrestige() {
		SortByPrestige sbp;
		std::sort(allSchools.begin(), allSchools.end(), sbp);
	}

	School* findSchool(Conference conf, std::string name) {
		for (auto& school : conferences[conf]) {
			if (school.getName() == name) return &school;
		}
		assert(false);
	}

	void playEntireSchedule() {
		std::cout << "Playing entire season... ";
		std::cout.flush();
		while (week < 16) {
			playOneWeek();
		}
	}

	void playOneWeek() {
		std::cout << "Playing week " << (week + 1) << "... ";
		std::cout.flush();
		std::vector<School::Matchup*> weekLineup = scheduler.getWeek(week);
		for (auto& matchup : weekLineup) {
			if (matchup->gameResult.homeStats != nullptr) continue;
			GamePlayer game(matchup->away, matchup->home);
			GameResult result = game.startRealTimeGameLoop(false);
			matchup->gameResult = result;
		}
		std::cout << "done." << std::endl;
		week++;
		performNewWeekTasks(week);
	}

	void performNewWeekTasks(int newWeek) {
		if (newWeek <= 14 || newWeek == 16) schoolRanker.rankTeams(newWeek);
		if (newWeek == 13) {
			scheduler.scheduleConferenceChampionshipGames();
			assignOffenseDefenseRankings();
		}
		if (newWeek == 14) scheduler.schedulePlayoffs();
		if (newWeek == 15) scheduler.scheduleFinals();
		if (newWeek == 16) {
			assessAllCoaches();
			std::cout << "\nAll weeks played - season complete." << std::endl;
		}
	}

	GameResult playOneGame(int matchupIndex, bool silent) {
		School::Matchup* matchup = scheduler.getWeek(week)[matchupIndex];
		GamePlayer game(matchup->away, matchup->home);
		GameResult result = game.startRealTimeGameLoop(!silent);
		if (matchup->gameResult.awayStats == nullptr) matchup->gameResult = result;
		return result;
	}



	void assignOffenseDefenseRankings() {
		std::vector<School*> sortedSchools = allSchools;
		SortByOffense sbo;
		std::sort(sortedSchools.begin(), sortedSchools.end(), sbo);
		for (int i = 0; i < (int)sortedSchools.size(); i++) { sortedSchools[i]->setOffenseRanking(i + 1); }
		SortByDefense sbd;
		std::sort(sortedSchools.begin(), sortedSchools.end(), sbd);
		for (int i = 0; i < (int)sortedSchools.size(); i++) { sortedSchools[i]->setDefenseRanking(i + 1); }
	}

	void assessAllCoaches() {
		for (School* school : allSchools) school->assessCoaches();
	}

	void makeCoachContractDecisions() {
		for (School* school : allSchools) school->makeCoachingDecisions();
	}

	School* findSchoolByName(std::string schoolName) {
		School* school = nullptr;
		for (auto& s : allSchools) {
			if (s->getName() == schoolName) {
				school = s;
				break;
			}
		}
		if (school == nullptr) {
			std::cout << "Error - couldn't find a school with that name\n";
			return nullptr;
		}
		return school;
	}

	void initializeSeason() {
		scheduler.clearSchedule();
		bool success = false;
		while (!success) {
			try {
				scheduler.scheduleRegularSeason();
				success = true;
			} catch (std::string e) {
				scheduler.clearSchedule();
				for (auto& school : allSchools) school->nukeSchedule();
			}
		}
		// Tell all schools to fix up depth charts and apply their coach bonuses
		// Also reset their rankings to preseason settings
		for (auto& school : allSchools) {
			school->getRoster()->organizeDepthChart();
			school->applyGametimeBonuses();
		}
		schoolRanker.resetPoll(allSchools);
	}

public:
	void printSchoolResults(std::string schoolName) {
		School* school = findSchoolByName(schoolName);
		if (school == nullptr) return;
		std::pair<int, int> winLossRecord = school->getWinLossRecord();
		std::cout << "Record: " << winLossRecord.first << " - " << winLossRecord.second;
		winLossRecord = school->getWinLossRecord(true);
		std::cout << " (" << winLossRecord.first << " - " << winLossRecord.second << ")\n";
		for (int week = 0; week < 16; ++week) {
			if (week < 13) std::cout << "--Week " << (week + 1) << (week < 9 ? ":  " : ": ");
			else if (week == 13)
				std::cout << "--CCG week:   ";
			else if (week == 14)
				std::cout << "--Bowl week:  ";
			else
				std::cout << "--CFBCG week: ";
			School::Matchup* m = school->getGameResults(week);
			if (m == nullptr) {
				if (week < 13) std::cout << "<bye>\n";
				else
					std::cout << "<not scheduled>\n";
				continue;
			}
			School* opp = pickNotMine(school, m->away, m->home, school);
			std::cout << "vs. " << opp->getName();
			if (m->gameResult.homeStats == nullptr) {
				std::cout << " <not played yet>\n";
				continue;
			}
			TeamStats* myStats = m->gameResult.homeStats;
			TeamStats* theirStats = m->gameResult.awayStats;
			if (opp == m->home) { std::swap(myStats, theirStats); }
			if (myStats->points > theirStats->points) std::cout << " (W) ";
			else if (myStats->points < theirStats->points)
				std::cout << " (L) ";
			else
				std::cout << " (T) ";
			std::cout << myStats->points << " - " << theirStats->points << "\n";
		}
	}

	void printWeekMatchups(int printWeek = -1) {
		if (printWeek == -1) printWeek = week;
		std::vector<School::Matchup*> weekLineup = scheduler.getWeek(printWeek);
		int i = 0;
		for (auto& matchup : weekLineup) {
			i++;
			if (matchup->gameResult.homeStats != nullptr) {
				printf("%2d) %26s vs. %-26s (%d - %d)\n", i, matchup->away->getRankedName().c_str(), matchup->home->getRankedName().c_str(),
					matchup->gameResult.awayStats->points, matchup->gameResult.homeStats->points);
			} else {
				printf("%2d) %26s vs. %-26s\n", i, matchup->away->getRankedName().c_str(), matchup->home->getRankedName().c_str());
			}
		}
	}

	TeamStats* printSchoolGameStats(std::string schoolName, int week) {
		School* school = findSchoolByName(schoolName);
		if (school == nullptr) return nullptr;
		if (school->getGameResults(week) == nullptr) {
			std::cout << "Bye week - no game was played." << std::endl;
			return nullptr;
		}
		school->getMyStats(week)->printBigStuff();
		return school->getMyStats(week);
	}

	TeamStats* getSchoolAggregatedStats(std::string schoolName) {
		School* school = findSchoolByName(schoolName);
		if (school == nullptr) return nullptr;
		TeamStats* aggregate = new TeamStats; // IT IS THE DRIVER'S RESPONSIBILITY TO DELETE THIS
		aggregate->games = 0;
		for (int i = 0; i < 16; i++) {
			School::Matchup* m = school->getGameResults(i);
			if (m != nullptr && m->gameResult.homeStats != nullptr) {
				// alsdfk
				if (school == m->home) *aggregate += *(m->gameResult.homeStats);
				else
					*aggregate += *(m->gameResult.awayStats);
			}
		}
		aggregate->printBigStuff();
		return aggregate;
	}

	bool printGamePlayerStats(TeamStats* stats, int player) {
		if (player >= (int)(stats->getPlayersRecorded().size())) return false;
		stats->printPlayerStats(stats->getPlayersRecorded()[player]);
		return true;
	}

	void printSchoolsByRanking() {
		schoolRanker.printAPTop25(week);
	}

	void printConferenceStandings(std::pair<Conference, Conference> divisions) {
		for (Conference division : { divisions.first, divisions.second }) {
			std::cout << "\n" << divisionName(division) << "\n";
			std::cout << "===========================================\n";
			std::vector<School*> schools;
			for (School& s : conferences[division]) { schools.push_back(&s); }
			SortByConferenceRecord sbcr;
			std::sort(schools.begin(), schools.end(), sbcr);
			for (int i = 0; i < (int)schools.size(); i++) {
				std::pair<int, int> rec = schools[i]->getWinLossRecord(false);
				std::pair<int, int> cRec = schools[i]->getWinLossRecord(true);
				std::string wlstr = std::to_string(rec.first) + " - " + std::to_string(rec.second);
				printf("%2d) %-25s%-7s(%d - %d)\n", i + 1, schools[i]->getRankedName().c_str(), wlstr.c_str(), cRec.first, cRec.second);
			}
			if (divisions.first == divisions.second) return;
		}
	}

	void printSchoolRoster(std::string schoolName) {
		School* school = findSchoolByName(schoolName);
		if (school == nullptr) return;
		school->getRoster()->printRoster();
	}

	void printPositionGroup(std::string schoolName, Position pos) {
		School* school = findSchoolByName(schoolName);
		if (school == nullptr) return;
		school->getRoster()->printPositionGroup(pos);
	}

	void printSchoolDetails(std::string schoolName) {
		School* school = findSchoolByName(schoolName);
		if (school == nullptr) return;
		school->printDetails();
	}

	void printSchoolCoachingStaff(std::string schoolName) {
		School* school = findSchoolByName(schoolName);
		if (school == nullptr) return;
		school->printCoachingStaff();
	}

	void printSchoolCoachingHistory(std::string schoolName) {
		School* school = findSchoolByName(schoolName);
		if (school == nullptr) return;
		school->printCoachingHistory();
	}

	void printLeagueCoaches(CoachType t) { coachesOrg.printCoachesByJobType(t); }

	void printSchoolCoachingByPosition(std::string schoolName, Position p) {
		School* school = findSchoolByName(schoolName);
		if (school == nullptr) return;
		school->printCoachingImpact(p);
	}

	void printCoachHistoryByName(std::string coachName) { coachesOrg.printCoachHistoryByName(coachName); }

	void printRecruitingSummary() {
		std::cout << "School              5★ 4★ 3★ 2★ 1★ 0★\n";
		std::cout << "=====================================\n";
		for (auto school : allSchools) school->printLatestRecruitingClass();
		std::cout << "\n";
		for (int i = 0; i < 10; i++) {
			/**
			 *  1) Jaylen Edwards       Athens, MI        LB 80 OVR
			 *     School: Ohio State         Top Priority: Academics
			 */
			Recruit& r = latestTRC.recruits[i];
			Player* p = r.getUnderlyingPlayer();
			printf("%2d) %-20s %-20s %-2s %-2d OVR\n", i + 1, p->getName().c_str(), p->getHometown()->formalName().c_str(), positionToStr(p->getPosition()).c_str(), p->getOVR());
			printf("    School: %-12s Top Priority: %s\n\n", latestTRC.schoolChoices[i]->getName().c_str(), r.getTopPreferenceStr().c_str());
		}
	}

	void simSeason() {
		playEntireSchedule();
	}

	void simOneWeek() { playOneWeek(); }

	bool simOneGame(int gameIndex, bool replay) {
		if (scheduler.getWeek(week)[gameIndex - 1]->gameResult.homeStats != nullptr && !replay) {
			return false;
		} else {
			playOneGame(gameIndex - 1, false);
			return true;
		}
	}

	bool simOneGame(std::string schoolName, bool replay) {
		int i = 0;
		for (auto& matchup : scheduler.getWeek(week)) {
			if (matchup->away->getName() == schoolName || matchup->home->getName() == schoolName) {
				if (matchup->gameResult.homeStats == nullptr || replay) playOneGame(i, false);
				else
					return false;
				return true;
			}
			i++;
		}
		std::cout << "Error: no school with that name\n";
		return true;
	}

	void simOneGameRepeatedly(int gameIndex, int numTimes) {
		std::cout << "Simulating";
		GameResult collectiveResult;
		int homeWins = 0;
		int awayWins = 0;
		for (int i = 0; i < numTimes; i++) {
			if (i % 10 == 0) {
				std::cout << ".";
				std::cout.flush();
			}
			GameResult result = playOneGame(gameIndex - 1, true);
			if (!result.awayWon && !result.homeWon) {
				i--;
				continue;
			}
			if (result.awayWon) awayWins++;
			else
				homeWins++;
			if (collectiveResult.awayStats == nullptr) collectiveResult = result;
			else
				collectiveResult += result;
		}
		std::cout << " done." << std::endl;
		std::cout << "\nAway - home wins: " << awayWins << " - " << homeWins << "\n";
		printf("Away won %.1f%% of the time\n\n", ((double)awayWins / numTimes) * 100.0);
		std::cout << "AWAY STATS:\n";
		collectiveResult.awayStats->printBigStuff();
		std::cout << "\nHOME STATS:\n";
		collectiveResult.homeStats->printBigStuff();
	}

	void simOneGameRepeatedly(std::string schoolName, int numTimes) {
		int i = 0;
		for (auto& matchup : scheduler.getWeek(week)) {
			if (matchup->away->getName() == schoolName || matchup->home->getName() == schoolName) { simOneGameRepeatedly(i + 1, numTimes); }
			i++;
		}
	}

	void prepareNextSeason() {
		makeCoachContractDecisions();
		coachesOrg.advanceYear();
		coachesOrg.fillAllVacancies(allSchools);

		for (School* school : allSchools) school->prepareNextSeason();
		RecruitLounge recruits;
		recruits.generateNewRecruitingClass();
		latestTRC = recruits.signRecruitingClass(allSchools);

		year++;
		week = 0;

		sortSchoolVectorByPrestige();
		initializeSeason();
	}

	int getCurrentWeek() { return week + 1; }
	int getCurrentYear() { return year; }

	League() {
		coachesOrg.initializeAllCoaches();

		conferences.resize(20);

		for (auto sd : GlobalData::getSchoolsData()) {
			City* city = GlobalData::getCityByName(GlobalData::stateNameToCode(sd.state), sd.city);
			if (city == nullptr) {
				std::cout << sd.name << " could not find city: " << sd.city << ", " << sd.state << std::endl;
				exit(1);
			}
			conferences[sd.division].emplace_back(sd.name, sd.mascot, sd.state, city, sd.prestige, sd.stadiumCapacity, sd.budget, sd.nflRating,
				sd.academicRating);
		}

		assembleSchoolVector();
		sortSchoolVectorByPrestige();

		coachesOrg.fillAllVacancies(allSchools);

		// Let's do a proper initialization
		for (int i = 0; i < 4; i++) {
			for (auto& school : allSchools) school->advanceRosterOneYear();
			RecruitLounge recruits;
			recruits.generateNewRecruitingClass();
			latestTRC = recruits.signRecruitingClass(allSchools);
		}

		scheduler.setSchools(&conferences, allSchools);
		initializeSeason();
	}
};

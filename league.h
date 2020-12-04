#pragma once

#include "coachesOrg.h"
#include "gamePlayer.h"
#include "loadData.h"

bool areSameConference(Conference div1, Conference div2) {
	if (div1 == div2) return true;
	return (getOppositeDivision(div1) == div2);
}

struct SortByPrestige {
	bool operator()(School* a, School* b) { return (a->getPrestige() > b->getPrestige()); }
};

struct SortByPublicRating {
	bool operator()(School* a, School* b) { return (a->getPublicRating() > b->getPublicRating()); }
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
	std::vector<std::vector<School::Matchup*>> schedule; // gets resized to 13

	std::vector<School*> allSchools;

	CoachesOrganization coachesOrg;

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

	void assignMatchup(int week, School* away, School* home) {
		School::Matchup* ptr = new School::Matchup { away, home };
		schedule[week].push_back(ptr);
		bool confGame = false;
		bool crossConfGame = false;
		if (away->getDivision() == home->getDivision()) confGame = true;
		else if (areSameConference(away->getDivision(), home->getDivision()))
			crossConfGame = true;
		away->assignGame(week, ptr, confGame, crossConfGame);
		home->assignGame(week, ptr, confGame, crossConfGame);
	}

	void assignMatchup(School* away, School* home) { assignMatchup(findLatestOpenWeek(away, home), away, home); }

	School* findSchool(Conference conf, std::string name) {
		for (auto& school : conferences[conf]) {
			if (school.getName() == name) return &school;
		}
		assert(false);
	}

	int findLatestOpenWeek(School* s1, School* s2) {
		for (int week = 12; week >= 0; --week) {
			if (s1->getScheduledOpponent(week) == nullptr && s2->getScheduledOpponent(week) == nullptr) return week;
		}
		return -1;
	}

	int findEarliestOpenWeek(School* s1, School* s2) {
		for (int week = 0; week < 13; ++week) {
			if (s1->getScheduledOpponent(week) == nullptr && s2->getScheduledOpponent(week) == nullptr) return week;
		}
		return -1;
	}

	void scheduleConferenceGame(School* school, int maxConfGames, bool oppDiv = false, School* exemption = nullptr) {
		Conference div = school->getDivision();
		if (oppDiv) div = getOppositeDivision(div);
		// first, get all schools within conference who haven't been scheduled w/ this school
		std::vector<School>& otherSchools = conferences[div];
		std::vector<School*> validChoices;
		for (auto& choice : otherSchools) {
			if (choice.getName() != school->getName() && !school->isOnSchedule(&choice)) {
				bool good = true;
				if (oppDiv && choice.getCrossConfGamesScheduled() >= maxConfGames) good = false;
				if (!oppDiv && choice.getConferenceGamesScheduled() >= maxConfGames) good = false;
				if (oppDiv && choice.getCrossConfGamesScheduled() > school->getCrossConfGamesScheduled()) good = false;
				if (exemption != nullptr && &choice == exemption) good = false;
				if (good) validChoices.push_back(&choice);
			}
		}
		while (validChoices.size() == 0) {
			assert(oppDiv);
			for (auto& choice : otherSchools) {
				if (!school->isOnSchedule(&choice) && findLatestOpenWeek(school, &choice) > -1) { validChoices.push_back(&choice); }
			}
			School* delinkTarget = *select_randomly(validChoices.begin(), validChoices.end());
			validChoices = { delinkTarget };
			std::vector<School*> victimCandidates;
			for (int week = 0; week < 13; ++week) {
				School* candidate = delinkTarget->getScheduledOpponent(week);
				if (candidate != nullptr && candidate->getDivision() == getOppositeDivision(delinkTarget->getDivision()))
					victimCandidates.push_back(candidate);
			}
			School* victim = *select_randomly(victimCandidates.begin(), victimCandidates.end());
			for (int week = 0; week < 13; ++week) {
				if (victim->getScheduledOpponent(week) == delinkTarget) {
					victim->assignGame(week, nullptr);
					delinkTarget->assignGame(week, nullptr);
				}
			}
			scheduleCrossDivGame(victim, maxConfGames, delinkTarget);
		}
		School* opponent = *select_randomly(validChoices.begin(), validChoices.end());
		int week = findLatestOpenWeek(school, opponent);
		assignMatchup(week, opponent, school);
	}

	void scheduleCrossDivGame(School* school, int maxConfGames, School* exemption = nullptr) {
		scheduleConferenceGame(school, maxConfGames, true, exemption);
	}

	void scheduleNonConGame(School* school) {
		std::vector<School*> randomSchools = allSchools;
		RNG::shuffle(randomSchools);
		for (auto& candidate : randomSchools) {
			if (school == candidate) continue;
			if (school->getDivision() == candidate->getDivision()) continue;
			if (school->getDivision() == getOppositeDivision(candidate->getDivision())) continue;
			if (candidate->getNumGamesScheduled() >= 12) continue;
			if (school->strategy.wantSchool(candidate) && candidate->strategy.wantSchool(school)) {
				int week = findEarliestOpenWeek(school, candidate);
				if (week >= 0) {
					assignMatchup(week, candidate, school);
					school->strategy.registerSchedule(candidate);
					candidate->strategy.registerSchedule(candidate);
					return;
				}
			}
		}
		// nobody we liked. just take the next guy available
		for (auto& candidate : randomSchools) {
			if (school == candidate) continue;
			if (school->getDivision() == candidate->getDivision()) continue;
			if (school->getDivision() == getOppositeDivision(candidate->getDivision())) continue;
			if (candidate->getNumGamesScheduled() >= 12) continue;
			int week = findEarliestOpenWeek(school, candidate);
			if (week >= 0) {
				assignMatchup(week, candidate, school);
				// no need to notify the strategies, it already sucks for them
				return;
			}
		}
		std::cout << "ALERT: FCS team needs to be scheduled for " << school->getName() << "\n";
	}

	void scheduleConferenceChampionshipGames() {
		// B1G
		for (std::vector<Conference> set : { std::vector<Conference> { BIGTENEAST, BIGTENWEST }, std::vector<Conference> { SECEAST, SECWEST },
											 std::vector<Conference> { ACCATLANTIC, ACCCOASTAL } }) {
			std::vector<School*> representatives;
			for (Conference conf : set) {
				int mostWins = 0;
				for (auto& school : conferences[conf]) {
					int schoolWins = school.getWinLossRecord(true).first;
					if (schoolWins > mostWins) mostWins = schoolWins;
				}
				std::vector<School*> tiedSchools;
				for (auto& school : conferences[conf]) {
					if (school.getWinLossRecord(true).first == mostWins) tiedSchools.push_back(&school);
				}
				if (tiedSchools.size() >= 3) {
					std::vector<int> wins;
					for (auto& school : tiedSchools) {
						int ourWins = 0;
						for (auto& opponent : tiedSchools) {
							if (opponent != school && school->didIWinAgainst(opponent)) ourWins++;
						}
						wins.push_back(ourWins);
					}
					mostWins = 0;
					for (auto& w : wins)
						if (w > mostWins) mostWins = w;
					for (int i = (int)tiedSchools.size() - 1; i >= 0; i--) {
						if (wins[i] < mostWins) tiedSchools.erase(tiedSchools.begin() + i);
					}
				}
				if (tiedSchools.size() == 2) {
					if (tiedSchools[0]->didIWinAgainst(tiedSchools[1])) tiedSchools.erase(tiedSchools.begin() + 1);
					else
						tiedSchools.erase(tiedSchools.begin());
				}
				School* representative = tiedSchools[0];
				if (tiedSchools.size() > 1) {
					std::cout << "WARNING - tiebreakers failed for B1G CCG, using random draw\n";
					representative = *select_randomly(tiedSchools.begin(), tiedSchools.end());
				}
				representatives.push_back(representative);
			}
			assignMatchup(13, representatives[0], representatives[1]);
		}
	}

	void schedulePlayoffs() {
		std::vector<School*> fourTeams;
		int i = 0;
		while (fourTeams.size() < 4) {
			bool playedATeam = false;
			for (int j = 0; j < (int)fourTeams.size(); j++) {
				if (allSchools[i]->getMatchupAgainst(fourTeams[j]) != nullptr) {
					playedATeam = true;
					break;
				}
			}
			if (!playedATeam) fourTeams.push_back(allSchools[i]);
			i++;
		}
		assignMatchup(14, fourTeams[3], fourTeams[0]);
		assignMatchup(14, fourTeams[2], fourTeams[1]);
	}

	void scheduleFinals() {
		// Find the two semifinals from week 14 (week 15 if 1-indexed)
		assert(schedule[14][0]->gameResult.awayStats != nullptr);
		assert(schedule[14][1]->gameResult.homeStats != nullptr);
		School* winner1;
		School* winner2;
		if (schedule[14][0]->gameResult.awayWon) winner1 = schedule[14][0]->away;
		else
			winner1 = schedule[14][0]->home;
		if (schedule[14][1]->gameResult.awayWon) winner2 = schedule[14][1]->away;
		else
			winner2 = schedule[14][1]->home;

		assignMatchup(15, winner2, winner1); // DUN DUN DUN
	}

	void createMatchups() {
		// protected matchups first
		assignMatchup(12, findSchool(BIGTENEAST, "Michigan"), findSchool(BIGTENEAST, "Ohio State"));
		assignMatchup(12, findSchool(SECWEST, "Auburn"), findSchool(SECWEST, "Alabama"));
		assignMatchup(12, findSchool(BIGTENWEST, "Purdue"), findSchool(BIGTENEAST, "Indiana"));
		School* usc = findSchool(SECEAST, "South Carolina");
		School* clem = findSchool(ACCATLANTIC, "Clemson");
		assignMatchup(12, usc, clem);
		usc->strategy.registerSchedule(clem);
		clem->strategy.registerSchedule(usc);
		// lots more to do in the future
		for (int passThru = 0; passThru < 13; ++passThru) {
			for (auto& school : allSchools) {
				if (school->getNumGamesScheduled() > passThru) continue;
				if (school->getNumGamesScheduled() == 12) continue;
				// All behavior depends on conference
				const Conference div = school->getDivision();
				const Conference oppDiv = getOppositeDivision(div);
				const int confGames = school->getConferenceGamesScheduled();
				const int crossConfGames = school->getCrossConfGamesScheduled();
				std::string name = school->getName();
				if (div == BIGTENEAST || div == BIGTENWEST) {
					if (confGames < 6) {
						// In-conference BIG10
						scheduleConferenceGame(school, 6);
					} else if (crossConfGames < 3) {
						// Cross-conference BIG10
						if (crossConfGames == 0) {
							// Protected matchup
							// lmao kill me
							std::string opponent = "";
							std::vector<std::pair<std::string, std::string>> pairings { std::make_pair("Maryland", "Minnesota"),
																						std::make_pair("Michigan", "Wisconsin"),
																						std::make_pair("Michigan State", "Northwestern"),
																						std::make_pair("Ohio State", "Nebraska"),
																						std::make_pair("Penn State", "Iowa"),
																						std::make_pair("Rutgers", "Illinois") };
							opponent = pickMyPartner(pairings, name);
							assignMatchup(findSchool(oppDiv, opponent), school);
						} else
							scheduleCrossDivGame(school, 3);
					} else {
						scheduleNonConGame(school);
					}
				} else if (div == SECEAST || div == SECWEST) {
					if (confGames < 6) {
						// In-conference SEC
						scheduleConferenceGame(school, 6);
					} else if (crossConfGames < 2) {
						// Cross-conference SEC
						if (crossConfGames == 0) {
							// Protected matchup
							std::string opponent = "";
							std::vector<std::pair<std::string, std::string>> pairings { std::make_pair("Alabama", "Tennessee"),
																						std::make_pair("Arkansas", "Missouri"),
																						std::make_pair("Auburn", "Georgia"),
																						std::make_pair("LSU", "Florida"),
																						std::make_pair("Mississippi State", "Kentucky"),
																						std::make_pair("Ole Miss", "Vanderbilt"),
																						std::make_pair("Texas A&M", "South Carolina") };
							opponent = pickMyPartner(pairings, name);
							assignMatchup(findSchool(oppDiv, opponent), school);
						} else
							scheduleCrossDivGame(school, 2);
					} else {
						// Out-of-conference SEC
						scheduleNonConGame(school);
					}
				} else if (div == ACCATLANTIC || div == ACCCOASTAL) {
					if (confGames < 6) {
						// In-conference ACC
						scheduleConferenceGame(school, 6);
					} else if (crossConfGames < 2) {
						// Cross-conference ACC
						if (crossConfGames == 0) {
							// Protected matchup
							// lmao kill me
							std::string opponent = "";
							std::vector<std::pair<std::string, std::string>> pairings { std::make_pair("Boston College", "Virginia Tech"),
																						std::make_pair("Clemson", "Georgia Tech"),
																						std::make_pair("Florida State", "Miami (FL)"),
																						std::make_pair("Louisville", "Virginia"),
																						std::make_pair("NC State", "North Carolina"),
																						std::make_pair("Syracuse", "Pittsburgh"),
																						std::make_pair("Wake Forest", "Duke") };
							opponent = pickMyPartner(pairings, name);
							assignMatchup(findSchool(oppDiv, opponent), school);
						} else
							scheduleCrossDivGame(school, 2);
					} else {
						// Out-of-conference BIG10
						scheduleNonConGame(school);
					}
				} else if (div == BIG12) {
					if (confGames < 9) {
						// In-conference ACC
						scheduleConferenceGame(school, 9);
					} else {
						// Out-of-conference BIG10
						scheduleNonConGame(school);
					}
				} else if (div == PAC12NORTH || div == PAC12SOUTH) {
					if (confGames < 5) {
						// In-conference PAC12
						scheduleConferenceGame(school, 5);
					} else if (crossConfGames < 4) {
						// Cross-conference PAC12
						// PAC12 is weird and has weird cross-div scheduling rules
						School* opponent;
						std::vector<std::string> choices;
						if (name == "California" || name == "Stanford" || name == "USC" || name == "UCLA") {
							if (crossConfGames < 2) {
								if (div == PAC12NORTH) choices = { "USC", "UCLA" };
								else
									choices = { "California", "Stanford" };
							} else {
								if (div == PAC12SOUTH) choices = { "Oregon", "Washington", "Washington State", "Oregon State" };
								else
									choices = { "Utah", "Arizona State", "Colorado", "Arizona" };
							}
						} else {
							// non-cali
							if (crossConfGames < 3) {
								if (div == PAC12NORTH) choices = { "Utah", "Arizona State", "Colorado", "Arizona" };
								else
									choices = { "Oregon", "Washington", "Washington State", "Oregon State" };
							} else {
								if (div == PAC12NORTH) choices = { "USC", "UCLA" };
								else
									choices = { "California", "Stanford" };
							}
						}
						int choice = std::rand() % choices.size();
						opponent = findSchool(oppDiv, choices[choice]);
						while (school->isOnSchedule(opponent)) {
							choice = (choice + 1) % choices.size();
							opponent = findSchool(oppDiv, choices[choice]);
						}
						assignMatchup(opponent, school);
					} else {
						// Out-of-conference PAC12
						scheduleNonConGame(school);
					}
				} else if (div == MACEAST || div == MACWEST || div == AACEAST || div == AACWEST || div == MWCMOUNTAIN || div == MWCWEST) {
					if (confGames < 5) {
						scheduleConferenceGame(school, 5);
					} else if (crossConfGames < 3) {
						scheduleCrossDivGame(school, 3);
					} else
						scheduleNonConGame(school);
				} else if (div == SUNBELTEAST || div == SUNBELTWEST) {
					if (confGames < 4) scheduleConferenceGame(school, 4);
					else if (crossConfGames < 4)
						scheduleCrossDivGame(school, 4);
					else
						scheduleNonConGame(school);
				} else if (div != INDEPENDENT) {
					// 7-person CUSA
					if (confGames < 6) scheduleConferenceGame(school, 6);
					else if (crossConfGames < 2)
						scheduleCrossDivGame(school, 2);
					else
						scheduleNonConGame(school);
				}
			}
		}
	}

	void playEntireSchedule() {
		std::cout << "Playing entire season... ";
		std::cout.flush();
		for (week; week < (int)schedule.size(); week++) {
			std::vector<School::Matchup*>& weekLineup = schedule[week];
			for (auto& matchup : weekLineup) {
				if (matchup->gameResult.homeStats != nullptr) continue;
				GamePlayer game(matchup->away, matchup->home);
				GameResult result = game.startRealTimeGameLoop(false);
				matchup->gameResult = result;
			}
			std::cout << "week " << (week + 1) << " done... ";
			std::cout.flush();
			if (week == 12) scheduleConferenceChampionshipGames();
			if (week == 13) {
				rankTeams();
				schedulePlayoffs();
			}
			if (week == 14) scheduleFinals();
		}
		std::cout << "\nAll weeks played - season complete." << std::endl;

		// Print final results
		std::cout << "\n";
		for (auto& school : allSchools) {
			std::pair<int, int> record = school->getWinLossRecord();
			std::cout << school->getName() << ": " << record.first << " - " << record.second << " .... " << school->getAverageOffense()
					  << " yds/avg\n";
		}
	}

	void playOneWeek() {
		std::cout << "Playing week " << (week + 1) << "... ";
		std::cout.flush();
		std::vector<School::Matchup*>& weekLineup = schedule[week];
		for (auto& matchup : weekLineup) {
			if (matchup->gameResult.homeStats != nullptr) continue;
			GamePlayer game(matchup->away, matchup->home);
			GameResult result = game.startRealTimeGameLoop(false);
			matchup->gameResult = result;
		}
		std::cout << "done." << std::endl;
		week++;
		if (week >= 3 && (week <= 14 || week == 16)) rankTeams();
		if (week == 13) scheduleConferenceChampionshipGames();
		if (week == 14) schedulePlayoffs();
		if (week == 15) scheduleFinals();
	}

	void playOneGame(int matchupIndex) {
		School::Matchup* matchup = schedule[week][matchupIndex];
		GamePlayer game(matchup->away, matchup->home);
		GameResult result = game.startRealTimeGameLoop(true);
		matchup->gameResult = result;
	}

	void rankTeams() {
		for (auto& school : allSchools) { school->setSimpleRating(); }
		for (int i = 0; i < 50; ++i) {
			for (auto& school : allSchools) { school->updatePrivateRating(); }
			for (auto& school : allSchools) { school->publishPrivateRating(); }
		}
		SortByPublicRating sbr;
		std::sort(allSchools.begin(), allSchools.end(), sbr);
		for (int i = 0; i < (int)allSchools.size(); i++) { allSchools[i]->setRanking(i + 1); }
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
	}

	void initializeSeason() {
		createMatchups();
		// Tell all schools to fix up depth charts
		for (auto& school : allSchools) { school->getRoster()->organizeDepthChart(); }
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
		std::vector<School::Matchup*> weekLineup = schedule[printWeek];
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
		for (int i = 0; i < 25; ++i) {
			std::cout << "#" << (i + 1) << ". " << allSchools[i]->getName() << "\n"; //"  ---  " << allSchools[i]->getPublicRating() << "\n";
		}
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

	League() {
		coachesOrg.initializeAllCoaches();

		schedule.resize(16); // thirteen total regular season weeks, one bye per team + 2 bowl weeks, 1 CCG week

		conferences.resize(20);

		for (auto sd : GlobalData::getSchoolsData()) {
			conferences[sd.division].emplace_back(sd.name, sd.mascot, sd.state, sd.city, sd.prestige, sd.stadiumCapacity, sd.budget, sd.nflRating,
												  sd.academicRating);
		}

		assembleSchoolVector();
		sortSchoolVectorByPrestige();

		coachesOrg.fillAllVacancies(allSchools);

		initializeSeason();
	}

	void simSeason() {
		playEntireSchedule();
		rankTeams();
	}

	void simOneWeek() { playOneWeek(); }

	void simOneGame(int gameIndex) {
		if (schedule[week][gameIndex - 1]->gameResult.homeStats != nullptr) {
			std::cout << "Error: this game has already been played!" << std::endl;
		} else {
			playOneGame(gameIndex - 1);
		}
	}

	void simOneGame(std::string schoolName) {
		int i = 0;
		for (auto& matchup : schedule[week]) {
			if (matchup->away->getName() == schoolName || matchup->home->getName() == schoolName) {
				playOneGame(i);
				return;
			}
			i++;
		}
	}

	void prepareNextSeason() {
		coachesOrg.advanceYear();
		for (School* school : allSchools) school->prepareNextSeason();
		year++;
		week = 0;
		schedule.clear();
		schedule.resize(16);
		sortSchoolVectorByPrestige();
		initializeSeason();
	}

	int getCurrentWeek() { return week + 1; }
};
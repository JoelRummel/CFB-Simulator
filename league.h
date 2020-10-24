#pragma once

#include "gamePlayer.h"
#include "school.h"

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

class League {
  private:
	std::vector<std::vector<School>> conferences; // IMPORTANT: everything actually lives here!
	std::vector<std::vector<School::Matchup*>> schedule; // gets resized to 13

	std::vector<School*> allSchools;

	void assembleSchoolVector() {
		for (int i = 0; i < (int)conferences.size(); ++i) {
			for (auto& school : conferences[i]) { school.setDivision((Conference)i); }
		}
		for (auto& conference : conferences) {
			for (auto& school : conference) { allSchools.push_back(&school); }
		}
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

	void assignMatchup(School* away, School* home) {
		assignMatchup(findLatestOpenWeek(away, home), away, home);
	}

	School* findSchool(Conference conf, std::string name) {
		for (auto& school : conferences[conf]) {
			if (school.getName() == name) return &school;
		}
		assert(false);
	}

	int findLatestOpenWeek(School* s1, School* s2) {
		for (int week = 12; week >= 0; --week) {
			if (s1->getScheduledOpponent(week) == nullptr &&
				s2->getScheduledOpponent(week) == nullptr)
				return week;
		}
		return -1;
	}

	int findEarliestOpenWeek(School* s1, School* s2) {
		for (int week = 0; week < 13; ++week) {
			if (s1->getScheduledOpponent(week) == nullptr &&
				s2->getScheduledOpponent(week) == nullptr)
				return week;
		}
		return -1;
	}

	void scheduleConferenceGame(School* school, int maxConfGames, bool oppDiv = false,
								School* exemption = nullptr) {
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
				if (oppDiv &&
					choice.getCrossConfGamesScheduled() > school->getCrossConfGamesScheduled())
					good = false;
				if (exemption != nullptr && &choice == exemption) good = false;
				if (good) validChoices.push_back(&choice);
			}
		}
		while (validChoices.size() == 0) {
			assert(oppDiv);
			for (auto& choice : otherSchools) {
				if (!school->isOnSchedule(&choice) && findLatestOpenWeek(school, &choice) > -1) {
					validChoices.push_back(&choice);
				}
			}
			School* delinkTarget = *select_randomly(validChoices.begin(), validChoices.end());
			validChoices = { delinkTarget };
			std::vector<School*> victimCandidates;
			for (int week = 0; week < 13; ++week) {
				School* candidate = delinkTarget->getScheduledOpponent(week);
				if (candidate != nullptr &&
					candidate->getDivision() == getOppositeDivision(delinkTarget->getDivision()))
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

	void createMatchups() {
		// protected matchups first
		std::cout << "Creating season schedule...";
		std::cout.flush();
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
							std::vector<std::pair<std::string, std::string>> pairings {
								std::make_pair("Maryland", "Minnesota"),
								std::make_pair("Michigan", "Wisconsin"),
								std::make_pair("Michigan State", "Northwestern"),
								std::make_pair("Ohio State", "Nebraska"),
								std::make_pair("Penn State", "Iowa"),
								std::make_pair("Rutgers", "Illinois")
							};
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
							std::vector<std::pair<std::string, std::string>> pairings {
								std::make_pair("Alabama", "Tennessee"),
								std::make_pair("Arkansas", "Missouri"),
								std::make_pair("Auburn", "Georgia"),
								std::make_pair("LSU", "Florida"),
								std::make_pair("Mississippi State", "Kentucky"),
								std::make_pair("Ole Miss", "Vanderbilt"),
								std::make_pair("Texas A&M", "South Carolina")
							};
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
							std::vector<std::pair<std::string, std::string>> pairings {
								std::make_pair("Boston College", "Virginia Tech"),
								std::make_pair("Clemson", "Georgia Tech"),
								std::make_pair("Florida State", "Miami (FL)"),
								std::make_pair("Louisville", "Virginia"),
								std::make_pair("North Carolina State", "North Carolina"),
								std::make_pair("Syracuse", "Pitt"),
								std::make_pair("Wake Forest", "Duke")
							};
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
						if (name == "California" || name == "Stanford" || name == "USC" ||
							name == "UCLA") {
							if (crossConfGames < 2) {
								if (div == PAC12NORTH) choices = { "USC", "UCLA" };
								else
									choices = { "California", "Stanford" };
							} else {
								if (div == PAC12SOUTH)
									choices = { "Oregon", "Washington", "Washington State",
												"Oregon State" };
								else
									choices = { "Utah", "Arizona State", "Colorado", "Arizona" };
							}
						} else {
							// non-cali
							if (crossConfGames < 3) {
								if (div == PAC12NORTH)
									choices = { "Utah", "Arizona State", "Colorado", "Arizona" };
								else
									choices = { "Oregon", "Washington", "Washington State",
												"Oregon State" };
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
				} else if (div == MACEAST || div == MACWEST || div == AACEAST || div == AACWEST ||
						   div == MWCMOUNTAIN || div == MWCWEST) {
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
		std::cout << "done." << std::endl;
	}

	void playSchedule() {
		std::cout << "Playing schedule... ";
		std::cout.flush();
		int weekCount = 0;
		for (auto& week : schedule) {
			weekCount++;
			// std::cout << "\nNEXT WEEK\n===================================\n";
			for (auto& matchup : week) {
				GamePlayer game(matchup->away, matchup->home);
				GameResult result = game.startRealTimeGameLoop();
				matchup->gameResult = result;
			}
			std::cout << "week " << weekCount << " done... ";
			std::cout.flush();
		}
		std::cout << "\nAll weeks played - season complete." << std::endl;

		// Print final results
		std::cout << "\n";
		for (auto& school : allSchools) {
			std::pair<int, int> record = school->getWinLossRecord();
			std::cout << school->getName() << ": " << record.first << " - " << record.second
					  << " .... " << school->getAverageOffense() << " yds/avg\n";
		}
	}

	void rankTeams() {
		for (auto& school : allSchools) { school->setSimpleRating(); }
		for (int i = 0; i < 50; ++i) {
			for (auto& school : allSchools) { school->updatePrivateRating(); }
			for (auto& school : allSchools) { school->publishPrivateRating(); }
		}
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

  public:
	void printSchoolResults(std::string schoolName) {
		School* school = findSchoolByName(schoolName);
		if (school == nullptr) return;
		std::pair<int, int> winLossRecord = school->getWinLossRecord();
		std::cout << "Record: " << winLossRecord.first << " - " << winLossRecord.second;
		winLossRecord = school->getWinLossRecord(true);
		std::cout << " (" << winLossRecord.first << " - " << winLossRecord.second << ")\n";
		for (int week = 0; week < 13; ++week) {
			std::cout << "--Week " << (week + 1) << ": ";
			School::Matchup* m = school->getGameResults(week);
			if (m == nullptr) {
				std::cout << "<bye>\n";
				continue;
			}
			School* opp = pickNotMine(school, m->away, m->home, school);
			std::cout << "vs. " << opp->getName();
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

	void printSchoolGameStats(std::string schoolName, int week) {
		School* school = findSchoolByName(schoolName);
		if (school == nullptr) return;
		if (school->getGameResults(week) == nullptr) {
			std::cout << "Bye week - no game was played." << std::endl;
			return;
		}
		school->getMyStats(week)->printBigStuff();
	}

	void printSchoolsByRanking() {
		std::vector<School*> sortedSchools = allSchools;
		SortByPublicRating sbr;
		std::sort(sortedSchools.begin(), sortedSchools.end(), sbr);
		for (int i = 0; i < (int)sortedSchools.size(); ++i) {
			std::cout << "#" << (i + 1) << ". " << sortedSchools[i]->getName() << "  ---  "
					  << sortedSchools[i]->getPublicRating() << "\n";
		}
	}

	void printSchoolRoster(std::string schoolName) {
		School* school = findSchoolByName(schoolName);
		if (school == nullptr) return;
		school->getRoster()->printRoster();
	}

	League() {
		schedule.resize(13); // thirteen total regular season weeks, one bye per team

		conferences.resize(20);
		std::vector<School>& bigTenEast = conferences[BIGTENEAST];
		bigTenEast.emplace_back("Michigan", 8);
		bigTenEast.emplace_back("Ohio State", 10);
		bigTenEast.emplace_back("Michigan State", 6);
		bigTenEast.emplace_back("Penn State", 9);
		bigTenEast.emplace_back("Maryland", 4);
		bigTenEast.emplace_back("Indiana", 5);
		bigTenEast.emplace_back("Rutgers", 2);

		std::vector<School>& bigTenWest = conferences[BIGTENWEST];
		bigTenWest.emplace_back("Wisconsin", 8);
		bigTenWest.emplace_back("Minnesota", 7);
		bigTenWest.emplace_back("Purdue", 4);
		bigTenWest.emplace_back("Nebraska", 5);
		bigTenWest.emplace_back("Illinois", 5);
		bigTenWest.emplace_back("Iowa", 7);
		bigTenWest.emplace_back("Northwestern", 3);

		std::vector<School>& secEast = conferences[SECEAST];
		secEast.emplace_back("Florida", 8);
		secEast.emplace_back("Georgia", 9);
		secEast.emplace_back("Kentucky", 6);
		secEast.emplace_back("Tennessee", 5);
		secEast.emplace_back("Missouri", 4);
		secEast.emplace_back("South Carolina", 4);
		secEast.emplace_back("Vanderbilt", 3);

		std::vector<School>& secWest = conferences[SECWEST];
		secWest.emplace_back("Alabama", 10);
		secWest.emplace_back("LSU", 9);
		secWest.emplace_back("Auburn", 8);
		secWest.emplace_back("Texas A&M", 6);
		secWest.emplace_back("Mississippi State", 5);
		secWest.emplace_back("Ole Miss", 4);
		secWest.emplace_back("Arkansas", 3);

		std::vector<School>& big12 = conferences[BIG12];
		big12.emplace_back("Oklahoma", 9);
		big12.emplace_back("Texas", 7);
		big12.emplace_back("Baylor", 7);
		big12.emplace_back("Oklahoma State", 6);
		big12.emplace_back("Kansas State", 6);
		big12.emplace_back("Iowa State", 5);
		big12.emplace_back("Texas Christian", 5);
		big12.emplace_back("West Virginia", 3);
		big12.emplace_back("Texas Tech", 4);
		big12.emplace_back("Kansas", 2);

		std::vector<School>& accA = conferences[ACCATLANTIC];
		accA.emplace_back("Clemson", 10);
		accA.emplace_back("Louisville", 5);
		accA.emplace_back("Wake Forest", 5);
		accA.emplace_back("Florida State", 4);
		accA.emplace_back("Boston College", 4);
		accA.emplace_back("Syracuse", 4);
		accA.emplace_back("North Carolina State", 3);

		std::vector<School>& accC = conferences[ACCCOASTAL];
		accC.emplace_back("Virginia", 6);
		accC.emplace_back("North Carolina", 6);
		accC.emplace_back("Virginia Tech", 5);
		accC.emplace_back("Miami (FL)", 4);
		accC.emplace_back("Pitt", 4);
		accC.emplace_back("Duke", 3);
		accC.emplace_back("Georgia Tech", 2);

		std::vector<School>& pacN = conferences[PAC12NORTH];
		pacN.emplace_back("Oregon", 9);
		pacN.emplace_back("Washington", 7);
		pacN.emplace_back("California", 6);
		pacN.emplace_back("Washington State", 6);
		pacN.emplace_back("Oregon State", 4);
		pacN.emplace_back("Stanford", 4);

		std::vector<School>& pacS = conferences[PAC12SOUTH];
		pacS.emplace_back("Utah", 8);
		pacS.emplace_back("USC", 6);
		pacS.emplace_back("Arizona State", 6);
		pacS.emplace_back("UCLA", 4);
		pacS.emplace_back("Colorado", 4);
		pacS.emplace_back("Arizona", 3);

		std::vector<School>& ind = conferences[INDEPENDENT];
		ind.emplace_back("Notre Dame", 8);
		ind.emplace_back("Liberty", 3);
		ind.emplace_back("BYU", 4);
		ind.emplace_back("Army", 4);
		ind.emplace_back("New Mexico State", 1);
		ind.emplace_back("Massachusetts", 0);

		std::vector<School>& macE = conferences[MACEAST];
		macE.emplace_back("Miami (OH)", 5);
		macE.emplace_back("Ohio", 4);
		macE.emplace_back("Akron", 0);
		macE.emplace_back("Kent State", 3);
		macE.emplace_back("Buffalo", 5);
		macE.emplace_back("Bowling Green State", 1);

		std::vector<School>& macW = conferences[MACWEST];
		macW.emplace_back("Central Michigan", 4);
		macW.emplace_back("Eastern Michigan", 2);
		macW.emplace_back("Western Michigan", 4);
		macW.emplace_back("Ball State", 3);
		macW.emplace_back("Northern Illinois", 2);
		macW.emplace_back("Toledo", 3);

		std::vector<School>& cusaE = conferences[CUSAEAST];
		cusaE.emplace_back("Florida Atlantic", 5);
		cusaE.emplace_back("Marshall", 4);
		cusaE.emplace_back("Western Kentucky", 4);
		cusaE.emplace_back("Charlotte", 3);
		cusaE.emplace_back("Florida International", 2);
		cusaE.emplace_back("Middle Tennessee State", 2);
		cusaE.emplace_back("Old Dominion", 0);

		std::vector<School>& cusaW = conferences[CUSAWEST];
		cusaW.emplace_back("Louisiana Tech", 5);
		cusaW.emplace_back("UAB", 4);
		cusaW.emplace_back("Southern Mississippi", 3);
		cusaW.emplace_back("North Texas", 2);
		cusaW.emplace_back("UTSA", 1);
		cusaW.emplace_back("Rice", 2);
		cusaW.emplace_back("UTEP", 0);

		std::vector<School>& aacE = conferences[AACEAST];
		aacE.emplace_back("Cincinnati", 6);
		aacE.emplace_back("UCF", 6);
		aacE.emplace_back("Temple", 5);
		aacE.emplace_back("South Florida", 3);
		aacE.emplace_back("East Carolina", 2);
		aacE.emplace_back("Connecticut", 1);

		std::vector<School>& aacW = conferences[AACWEST];
		aacW.emplace_back("Memphis", 6);
		aacW.emplace_back("Navy", 6);
		aacW.emplace_back("SMU", 5);
		aacW.emplace_back("Tulane", 4);
		aacW.emplace_back("Houston", 2);
		aacW.emplace_back("Tulsa", 2);

		std::vector<School>& sunE = conferences[SUNBELTEAST];
		sunE.emplace_back("Appalachian State", 6);
		sunE.emplace_back("Georgia Southern", 4);
		sunE.emplace_back("Georgia State", 3);
		sunE.emplace_back("Troy", 3);
		sunE.emplace_back("Coastal Carolina", 2);

		std::vector<School>& sunW = conferences[SUNBELTWEST];
		sunW.emplace_back("Louisiana", 5);
		sunW.emplace_back("Arkansas State", 4);
		sunW.emplace_back("Louisiana-Monroe", 3);
		sunW.emplace_back("Texas State", 1);
		sunW.emplace_back("South Alabama", 1);

		std::vector<School>& mwcM = conferences[MWCMOUNTAIN];
		mwcM.emplace_back("Boise State", 6);
		mwcM.emplace_back("Air Force", 6);
		mwcM.emplace_back("Utah State", 4);
		mwcM.emplace_back("Wyoming", 3);
		mwcM.emplace_back("Colorado State", 2);
		mwcM.emplace_back("New Mexico", 1);

		std::vector<School>& mwcW = conferences[MWCWEST];
		mwcW.emplace_back("San Diego State", 5);
		mwcW.emplace_back("Hawaii", 4);
		mwcW.emplace_back("Nevada", 3);
		mwcW.emplace_back("San Jose State", 2);
		mwcW.emplace_back("Fresno State", 2);
		mwcW.emplace_back("Nevada-Las Vegas", 1);

		assembleSchoolVector();

		createMatchups();

		playSchedule();

		rankTeams();
	}
};
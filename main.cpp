#include "league.h"
#include "loadData.h"

#include <iostream>

void printPlayer(Player& p) {
	std::cout << positionToStr(p.getPosition()) << " " << p.getName() << " (" << p.getRating() << " OVR, " << p.getPotential() << " POT) - "
			  << p.getYearString() << "\n";
}

int getInt() {
	int i;
	std::cin >> i;
	std::cin.ignore(256, '\n');
	return i;
}

struct Driver {
	League* league;

	~Driver() { delete league; }

	Driver() {
		std::cout << "Welcome to the CFB Simulator!\n";
		std::cout << "Generating league... ";
		std::cout.flush();
		league = new League();
		std::cout << "done." << std::endl;

		while (true) { mainMenu(); }
	}

	void mainMenu() {
		std::cout << "\nOptions: \n  1) View conference standings\n  2) View AP top 25\n  3) View information by school\n  4) Advance the "
					 "season\nEnter selection: ";
		int choice = getInt();
		if (choice == 1) {
			// TODO
			conferenceStandingsMenu();
		} else if (choice == 2) {
			if (league->getCurrentWeek() <= 3) std::cout << "League rankings will be available starting week 4\n";
			else
				league->printSchoolsByRanking();
		} else if (choice == 3) {
			seeSchoolDetails();
		} else if (choice == 4) {
			simMenu();
		} else {
			std::cout << "Invalid choice\n";
		}
	}

	void seeSchoolDetails() {
		while (true) {
			std::cout << "Enter a school name to see options, or leave blank to return: ";
			std::string schoolName;
			std::getline(std::cin, schoolName);
			if (schoolName == "") return;
			league->printSchoolDetails(schoolName);
			while (true) {
				std::cout << "\nOptions: \n  1) View " + schoolName + "'s roster\n  2) View " + schoolName + "'s schedule/results\n  3) View " +
								 schoolName + "'s season stats\n  4) Go back\n";
				std::cout << "Enter selection: ";
				int choice = getInt();
				if (choice == 1) {
					league->printSchoolRoster(schoolName);
					std::cout << "Enter a position group to see their rating breakdowns.\nValid position groups are QB, HB, WR, TE, OL, DL, LB, CB, "
								 "S, K, P: ";
					std::string pos;
					std::getline(std::cin, pos);
					league->printPositionGroup(schoolName, strToPosition(pos));
				} else if (choice == 2)
					league->printSchoolResults(schoolName);
				else if (choice == 3) {
					if (league->getCurrentWeek() == 1) {
						std::cout << "No games have been played yet this season.\n";
						continue;
					}
					std::cout << "Enter week number, or use 0 for an aggregation of season-wide stats: ";
					choice = getInt();
					while (choice > league->getCurrentWeek() - 1) {
						std::cout << "Week must be between 1 and " << league->getCurrentWeek() - 1 << ", try again: ";
						choice = getInt();
					}
					--choice;
					TeamStats* stats;
					if (choice < 0) stats = league->getSchoolAggregatedStats(schoolName);
					else
						stats = league->printSchoolGameStats(schoolName, choice);
					if (stats == nullptr) continue;
					std::cout << "Press enter repeatedly to iterate over individual player stats. Type "
								 "anything and then hit enter to exit.\n";
					int player = 0;
					while (true) {
						std::string skip = "";
						std::getline(std::cin, skip);
						if (skip != "") break;
						if (!league->printGamePlayerStats(stats, player)) break;
						++player;
					}
					if (choice < 0) delete stats;
				} else
					break;
			}
		}
	}

	void simMenu() {
		if (league->getCurrentWeek() == 17) {
			std::cout << "The entire season has been played. Advance to next season? (y/n): ";
			char c;
			std::cin >> c;
			if (c == 'y') {
				std::cout << "Advancing to next season...";
				std::cout.flush();
				league->prepareNextSeason();
				std::cout << " done.\n\n";
			}
			return;
		}

		std::cout << "\nOptions: \n";
		std::cout << "  1) Play one game\n  2) Advance one or more weeks\n  3) Advance through the end of the season\n  4) Back\nEnter selection: ";
		int choice = getInt();
		if (choice == 1) {
			league->printWeekMatchups();
			std::cout << "\nEnter a matchup number, or enter a school name to play its game: \n";
			std::string input;
			std::getline(std::cin, input);
			try {
				int matchupIndex = std::stoi(input);
				league->simOneGame(matchupIndex);
			} catch (std::invalid_argument e) { league->simOneGame(input); }
		} else if (choice == 2) {
			std::cout << "Number of weeks to advance: ";
			for (int weeks = getInt(); weeks > 0; weeks--) league->simOneWeek();
		} else if (choice == 3)
			league->simSeason();
	}

	void conferenceStandingsMenu() {
		while (true) {
			std::cout
				<< "\nConferences:\n  1) Big Ten\n  2) PAC-12\n  3) Big 12\n  4) ACC\n  5) SEC\n  6) MAC\n  7) C-USA\n  8) AAC\n  9) Sun Belt\n  10) "
				   "MWC\n  11) Independent\n  12) Go back\n";
			std::cout << "Enter a choice: ";
			int choice = getInt();
			std::pair<Conference, Conference> divs;
			switch (choice) {
			case 1: divs = std::make_pair(BIGTENEAST, BIGTENWEST); break;
			case 2: divs = std::make_pair(PAC12NORTH, PAC12SOUTH); break;
			case 3: divs = std::make_pair(BIG12, BIG12); break;
			case 4: divs = std::make_pair(ACCATLANTIC, ACCCOASTAL); break;
			case 5: divs = std::make_pair(SECEAST, SECWEST); break;
			case 6: divs = std::make_pair(MACEAST, MACWEST); break;
			case 7: divs = std::make_pair(CUSAEAST, CUSAWEST); break;
			case 8: divs = std::make_pair(AACEAST, AACWEST); break;
			case 9: divs = std::make_pair(SUNBELTEAST, SUNBELTWEST); break;
			case 10: divs = std::make_pair(MWCMOUNTAIN, MWCWEST); break;
			case 11: divs = std::make_pair(INDEPENDENT, INDEPENDENT); break;
			default: return;
			}
			league->printConferenceStandings(divs);
		}
	}
};

int main() {
	srand(time(NULL));
	GlobalData::loadEverything();

	Driver d;

	return 0;
}
/*
int mainTwo() {
		School s1("Michigan", 8);
	School s2("Ohio State", 10);

	GamePlayer game(&s1, &s2);
	std::pair<GamePlayer::TeamStats*, GamePlayer::TeamStats*> results =
		game.startRealTimeGameLoop();

	std::cout << "\n\nStats for Away Team (Michigan):\n";
	results.first->printBigStuff();
	std::cout << "\n\nStats for Home Team (Ohio State):\n";
	results.second->printBigStuff();

	int i;
	int m = 0;
	for (std::vector<Player*> playerSet :
		 { results.first->getPlayersRecorded(), results.second->getPlayersRecorded() }) {
		i = 0;
		for (auto awayPlayer : playerSet) {
			i++;
			std::cout << i << ": " << positionToStr(awayPlayer->getPosition()) << " "
					  << awayPlayer->getName() << "\n";
			if (m == 0) results.first->printPlayerStats(awayPlayer);
			else
				results.second->printPlayerStats(awayPlayer);
			std::cout << "\n\n";
		}
		std::cout << "----------------------------------------\n\n";
		m++;
	}

	delete results.first;
	delete results.second;

	return 0;
}*/
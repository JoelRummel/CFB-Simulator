#include "league.h"

#include <iostream>

void printPlayer(Player& p) {
	std::cout << positionToStr(p.getPosition()) << " " << p.getName() << " (" << p.getRating()
			  << " OVR, " << p.getPotential() << " POT) - " << p.getYearString() << "\n";
}

int getInt() {
	int i;
	std::cin >> i;
	std::cin.ignore(256, '\n');
	return i;
}

void seeSchoolDetails(League& league) {
	while (true) {
		std::cout << "Enter a school name to see options, or leave blank to return: ";
		std::string schoolName;
		std::getline(std::cin, schoolName);
		if (schoolName == "") return;

		while (true) {
			std::cout
				<< "\nOptions: \n  1) View roster\n  2) See schedule/results\n  3) See individual "
				   "game results\n  4) Go back\n";
			std::cout << "Enter selection: ";
			int choice = getInt();
			if (choice == 1) {
				league.printSchoolRoster(schoolName);
			} else if (choice == 2)
				league.printSchoolResults(schoolName);
			else if (choice == 3) {
				if (league.getCurrentWeek() == 1) {
					std::cout << "No games have been played yet this season.\n";
					continue;
				}
				std::cout << "Enter week number: ";
				choice = getInt();
				while (choice < 1 || choice > league.getCurrentWeek() - 1) {
					std::cout << "Week must be between 1 and " << league.getCurrentWeek() - 1
							  << ", try again: ";
					choice = getInt();
				}
				--choice;
				TeamStats* stats = league.printSchoolGameStats(schoolName, choice);
				if (stats == nullptr) continue;
				std::cout << "Press enter repeatedly to iterate over individual player stats. Type "
							 "anything and then hit enter to exit.\n";
				int player = 0;
				while (true) {
					std::string skip = "";
					std::getline(std::cin, skip);
					if (skip != "") break;
					if (!league.printGamePlayerStats(stats, player)) break;
					++player;
				}
			} else
				break;
		}
	}
}

int main() {
	srand(time(NULL));

	std::cout << "Welcome to the CFB Simulator!\n";
	std::cout << "Generating league... ";
	std::cout.flush();
	League league;
	std::cout << "done." << std::endl;
	while (league.getCurrentWeek() <= 13) {
		std::cout << "\nOptions: \n";
		std::cout << "  1) Play one week\n  2) Play remainder of season\n  3) See school "
					 "details\n  4) Play one game\n  5) See school rankings\nEnter selection: ";
		int choice = getInt();
		if (choice == 1) league.simOneWeek();
		else if (choice == 2)
			league.simSeason();
		else if (choice == 3)
			seeSchoolDetails(league);
		else if (choice == 4) {
			league.printWeekMatchups();
			std::cout << "\nEnter a matchup number, or enter a school name to play its game: \n";
			std::string input;
			std::getline(std::cin, input);
			try {
				int matchupIndex = std::stoi(input);
				league.simOneGame(matchupIndex);
			} catch (std::invalid_argument e) { league.simOneGame(input); }
		} else if (choice == 5) {
			if (league.getCurrentWeek() <= 3)
				std::cout << "League rankings will be available starting week 4\n";
			else
				league.printSchoolsByRanking();
		} else
			std::cout << "Invalid choice\n";
	}

	league.printSchoolsByRanking();
	seeSchoolDetails(league);
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
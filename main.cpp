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

int main() {
	srand(time(NULL));

	std::cout << "Welcome to the CFB Simulator!\n";
	League league;
	league.printSchoolsByRanking();
	while (true) {
		std::cout << "Enter a school name to see options: ";
		std::string schoolName;
		std::getline(std::cin, schoolName);
		if (schoolName == "Exit") return 0;

		while (true) {
			std::cout
				<< "Options: \n  1) View roster\n  2) See season results\n  3) See individual "
				   "game results\n  4) Go back\n";
			std::cout << "Enter selection: ";
			int choice = getInt();
			if (choice == 1) {
				league.printSchoolRoster(schoolName);
			} else if (choice == 2)
				league.printSchoolResults(schoolName);
			else if (choice == 3) {
				std::cout << "Enter week number: ";
				choice = getInt();
				while (choice < 1 || choice > 13) {
					std::cout << "Weeks must be between 1 and 13, try again: " << std::endl;
					choice = getInt();
				}
				--choice;
				league.printSchoolGameStats(schoolName, choice);
			} else
				break;
		}
	}
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
#pragma once

#include "player.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

struct SortByRating {
	bool operator()(const Player* p1, const Player* p2) { return (p1->getOVR() > p2->getOVR()); }
};

struct Needs {
	Position pos;
	int num;
};

class Roster {
private:
	std::vector<Player> roster; // This is where all players actually live!
	std::vector<std::vector<Player*>> depthChart;
	int startingPrestige;

	void generateOffRoster() {
		/*
		4 QB
		5 RB
		11 OL
		3 TE
		7 WR
		30 TOTAL
		*/
		std::vector<std::pair<Position, int>> orders{ std::make_pair(QB, 3), std::make_pair(HB, 5), std::make_pair(OL, 11), std::make_pair(TE, 3),
													   std::make_pair(WR, 7) };

		for (auto order : orders) {
			for (int i = 0; i < order.second; ++i) roster.push_back(playerFactory(order.first, (std::rand() % 4) + 1, startingPrestige));
		}
	}

	void generateDefRoster() {
		/*
		10 DL
		8 LB
		7 CB
		5 S
		30 TOTAL
		*/
		std::vector<std::pair<Position, int>> orders{ std::make_pair(DL, 10), std::make_pair(LB, 8), std::make_pair(CB, 7), std::make_pair(S, 5) };

		for (auto order : orders) {
			for (int i = 0; i < order.second; ++i) roster.push_back(playerFactory(order.first, (std::rand() % 4) + 1, startingPrestige));
		}
	}

	void generateSpecialTeams() {
		std::vector<std::pair<Position, int>> orders{ std::make_pair(P, 1), std::make_pair(K, 1) };

		for (auto order : orders) {
			for (int i = 0; i < order.second; ++i) roster.push_back(playerFactory(order.first, (std::rand() % 4) + 1, startingPrestige));
		}
	}

public:
	Roster() {}

	Player* addPlayer(Player* player) {
		roster.push_back(*player);
		delete player;
		return &roster.back();
	}

	void ageAndGraduatePlayers() {
		for (int i = (int)roster.size() - 1; i >= 0; i--) {
			bool graduated = roster[i].ageAndGraduate();
			if (graduated) roster.erase(roster.begin() + i);
		}
	}

	void trainPlayersAtPosition(Position pos, double trainingMultiplier) {
		for (Player* player : getAllPlayersAt(pos, false)) {
			int amount = std::ceil(4 * trainingMultiplier) + 2;
			player->train(amount);
		}
	}

	void generateRoster(int prestige) {
		startingPrestige = prestige;
		generateOffRoster();
		generateDefRoster();
		generateSpecialTeams();
	}

	std::vector<Player*> getAllPlayersAt(Position p, bool sorted = true) {
		std::vector<Player*> vec;
		for (int i = 0; i < (int)roster.size(); ++i) {
			if (roster[i].getPosition() == p) vec.push_back(&(roster[i]));
		}
		if (sorted) {
			SortByRating sbr;
			std::sort(vec.begin(), vec.end(), sbr);
		}
		return vec;
	}

	std::vector<Player*> getElevenMen(const std::vector<Needs>& orders) {
		std::vector<Player*> eleven;
		for (Needs order : orders) {
			for (int i = 0; i < order.num; ++i) eleven.push_back(depthChart[order.pos][i]);
		}
		assert(eleven.size() == 11);
		return eleven;
	}

	void organizeDepthChart() {
		depthChart.clear();
		for (Position p : { QB, HB, WR, TE, OL, DL, LB, CB, S, K, P }) { depthChart.push_back(getAllPlayersAt(p)); }
	}

	int getRosterSize() {
		return roster.size();
	}

	void printRoster() {
		std::cout << "    Name                 Pos Year       OVR \n";
		std::cout << "-------------------------------------------\n";
		//            46. Jalen Edwards        QB  Sophomore  97
		int num = 1;
		for (auto& player : roster) {
			std::string name = player.getName();
			std::string posStr = positionToStr(player.getPosition());
			std::string yearStr = player.getYearString();
			std::printf("%2d. %-21s%-3s%-11s%-3d\n", num, name.c_str(), posStr.c_str(), yearStr.c_str(), player.getOVR());
			++num;
		}
	}

	void printPositionGroup(Position pos) {
		// We need to decide relevant statistics
		std::vector<std::pair<Rating, int>> ratings = getRatingFactors(pos).first;
		printf("Name                 Year       OVR  ");
		for (int i = 0; i < ratings.size(); i++) { printf("%-5s", ratingToStr(ratings[i].first).c_str()); }
		printf("\n------------------------------------");
		for (int i = 0; i < ratings.size(); i++) printf("-----");
		printf("\n");
		for (auto& player : roster) {
			if (player.getPosition() != pos) continue;
			printf("%-21s%-11s%-5d", player.getName().c_str(), player.getYearString().c_str(), player.getOVR());
			for (auto& rating : ratings) { printf("%-5d", player.getRating(rating.first, true)); }
			printf("\n");
		}
	}
};
#pragma once

#include "player.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

struct SortByRating {
	bool operator()(const Player* p1, const Player* p2) { return (p1->getRating() > p2->getRating()); }
};

struct Needs {
	Position pos;
	int num;
};

class Roster {
  private:
	std::vector<Player> roster; // This is where all players actually live!
	int startingPrestige;

	void generateOffRoster() {
		/*
		4 QB
		5 RB
		14 OL
		3 TE
		8 WR
		34 TOTAL
		*/
		std::vector<std::pair<Position, int>> orders { std::make_pair(QB, 4), std::make_pair(HB, 5), std::make_pair(OL, 14), std::make_pair(TE, 3),
													   std::make_pair(WR, 8) };

		for (auto order : orders) {
			for (int i = 0; i < order.second; ++i) roster.push_back(playerFactory(order.first, (std::rand() % 4) + 1, startingPrestige));
		}
	}

	void generateDefRoster() {
		/*
		10 DL
		9 LB
		5 CB
		5 S
		29 TOTAL
		*/
		std::vector<std::pair<Position, int>> orders { std::make_pair(DL, 10), std::make_pair(LB, 9), std::make_pair(CB, 5), std::make_pair(S, 5) };

		for (auto order : orders) {
			for (int i = 0; i < order.second; ++i) roster.push_back(playerFactory(order.first, (std::rand() % 4) + 1, startingPrestige));
		}
	}

	void generateSpecialTeams() {
		std::vector<std::pair<Position, int>> orders { std::make_pair(P, 1), std::make_pair(K, 2) };

		for (auto order : orders) {
			for (int i = 0; i < order.second; ++i) roster.push_back(playerFactory(order.first, (std::rand() % 4) + 1, startingPrestige));
		}
	}

	double getGroupAvg(Position p, int depth) {
		std::vector<Player*> oline = getAllPlayersAt(OL);
		double total = 0;
		for (int i = 0; i < depth; ++i) { total += oline[i]->getRating(); }
		return total / depth;
	}

  public:
	Roster() {}

	void generateRoster(int prestige) {
		startingPrestige = prestige;
		generateOffRoster();
		generateDefRoster();
		generateSpecialTeams();
	}

	Player* getStarterAt(Position p) {
		int highestRating = 0;
		int highestIndex = 0;
		for (int i = 0; i < (int)roster.size(); ++i) {
			if (roster[i].getPosition() == p && highestRating < roster[i].getRating()) {
				highestRating = roster[i].getRating();
				highestIndex = i;
			}
		}
		return &roster[highestIndex];
	}

	std::vector<Player*> getAllPlayersAt(Position p) {
		std::vector<Player*> vec;
		for (int i = 0; i < (int)roster.size(); ++i) {
			if (roster[i].getPosition() == p) vec.push_back(&(roster[i]));
		}
		SortByRating sbr;
		std::sort(vec.begin(), vec.end(), sbr);
		return vec;
	}

	double getOLineRating() { return getGroupAvg(OL, 5); }

	double getDLineRating() { return getGroupAvg(DL, 4); }

	std::vector<Player*> getElevenMen(const std::vector<Needs>& orders) {
		std::vector<Player*> eleven;
		for (Needs order : orders) {
			std::vector<Player*> playersAt = getAllPlayersAt(order.pos);
			for (int i = 0; i < order.num; ++i) eleven.push_back(playersAt[i]);
		}
		assert(eleven.size() == 11);
		return eleven;
	}

	void printRoster() {
		std::cout << "    Name              Pos Year       OVR \n";
		std::cout << "----------------------------------------\n";
		//            46. Jalen Edwards     QB  Sophomore  97
		int num = 1;
		for (auto& player : roster) {
			std::string name = player.getName();
			std::string posStr = positionToStr(player.getPosition());
			std::string yearStr = player.getYearString();
			std::printf("%2d. %-18s%-3s%-11s%-3d\n", num, name.c_str(), posStr.c_str(), yearStr.c_str(), player.getOVR());
			++num;
		}
	}

	void printPositionGroup(Position pos) {
		// We need to decide relevant statistics
		std::vector<std::pair<Rating, double>> ratings = getRatingFactors(pos);
		printf("Name              Year       OVR  ");
		for (int i = 0; i < ratings.size(); i++) { printf("%-5s", ratingToStr(ratings[i].first).c_str()); }
		printf("\n---------------------------------");
		for (int i = 0; i < ratings.size(); i++) printf("-----");
		printf("\n");
		for (auto& player : roster) {
			if (player.getPosition() != pos) continue;
			printf("%-18s%-11s%-5d", player.getName().c_str(), player.getYearString().c_str(), player.getOVR());
			for (auto& rating : ratings) { printf("%-5d", player.getRating(rating.first)); }
			printf("\n");
		}
	}
};
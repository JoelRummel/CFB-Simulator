#pragma once

#include "player.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

struct SortByOVR {
	bool operator()(const Player* p1, const Player* p2) { return (p1->getOVR() > p2->getOVR()); }
};

struct SortByRating {
	Rating rating;

	SortByRating(Rating r) : rating(r) {}

	bool operator()(const Player* p1, const Player* p2) {
		return (p1->getRating(rating) > p2->getRating(rating));
	}
};

struct SortByOutOfPositionOvr {
	Position position;

	SortByOutOfPositionOvr(Position p) : position(p) {}

	bool operator()(const Player* p1, const Player* p2) {
		int ovr1 = estimateOutOfPositionOvr(p1->getRatingsVector(), position);
		int ovr2 = estimateOutOfPositionOvr(p2->getRatingsVector(), position);
		return (ovr1 > ovr2);
	}
};

struct Needs {
	Position pos;
	int num;
};

// 					  { QB,HB,WR,TE,OL,DL,LB,CB,S,K,P }
int startingCount[] = { 1, 1, 3, 1, 5, 4, 3, 2, 2,1,1 };

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
		4 TE
		9 WR
		33 TOTAL
		*/
		std::vector<std::pair<Position, int>> orders{ std::make_pair(QB, 4), std::make_pair(HB, 5), std::make_pair(OL, 11), std::make_pair(TE, 4),
													   std::make_pair(WR, 9) };

		for (auto order : orders) {
			for (int i = 0; i < order.second; ++i) roster.push_back(playerFactory(order.first, (std::rand() % 4) + 1, startingPrestige));
		}
	}

	void generateDefRoster() {
		/*
		10 DL
		9 LB
		8 CB
		6 S
		33 TOTAL
		*/
		std::vector<std::pair<Position, int>> orders{ std::make_pair(DL, 10), std::make_pair(LB, 9), std::make_pair(CB, 8), std::make_pair(S, 6) };

		for (auto order : orders) {
			for (int i = 0; i < order.second; ++i) roster.push_back(playerFactory(order.first, (std::rand() % 4) + 1, startingPrestige));
		}
	}

	void generateSpecialTeams() {
		std::vector<std::pair<Position, int>> orders{ std::make_pair(P, 2), std::make_pair(K, 2) };

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

	void advanceOneWeek() {
		for (auto& player : roster) {
			player.advanceOneWeek();
		}
	}

	void ageAndGraduatePlayers() {
		for (int i = (int)roster.size() - 1; i >= 0; i--) {
			bool graduated = roster[i].ageAndGraduate();
			if (graduated) roster.erase(roster.begin() + i);
		}
	}

	void trainPlayersAtPosition(Position pos, double trainingMultiplier) {
		for (Player* player : getAllPlayersAt(pos, false)) {
			player->train(trainingMultiplier);
		}
	}

	void generateRoster(int prestige) {
		startingPrestige = prestige;
		generateOffRoster();
		generateDefRoster();
		generateSpecialTeams();
	}

	std::vector<Player*> getAllPlayersAt(Position p, bool sorted = true, bool excludeInjured = false) {
		std::vector<Player*> vec;
		for (int i = 0; i < (int)roster.size(); ++i) {
			if (roster[i].isInjured() && excludeInjured) continue;
			if (roster[i].getPosition() == p) vec.push_back(&(roster[i]));
		}
		if (sorted) {
			SortByOVR sbr;
			std::sort(vec.begin(), vec.end(), sbr);
		}
		return vec;
	}

	std::vector<Player*> getElevenMen(const std::vector<Needs>& orders) {
		std::vector<Player*> eleven;
		std::unordered_set<Player*> elevenSet;
		for (Needs order : orders) {
			int playersFound = 0;
			for (int i = 0; playersFound < order.num; ++i) { // TODO: find a way to not blow up if everyone's hurt!
				Player* player = depthChart[order.pos][i];
				if (player->isInjured()) continue;
				eleven.push_back(depthChart[order.pos][i]);
				playersFound++;
			}
		}
		assert(eleven.size() == 11);
		return eleven;
	}

	void organizeDepthChart() {
		depthChart.clear();
		for (Position p : { QB, HB, WR, TE, OL, DL, LB, CB, S, K, P }) {
			std::vector<Player*> players = getAllPlayersAt(p);
			std::vector<Player*> remainingPlayers;
			for (auto& player : roster) {
				if (player.getPosition() != p) remainingPlayers.push_back(&player);
			}
			SortByOutOfPositionOvr sboopo(p);
			std::sort(remainingPlayers.begin(), remainingPlayers.end(), sboopo);
			for (auto player : remainingPlayers) players.push_back(player);
			depthChart.push_back(players);
		}
	}

	std::pair<int, int> calcTotalOvrs() {
		std::pair<int, int> totals = { 0, 0 };
		for (Position p : { QB, HB, WR, TE, OL, DL, LB, CB, S }) {
			std::vector<Player*> players = getAllPlayersAt(p);
			int starters = startingCount[p];
			int positionTotal = 0;
			for (int i = 0; i < starters; i++) {
				positionTotal += players[i]->getOVR() * 2;
			}
			for (int i = starters; i < starters * 2; i++) {
				positionTotal += players[i]->getOVR();
			}
			if (p == DL || p == LB || p == CB || p == S) totals.second += positionTotal;
			else totals.first += positionTotal;
		}
		return totals;
	}

	int getRosterSize() {
		return roster.size();
	}

	void printRoster() {
		std::cout << "    Name                 Pos Year       OVR     Status  \n";
		std::cout << "--------------------------------------------------------\n";
		//            46. Jalen Edwards        QB  Sophomore  97 (+2) OUT 2wks
		int num = 1;
		for (auto& player : roster) {
			std::printf("%2d. ", num);
			player.printInfoLine();
			++num;
		}
	}

	void printPositionGroup(Position pos) {
		// We need to decide relevant statistics
		std::vector<std::pair<Rating, int>> ratings = getRatingFactors(pos).first;
		printf("Name                 Pos Year       OVR  ");
		for (int i = 0; i < ratings.size(); i++) { printf("%-5s", ratingToStr(ratings[i].first).c_str()); }
		printf("\n----------------------------------------");
		for (int i = 0; i < ratings.size(); i++) printf("-----");
		printf("\n");
		int count = 0;
		for (auto& player : depthChart[pos]) {
			int ovr = player->getPosition() == pos ? player->getOVR() : estimateOutOfPositionOvr(player->getRatingsVector(), pos);
			printf("%-21s%-4s%-11s%-5d", player->getName().c_str(), positionToStr(player->getPosition()).c_str(), player->getYearString().c_str(), ovr);
			for (auto& rating : ratings) { printf("%-5d", player->getRating(rating.first, true)); }
			printf("\n");
			count += 1;
			if (count >= 20) break;
			if (count == startingCount[pos]) printf("--------------------------------------------------------------------------------------\n");
		}
	}
};
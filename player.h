#pragma once

#include "loadData.h"
#include "util.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <string>
#include <unordered_map>

enum Position { QB, HB, WR, TE, OL, DL, LB, CB, S, K, P };

std::string positionToStr(Position p) {
	switch (p) {
	case QB: return "QB";
	case HB: return "HB";
	case WR: return "WR";
	case TE: return "TE";
	case OL: return "OL";
	case DL: return "DL";
	case LB: return "LB";
	case CB: return "CB";
	case S: return "S";
	case K: return "K";
	case P: return "P";
	}
	return "";
}

Position strToPosition(std::string s) {
	if (s == "QB") return QB;
	if (s == "HB") return HB;
	if (s == "WR") return WR;
	if (s == "TE") return TE;
	if (s == "OL") return OL;
	if (s == "DL") return DL;
	if (s == "LB") return LB;
	if (s == "CB") return CB;
	if (s == "S") return S;
	if (s == "K") return K;
	if (s == "P") return P;
	assert(false);
}

/*

				S                  S

				   LB    LB    LB
 CB                             CB
				 DL   DL   DL   DL
-----------------------------------
WR       OL  OL  OL  OL  OL       WR
		  TE         QB            WR

								 HB
*/

enum Rating {
	SPEED,
	STRENGTH,
	BREAKTACKLE,
	BALLSECURITY,
	CATCH,
	RUNBLOCK,
	PASSBLOCK,
	RUNVISION,
	PASSVISION,
	PASSACCURACY,
	PASSPOWER,
	GETTINGOPEN,

	TACKLE,
	PASSCOVER,
	RUNSTOP,
	PASSRUSH,
	STRIPBALL,

	KICKPOWER,
	KICKACCURACY,
	PUNTPOWER,
	PUNTACCURACY
};

std::string ratingToStr(Rating r) {
	switch (r) {
	case SPEED: return "SPD";
	case STRENGTH: return "STR";
	case BREAKTACKLE: return "BRTK";
	case BALLSECURITY: return "BLSC";
	case CATCH: return "CAT";
	case RUNBLOCK: return "RUBL";
	case PASSBLOCK: return "PABL";
	case RUNVISION: return "RVIS";
	case PASSVISION: return "PVIS";
	case PASSACCURACY: return "PACC";
	case PASSPOWER: return "PPOW";
	case GETTINGOPEN: return "OPEN";

	case TACKLE: return "TAK";
	case PASSCOVER: return "COV";
	case RUNSTOP: return "RSTP";
	case PASSRUSH: return "PRUS";
	case STRIPBALL: return "STRP";

	case KICKPOWER: return "KPOW";
	case KICKACCURACY: return "KACC";
	case PUNTPOWER: return "PUPO";
	case PUNTACCURACY: return "PUAC";
	}
}

enum Action {
	RUNBLOCKING,
	PASSBLOCKING,
	BLITZING,
	RUSHING,
	RUNNINGSHORT,
	RUNNINGMIDDLE,
	RUNNINGDEEP,
	PASSING,
	KICKING,
	COVERING,
	ZONECOVERING,
	HANDINGOFF
};

enum Zone { LINE, LINELEFT, LINERIGHT, SHORTLEFT, SHORTRIGHT, BACKFIELD, MIDDLE, DEEP, OUTOFPLAY };

std::vector<std::pair<Rating, double>> getRatingFactors(Position p) {
	using f = std::pair<Rating, double>;
	switch (p) {
	case QB: return { f(SPEED, 4), f(PASSVISION, 6), f(PASSPOWER, 6), f(PASSACCURACY, 6), f(BREAKTACKLE, 2), f(BALLSECURITY, 4), f(RUNVISION, 4) };
	case HB:
		return { f(SPEED, 5),     f(STRENGTH, 4), f(BREAKTACKLE, 6), f(BALLSECURITY, 6), f(RUNBLOCK, 3),
				 f(PASSBLOCK, 3), f(CATCH, 3),    f(RUNVISION, 6),   f(GETTINGOPEN, 3) };
	case WR:
		return {
			f(SPEED, 6), f(STRENGTH, 2), f(BREAKTACKLE, 3), f(RUNBLOCK, 2), f(CATCH, 6), f(BALLSECURITY, 4), f(RUNVISION, 4), f(GETTINGOPEN, 6)
		};
	case TE:
		return { f(SPEED, 3),     f(STRENGTH, 5), f(BREAKTACKLE, 2), f(BALLSECURITY, 4), f(CATCH, 5),
				 f(RUNVISION, 2), f(RUNBLOCK, 5), f(PASSBLOCK, 4),   f(GETTINGOPEN, 4) };
	case OL: return { f(STRENGTH, 5), f(RUNBLOCK, 6), f(PASSBLOCK, 6) };

	case DL: return { f(STRENGTH, 6), f(SPEED, 3), f(RUNSTOP, 6), f(PASSRUSH, 6), f(TACKLE, 5), f(STRIPBALL, 4), f(PASSCOVER, 2) };
	case LB: return { f(STRENGTH, 4), f(SPEED, 4), f(RUNSTOP, 3), f(PASSRUSH, 4), f(PASSCOVER, 4), f(TACKLE, 5), f(STRIPBALL, 3), f(CATCH, 3) };
	case CB: return { f(STRENGTH, 2), f(SPEED, 6), f(PASSCOVER, 6), f(TACKLE, 4), f(STRIPBALL, 2), f(PASSRUSH, 3), f(RUNSTOP, 2), f(CATCH, 5) };
	case S: return { f(STRENGTH, 3), f(SPEED, 5), f(PASSCOVER, 5), f(TACKLE, 6), f(PASSRUSH, 2), f(RUNSTOP, 3), f(CATCH, 3) };

	case K: return { f(KICKPOWER, 6), f(KICKACCURACY, 6), f(PUNTPOWER, 3), f(PUNTACCURACY, 2) };
	case P: return { f(PUNTPOWER, 6), f(PUNTACCURACY, 6), f(KICKPOWER, 3), f(KICKACCURACY, 2) };
	default: return {};
	}
}

class Player {
  private:
	std::string name;
	Position position;
	int year;
	int positionRating;
	double trainingMultiplier;
	std::vector<int> ratings;

  public:
	struct GameState {
		int fatigue = 100;
		int energy = 100;
		Action action;
		Zone zone;
		Player* target;
		int tick = 0;
		int clash = -1;
		double coverRating = 0;

		bool isReceiving() { return (action == RUNNINGSHORT || action == RUNNINGMIDDLE || action == RUNNINGDEEP); }
		bool isInTargetZone() {
			if (action == RUNNINGSHORT && (zone == SHORTLEFT || zone == SHORTRIGHT)) return true;
			if (action == RUNNINGMIDDLE && zone == MIDDLE) return true;
			if (action == RUNNINGDEEP && zone == DEEP) return true;
			return false;
		}
	};

	GameState gameState;

	Player(std::string n, Position p, int y, int rat, double pot, const std::vector<int>& rats) :
			name { n }, position { p }, year { y }, positionRating { rat }, trainingMultiplier { pot }, ratings { rats } {};

	std::string getName() const { return name; }
	Position getPosition() const { return position; }
	int getYear() const { return year; }
	int getRating() const { return positionRating; }
	std::string getYearString() const { return year == 1 ? "Freshman" : year == 2 ? "Sophomore" : year == 3 ? "Junior" : "Senior"; }
	int getPotential() const { return getRating() + std::round(3 * trainingMultiplier * (4 - year)); }
	int getOVR() const {
		std::vector<std::pair<Rating, double>> factors = getRatingFactors(position);
		double sum = 0;
		for (auto& factor : factors) { sum += (120 * getRating(factor.first) - 3600) / (factor.second * 14); }
		return std::round(sum / (double)factors.size());
	}
	int getRating(Rating r) const { return ratings[r]; }
	void advanceTick() {
		assert(!(position == WR && gameState.zone == BACKFIELD));
		gameState.tick++;
		if (std::rand() % 250 < getRating(SPEED)) gameState.tick++;

		if ((gameState.action == RUNNINGSHORT && gameState.zone == LINE && gameState.tick >= 4)) {
			gameState.tick = 0;
			gameState.zone = (std::rand() % 100 < 50 ? SHORTLEFT : SHORTRIGHT);
		}
		if ((gameState.action == RUNNINGMIDDLE || gameState.action == RUNNINGDEEP) &&
			(gameState.zone == SHORTLEFT || gameState.zone == SHORTRIGHT || gameState.zone == LINE || gameState.zone == BACKFIELD) &&
			gameState.tick >= 5) {
			// break up this line because clang is a biatch
			gameState.tick = 0;
			gameState.zone = MIDDLE;
		} else if (gameState.action == RUNNINGDEEP && gameState.zone == MIDDLE && gameState.tick >= 5) {
			gameState.tick = 0;
			gameState.zone = DEEP;
		}
	}
};

Player playerFactory(Position p, int y, int prestige) {
	std::string name = GlobalData::getRandomName();
	int rating = 30 + std::round(((RNG::randomNumberNormalDist(15, 5)) + ((y - 1) * 5)) * (prestige * 0.2222));
	if (rating > 100) rating = 100;
	// if (p == LB) rating = 40;
	double potential = ((std::rand() % 100) + 50) / 100;

	std::vector<int> rats(21, 30);
	std::vector<std::pair<Rating, double>> factors = getRatingFactors(p);
	for (const auto& pair : factors) {
		rats[pair.first] += ((rating * pair.second * 14) / 120) + RNG::randomNumberNormalDist(0, 6);
		if (rats[pair.first] > 100) rats[pair.first] = 100;
	}

	return Player(name, p, y, rating, potential, rats);
}

struct PlayerStats {
	int rushes;
	int rushingYards;
	int passingYards;
	int receivingYards;
	int completions;
	int incompletions;
	int rushingTDs;
	int passingTDs;
	int receivingTDs;
	int catches;
	int drops;
	int fumblesLost;
	int INTsThrown;

	int tackles;
	int sacks;
	int TFLs;
	int INTsCaught;
	int passDefenses;

	int FGsMade;
	int FGsMissed;
	int punts;
	int puntYards;
	int longestFG;
	int longestPunt;

	PlayerStats& operator+=(const PlayerStats& rhs) {
		this->rushes += rhs.rushes;
		this->rushingYards += rhs.rushingYards;
		this->passingYards += rhs.passingYards;
		this->receivingYards += rhs.receivingYards;
		this->completions += rhs.completions;
		this->incompletions += rhs.incompletions;
		this->rushingTDs += rhs.rushingTDs;
		this->passingTDs += rhs.passingTDs;
		this->receivingTDs += rhs.receivingTDs;
		this->catches += rhs.catches;
		this->drops += rhs.drops;
		this->fumblesLost += rhs.fumblesLost;
		this->INTsThrown += rhs.INTsThrown;
		this->tackles += rhs.tackles;
		this->sacks += rhs.sacks;
		this->TFLs += rhs.TFLs;
		this->INTsCaught += rhs.INTsCaught;
		this->passDefenses += rhs.passDefenses;
		this->FGsMade += rhs.FGsMade;
		this->FGsMissed += rhs.FGsMissed;
		this->punts += rhs.punts;
		this->puntYards += rhs.puntYards;
		this->longestFG = std::max(this->longestFG, rhs.longestFG);
		this->longestPunt = std::max(this->longestPunt, rhs.longestPunt);
		return *this;
	}
};

struct TeamStats {
	int games = 1;

	int points = 0;
	int numPossessions = 0;
	int timeOfPossession = 0;
	int sacksAllowed = 0;

	int yardsAllowed = 0;

	std::unordered_map<Player*, PlayerStats> players;

	void recordRush(Player* runner, int yards) {
		players[runner].rushes++;
		players[runner].rushingYards += yards;
		timeOfPossession += 30;
	}
	void recordPass(Player* qb, Player* receiver, int yards, bool complete) {
		if (complete) {
			players[qb].passingYards += yards;
			players[receiver].receivingYards += yards;
			players[qb].completions++;
			players[receiver].catches++;
		} else
			players[qb].incompletions++;
		timeOfPossession += 30;
	}
	void recordDrop(Player* dropper) { players[dropper].drops++; }
	void recordRushingTD(Player* scorer) { players[scorer].rushingTDs++; }
	void recordPassingTD(Player* qb, Player* receiver) {
		players[qb].passingTDs++;
		players[receiver].receivingTDs++;
	}

	void recordYardsAllowed(int yds) { yardsAllowed += yds; }
	void recordTackle(Player* p) { players[p].tackles++; }
	void recordSack(Player* p) { players[p].sacks++; }
	void recordTFL(Player* p) { players[p].TFLs++; }
	void recordPassDefense(Player* p) { players[p].passDefenses++; }
	void recordSackAllowed() { sacksAllowed++; }
	void recordINTThrown(Player* p) { players[p].INTsThrown++; }
	void recordINTCaught(Player* p) { players[p].INTsCaught++; }
	void recordFumble(Player* p) { players[p].fumblesLost++; }
	void recordFGAttempt(Player* p, bool made, int distance) {
		if (made) {
			players[p].FGsMade++;
			if (players[p].longestFG < distance) players[p].longestFG = distance;
		} else
			players[p].FGsMissed++;
	}
	void recordPunt(Player* p, int distance) {
		players[p].punts++;
		players[p].puntYards += distance;
		if (players[p].longestPunt < distance) players[p].longestPunt = distance;
	}

	int rushes() {
		int rush = 0;
		for (auto player : players) rush += player.second.rushes;
		return rush;
	}
	int rushingYards() {
		int yards = 0;
		for (auto player : players) yards += player.second.rushingYards;
		return yards;
	}
	int passingYards() {
		int yards = 0;
		for (auto player : players) yards += player.second.passingYards;
		return yards;
	}
	int offensiveYards() { return rushingYards() + passingYards(); }
	int completions() {
		int made = 0;
		for (auto player : players) made += player.second.completions;
		return made;
	}
	int incompletions() {
		int miss = 0;
		for (auto player : players) miss += player.second.incompletions;
		return miss;
	}
	int passAttempts() {
		int att = 0;
		for (auto player : players) att += player.second.completions + player.second.incompletions;
		return att;
	}
	double completionRate() {
		int completed = completions();
		return ((double)completed / (completed + incompletions())) * 100;
	}
	int drops() {
		int d = 0;
		for (auto player : players) d += player.second.drops;
		return d;
	}
	double rushingAvg() { return ((double)rushingYards() / rushes()); }
	int rushingTDs() {
		int tds = 0;
		for (auto player : players) tds += player.second.rushingTDs;
		return tds;
	}
	int passingTDs() {
		int tds = 0;
		for (auto player : players) tds += player.second.passingTDs;
		return tds;
	}

	int sacks() {
		int s = 0;
		for (auto player : players) s += player.second.sacks;
		return s;
	}
	int TFLs() {
		int tfls = 0;
		for (auto player : players) tfls += player.second.TFLs;
		return tfls;
	}
	int passDefenses() {
		int pds = 0;
		for (auto player : players) pds += player.second.passDefenses;
		return pds;
	}
	int INTsThrown() {
		int i = 0;
		for (auto player : players) i += player.second.INTsThrown;
		return i;
	}
	int fumblesLost() {
		int f = 0;
		for (auto player : players) f += player.second.fumblesLost;
		return f;
	}
	int INTsCaught() {
		int i = 0;
		for (auto player : players) i += player.second.INTsCaught;
		return i;
	}

	int FGsMade() {
		int i = 0;
		for (auto player : players) i += player.second.FGsMade;
		return i;
	}
	int FGsMissed() {
		int i = 0;
		for (auto player : players) i += player.second.FGsMissed;
		return i;
	}
	double FGSuccessRate() {
		int completed = FGsMade();
		return ((double)completed / (completed + FGsMissed())) * 100;
	}
	int punts() {
		int i = 0;
		for (auto player : players) i += player.second.punts;
		return i;
	}
	int puntYards() {
		int i = 0;
		for (auto player : players) i += player.second.puntYards;
		return i;
	}
	double avgPuntYards() {
		if (punts() == 0) return 0;
		return puntYards() / (double)punts();
	}
	int longestFG() {
		int l = 0;
		for (auto player : players) l = std::max(l, player.second.longestFG);
		return l;
	}
	int longestPunt() {
		int l = 0;
		for (auto player : players) l = std::max(l, player.second.longestPunt);
		return l;
	}

	void printBigStuff() {
		//     |----------------------------------------|
		bool agg = (games > 1);
		double dGames = (double)games;
		printf("OFFENSE\n----------------------------------------\n");
		printf("Total yards: %27d\n", offensiveYards());
		if (agg) printf("   Average: %28.1f\n", offensiveYards() / dGames);
		printf("Rushing yards: %25d\n", rushingYards());
		printf("Rushing average: %23.1f\n", rushingAvg());
		printf("Rushing TDs: %27d\n", rushingTDs());
		if (agg) printf("   Average: %28.1f\n", rushingTDs() / dGames);
		printf("Passing yards: %25d\n", passingYards());
		if (agg) printf("   Average: %28.1f\n", passingYards() / dGames);
		printf("Pass comp / att: %14d/%2d %3.1f%%\n", completions(), passAttempts(), completionRate());
		printf("Drops: %33d\n", drops());
		if (agg) printf("   Average: %28.1f\n", drops() / dGames);
		printf("Passing TDs: %27d\n", passingTDs());
		if (agg) printf("   Average: %28.1f\n", passingTDs() / dGames);
		printf("Fumbles lost: %26d\n", fumblesLost());
		if (agg) printf("   Average: %28.1f\n", fumblesLost() / dGames);
		printf("INTs thrown: %27d\n", INTsThrown());
		if (agg) printf("   Average: %28.1f\n", INTsThrown() / dGames);
		printf("Sacks allowed: %25d\n", sacksAllowed);
		if (agg) printf("   Average: %28.1f\n", sacksAllowed / dGames);
		printf("Field goals: %18d/%2d %3.1f%%\n", FGsMade(), FGsMade() + FGsMissed(), FGSuccessRate());
		printf("Longest FG: %28d\n", longestFG());
		printf("\nDEFENSE\n----------------------------------------\n");
		printf("Total yards allowed: %19d\n", yardsAllowed);
		if (agg) printf("   Average: %28.1f\n", yardsAllowed / dGames);
		printf("Tackles for loss: %22d\n", TFLs());
		if (agg) printf("   Average: %28.1f\n", TFLs() / dGames);
		printf("Sacks: %33d\n", sacks());
		if (agg) printf("   Average: %28.1f\n", sacks() / dGames);
		printf("Pass defenses: %25d\n", passDefenses());
		if (agg) printf("   Average: %28.1f\n", passDefenses() / dGames);
		printf("INTs caught: %27d\n", INTsCaught());
		if (agg) printf("   Average: %28.1f\n", INTsCaught() / dGames);
		printf("Average punting yards: %17.1f\n", avgPuntYards());
		printf("Longest punt: %26d\n", longestPunt());
	}

	void printPlayerStats(Player* p) {
		const PlayerStats& s = players[p];
		bool agg = (games > 1);
		double dGames = (double)games;
		std::cout << positionToStr(p->getPosition()) << " " << p->getName() << " (" << p->getOVR() << " OVR)\n";
		std::cout << "----------------------------------------\n";
		if (s.rushes > 0) {
			printf("Rushes: %32d\n", s.rushes);
			if (agg) printf("   Average: %28.1f\n", s.rushes / dGames);
			printf("Rushing yards: %25d\n", s.rushingYards);
			double rushAvg = (double)s.rushingYards / s.rushes;
			printf("Rushing average: %23.1f\n", rushAvg);
			printf("Rushing TDs: %27d\n", s.rushingTDs);
			if (agg) printf("   Average: %28.1f\n", s.rushingTDs / dGames);
		}
		if (s.completions > 0 || s.incompletions > 0) {
			printf("Passing yards: %25d\n", s.passingYards);
			if (agg) printf("   Average: %28.1f\n", s.passingYards / dGames);
			double compRate = ((double)s.completions / (s.completions + s.incompletions)) * 100;
			printf("Pass comp / att: %14d/%2d %3.1f%%\n", s.completions, s.completions + s.incompletions, compRate);
			printf("Passing TDs: %27d\n", s.passingTDs);
			printf("Interceptions: %25d\n", s.INTsThrown);
			printf("TD-INT Ratio: %26.1f\n", (double)s.passingTDs / (double)s.INTsThrown);
		}
		if (s.catches > 0 || s.drops > 0) {
			printf("Catches: %31d\n", s.catches);
			if (agg) printf("   Average: %28.1f\n", s.catches / dGames);
			printf("Drops: %33d\n", s.drops);
			if (agg) printf("   Average: %28.1f\n", s.drops / dGames);
			printf("Receiving yards: %23d\n", s.receivingYards);
			if (agg) printf("   Average: %28.1f\n", s.receivingYards / dGames);
			printf("Receiving TDs: %25d\n", s.receivingTDs);
			if (agg) printf("   Average: %28.1f\n", s.receivingTDs / dGames);
		}
		if (s.fumblesLost > 0) { printf("Fumbles lost: %26d\n", s.fumblesLost); }
		if (s.tackles > 0) {
			printf("Tackles: %31d\n", s.tackles);
			if (agg) printf("   Average: %28.1f\n", s.tackles / dGames);
		}
		if (s.TFLs > 0) {
			printf("Tackles for loss: %22d\n", s.TFLs);
			if (agg) printf("   Average: %28.1f\n", s.TFLs / dGames);
		}
		if (s.sacks > 0) {
			printf("Sacks: %33d\n", s.sacks);
			if (agg) printf("   Average: %28.1f\n", s.sacks / dGames);
		}
		if (s.passDefenses > 0) {
			printf("Pass defenses: %25d\n", s.passDefenses);
			if (agg) printf("   Average: %28.1f\n", s.passDefenses / dGames);
		}
		if (s.INTsCaught > 0) { printf("Interceptions: %25d\n", s.INTsCaught); }
		if (s.FGsMade > 0 || s.FGsMissed > 0) {
			double compRate = ((double)s.FGsMade / (s.FGsMade + s.FGsMissed)) * 100;
			printf("Field goals: %18d/%2d %3.1f%%\n", s.FGsMade, s.FGsMade + s.FGsMissed, compRate);
			if (s.FGsMade > 0) printf("Longest FG: %28d\n", s.longestFG);
		}
		if (s.punts > 0) {
			printf("Average punting yards: %17.1f\n", (double)s.puntYards / s.punts);
			printf("Longest punt: %26d\n", s.longestPunt);
		}
	}

	std::vector<Player*> getPlayersRecorded() const {
		std::vector<Player*> recorded;
		for (auto p : players) recorded.push_back(p.first);
		return recorded;
	}

	TeamStats& operator+=(TeamStats& rhs) {
		for (Player* player : rhs.getPlayersRecorded()) { this->players[player] += rhs.players[player]; }
		this->sacksAllowed += rhs.sacksAllowed;
		this->numPossessions += rhs.numPossessions;
		this->yardsAllowed += rhs.yardsAllowed;
		this->games += rhs.games;

		return *this;
	}
};

struct GameResult {
	TeamStats* awayStats = nullptr;
	TeamStats* homeStats = nullptr;
	bool awayWon;
	bool homeWon;
};
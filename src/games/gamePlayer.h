#pragma once

#include "../news.h"
#include "../school.h"
#include "numberMaker.h"
#include "gameManager.h"
#include "gamePlayExecutor.h"

#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <functional>

/**
 * Pure function that returns all players at a certain position from a given
 * subset of players.
 */
std::vector<Player*> getPlayersAtPosition(const std::vector<Player*>& playerSet, Position pos) {
	std::vector<Player*> players;
	for (auto player : playerSet) {
		if (player->getPosition() == pos) players.push_back(player);
	}
	return players;
}

class GamePlayer {
private:
	GameManager gameState;
	School* away;
	School* home;
	TeamStats* offStats;
	TeamStats* defStats;
	Roster* offense;
	Roster* defense;
	bool homePossession = false;

	bool printPlayByPlay = false;

	// Assume one back unless otherwise stated or implied
	enum OffensiveFormation { GOALLINE, TE2, HB2, WR3, WR4, WR4_EMPTY, WR5, FGFORM, PUNTFORM };

	// This MUST MATCH UP with the order of the OffensiveFormation enum!
	const std::vector<std::vector<Needs>> OffensivePersonnel{
		{ // Goal line
		  Needs { OL, 5 }, Needs { QB, 1 }, Needs { HB, 2 }, Needs { TE, 2 }, Needs { WR, 1 } },
		{ // TE2 WR2
		  Needs { OL, 5 }, Needs { QB, 1 }, Needs { HB, 1 }, Needs { TE, 2 }, Needs { WR, 2 } },
		{ // Split backs
		  Needs { OL, 5 }, Needs { QB, 1 }, Needs { HB, 2 }, Needs { TE, 1 }, Needs { WR, 2 } },
		{ // 3 WR 1 TE
		  Needs { OL, 5 }, Needs { QB, 1 }, Needs { HB, 1 }, Needs { TE, 1 }, Needs { WR, 3 } },
		{ // 4 WR
		  Needs { OL, 5 }, Needs { QB, 1 }, Needs { HB, 1 }, Needs { WR, 4 } },
		{ // Empty set 4WR
		  Needs { OL, 5 }, Needs { QB, 1 }, Needs { WR, 4 }, Needs { TE, 1 } },
		{ // Empty set 5WR
		  Needs { OL, 5 }, Needs { QB, 1 }, Needs { WR, 5 } },
		{ // Field goal
		  Needs { OL, 5 }, Needs { K, 1 }, Needs { QB, 1 }, Needs { TE, 2 }, Needs { HB, 2 } },
		{ // Punt
		  Needs { OL, 5 }, Needs { P, 1 }, Needs { HB, 1 }, Needs { TE, 2 }, Needs { CB, 2 } }
	};

	void printPlay(std::string msg) {
		if (printPlayByPlay) gameState.printPlay(msg);
	}

	/**
	 * Decides on what type of offensive play to run based on the current game state.
	 */
	PlayType decidePlay() {
		int down = gameState.getDown();
		int yardLine = gameState.getYardLine();
		int yardsToGo = gameState.getYardsToGo();
		if (down == 3 && yardsToGo > 5) return (std::rand() % 100 < 90 ? PASS : RUN);
		if (down == 4) {
			int fgDistance = yardLine + 17;
			if (fgDistance > 50) {
				if (yardLine > 40 || yardsToGo > 5) return PUNT;
				return (std::rand() & 100 < 70 ? PASS : RUN);
			} else
				return KICK;
		}
		if (yardsToGo < 4) return (std::rand() % 100 < 30 ? PASS : RUN);
		return (std::rand() % 100 < 60) ? PASS : RUN;
	}

	/**
	 * Takes a play type and randomly decides on an offensive formation.
	 * Reads game state to make the decision but doesn't overwrite any game state.
	 */
	OffensiveFormation decideFormation(PlayType play) {
		if (play == KICK) return FGFORM;
		if (play == PUNT) return PUNTFORM;
		if (gameState.getYardLine() < 3) return GOALLINE;
		if (play == PASS && gameState.getDown() > 2 && gameState.getYardsToGo() > 5) {
			// need to be more aggressive
			const OffensiveFormation forms[4] = { WR3, WR4, WR4_EMPTY, WR5 };
			return forms[RNG::randomWeightedIndex(std::vector<double> { 5, 8, 8, 8 })];
		}
		if (play == RUN) {
			const OffensiveFormation forms[6] = { HB2, TE2, WR3, WR4, WR4_EMPTY, WR5 };
			return forms[RNG::randomWeightedIndex(std::vector<double> { 10, 10, 10, 8, 3, 3 })];
		}
		if (play == PASS) {
			const OffensiveFormation forms[6] = { HB2, TE2, WR3, WR4, WR4_EMPTY, WR5 };
			return forms[RNG::randomWeightedIndex(std::vector<double> { 6, 4, 10, 10, 6, 4 })];
		}
		return WR3;
	}

	std::vector<Needs> decideDefensivePersonnel(OffensiveFormation form) {
		Needs linebackerNeed{ LB, 0 };
		Needs cornerbackNeed{ CB, 0 };
		Needs safetyNeed{ S, 0 };
		int spotsRemaining = 7; // 4 d-linemen at all times, 11 - 4 = 7
		for (auto offNeed : OffensivePersonnel[form]) {
			if (offNeed.pos == WR) {
				cornerbackNeed.num += offNeed.num;
				spotsRemaining -= offNeed.num;
			}
			if (offNeed.pos == TE || offNeed.pos == HB) {
				linebackerNeed.num += offNeed.num;
				spotsRemaining -= offNeed.num;
			}
		}
		while (spotsRemaining > 0 && safetyNeed.num < 2) {
			safetyNeed.num++;
			spotsRemaining--;
		}
		linebackerNeed.num += spotsRemaining;

		return { { DL, 4 }, linebackerNeed, cornerbackNeed, safetyNeed };
	}

	/**
	 * Mutates the game state of all 22 players on the field to assign them an action
	 * based on the play type.
	 */
	Field applyFormation(OffensiveFormation form, PlayType play) {
		std::vector<Player*> offOnField = offense->getElevenMen(OffensivePersonnel[form]);
		std::vector<Player*> defOnField = defense->getElevenMen(decideDefensivePersonnel(form));

		for (int i = 0; i < 11; ++i) {
			Position offPos = offOnField[i]->getPosition();
			offOnField[i]->gameState.action = BLOCKING; // as a default

			if (offPos == OL) {
				offOnField[i]->gameState.action = BLOCKING;
			}
			if (offPos == QB) {
				offOnField[i]->gameState.action = (play == PASS) ? PASSING : HANDINGOFF;
			}
			if (offPos == HB) {
				const Action a[2] = { BLOCKING, RECEIVING };
				offOnField[i]->gameState.action =
					(play == RUN) ? BLOCKING : a[RNG::randomWeightedIndex(std::vector<double> { 4, 5 })];
			}
			if (offPos == K || offPos == P) {
				offOnField[i]->gameState.action = KICKING;
			}
			if (offPos == TE) {
				if (play == PASS) {
					const Action a[2] = { BLOCKING, RECEIVING };
					offOnField[i]->gameState.action = a[RNG::randomWeightedIndex(std::vector<double> { 2, 5 })];
				} else {
					offOnField[i]->gameState.action = BLOCKING;
				}
			}
			if (offPos == WR) {
				offOnField[i]->gameState.action = (play == PASS) ? RECEIVING : BLOCKING;
			}
		}

		// If play is a pass, assign a runner
		if (play == RUN) {
			std::vector<Player*> halfbacks = getPlayersAtPosition(offOnField, HB);
			Player* quarterback = getPlayersAtPosition(offOnField, QB)[0]; // if this is out of bounds, something's wrong
			if (halfbacks.size() == 0) {
				quarterback->gameState.action = RUSHING;
			} else {
				// TODO: decide whether or not to run QB based on QB's running ability
				if (std::rand() % 100 < 15) quarterback->gameState.action = RUSHING;
				else (*select_randomly(halfbacks.begin(), halfbacks.end()))->gameState.action = RUSHING;
			}
		}

		// Now assign the defense
		for (int i = 0; i < 11; ++i) {
			if (i < 4 || play == RUN) {
				defOnField[i]->gameState.action = BLITZING;
			} else {
				defOnField[i]->gameState.action = COVERING;
			}
		}

		return std::make_pair(offOnField, defOnField);
	}

	void updateGameState(PlayResult result) {
		if (!result.specialTeamsPlay) {
			gameState.gainYards(result.yards);
		}
		if (result.outcome == FIELD_GOAL_SCORED) {
			gameState.scoreFieldGoal();
		}
		if (result.outcome == FIELD_GOAL_MISSED || result.incompleteReason == PASS_INCOMPLETE_INTERCEPTED) {
			gameState.flipPossession();
		}
		if (result.outcome == BALL_PUNTED) {
			gameState.puntBall(result.yards);
		}
		if (homePossession != gameState.homeHasPossession()) {
			homePossession = gameState.homeHasPossession();
			std::swap(offense, defense);
			std::swap(offStats, defStats);
		}
	}

	void runInjuryRisks(Field& field, PlayResult result) {
		bool touchdown = (result.yards >= gameState.getYardLine());
		const std::vector<std::vector<Player*>> sides = { field.first, field.second };
		School* currentSchool = gameState.homeHasPossession() ? home : away;
		for (auto& side : sides) {
			for (auto player : side) {
				const Action action = player->gameState.action;
				if (action == RUSHING) player->runInjuryRisk(INJURY_RISK_HIGH);
				if (action == BLOCKING) player->runInjuryRisk(INJURY_RISK_MEDIUM);
				if (action == HANDINGOFF) player->runInjuryRisk(INJURY_RISK_LOW);
				if (action == KICKING) player->runInjuryRisk(INJURY_RISK_MEDIUM);

				if (action == PASSING) {
					if (result.carrier == player && !touchdown) player->runInjuryRisk(INJURY_RISK_HIGH);
					else player->runInjuryRisk(INJURY_RISK_LOW);
				}
				if (action == RECEIVING) {
					if (result.carrier == player && !touchdown) player->runInjuryRisk(INJURY_RISK_HIGH);
					else player->runInjuryRisk(INJURY_RISK_LOW);
				}

				if (player == result.defender && !touchdown) player->runInjuryRisk(INJURY_RISK_HIGH);
				else {
					if (action == BLITZING) player->runInjuryRisk(INJURY_RISK_MEDIUM);
					if (action == COVERING) player->runInjuryRisk(INJURY_RISK_LOW);
				}

				if (player->isInjured()) {
					printPlay(currentSchool->getName() + " " + player->getPositionedName() + " was injured on the play");
				}
			}
			if (currentSchool == home) currentSchool = away;
			else currentSchool = home;
		}
	}

	void recordPlayResult(PlayResult result) {
		for (auto& str : result.messages) printPlay(str);
		if (result.yards > gameState.getYardLine()) result.yards = gameState.getYardLine();
		int gain = result.yards;
		bool touchdown = (gain >= gameState.getYardLine());

		if (!result.specialTeamsPlay) defStats->recordYardsAllowed(gain);
		if (result.defender != nullptr && result.outcome != BALL_PASSED_INCOMPLETE) {
			defStats->recordTackle(result.defender);
			if (gain < 0) defStats->recordTFL(result.defender);
			if (result.outcome == PASSER_SACKED) defStats->recordSack(result.defender);
		}
		if (result.outcome == BALL_PASSED_COMPLETE || result.outcome == BALL_PASSED_INCOMPLETE) {
			offStats->recordPass(result.thrower, result.carrier, gain, result.outcome == BALL_PASSED_COMPLETE);
			if (touchdown) offStats->recordPassingTD(result.thrower, result.carrier);
			if (result.incompleteReason == PASS_INCOMPLETE_INTERCEPTED) {
				offStats->recordINTThrown(result.thrower);
				defStats->recordINTCaught(result.defender);
			}
			if (result.incompleteReason == PASS_INCOMPLETE_DROPPED) offStats->recordDrop(result.carrier);
			if (result.incompleteReason == PASS_INCOMPLETE_DEFLECTED) defStats->recordPassDefense(result.defender);
		}
		if (result.outcome == BALL_RUSHED) {
			offStats->recordRush(result.carrier, gain);
			if (touchdown) offStats->recordRushingTD(result.carrier);
		}
		if (result.outcome == FIELD_GOAL_SCORED || result.outcome == FIELD_GOAL_MISSED) {
			offStats->recordFGAttempt(result.carrier, result.outcome == FIELD_GOAL_SCORED, result.yards);
		}
		if (result.outcome == BALL_PUNTED) {
			offStats->recordPunt(result.carrier, result.yards);
		}
	}

	void runOneSnap() {
		PlayType play = decidePlay();
		OffensiveFormation form = decideFormation(play);
		Field field = applyFormation(form, play);
		PlayResult result = GamePlayExecutor::executePlay(play, field, gameState.getYardLine());
		recordPlayResult(result);
		runInjuryRisks(field, result);
		updateGameState(result);
		gameState.printStatus();
	}

	void gameLoop() {
		if (gameState.countdownClock()) {
			// Clock has hit zero
			if (gameState.getAwayScore() != gameState.getHomeScore()) {
				// Overtime time, baby!
			}
			return;
		}
		runOneSnap();
		gameLoop();
	}

public:
	GamePlayer(School* awaySchool, School* homeSchool) : away{ awaySchool }, home{ homeSchool } {
		offense = awaySchool->getRoster();
		defense = homeSchool->getRoster();
		offStats = new TeamStats();
		defStats = new TeamStats();
		gameState.setCompetingSchools(homeSchool, awaySchool);
	}

	GameResult startRealTimeGameLoop(bool playByPlay) {
		printPlayByPlay = playByPlay;
		gameState.printPlayByPlay = playByPlay;
		if (playByPlay)
			std::cout << "========== " << str_upper(away->getRankedName()) << " vs. " << str_upper(home->getRankedName()) << " ==========\n\n";
		printPlay(away->getName() + " to start with the ball");
		gameLoop();
		if (homePossession) std::swap(offStats, defStats);
		offStats->points = gameState.getAwayScore();
		defStats->points = gameState.getHomeScore();
		bool homeWins = (gameState.getAwayScore() < gameState.getHomeScore());
		if (playByPlay)
			std::cout << "GAME OVER! Final score is " << away->getName() << ": " << gameState.getAwayScore() << ", " << home->getName() << ": " <<
			gameState.getHomeScore() << "\n";
		return GameResult{ offStats, defStats, !homeWins, homeWins };
	}
};
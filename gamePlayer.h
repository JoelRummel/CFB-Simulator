#pragma once

#include "school.h"

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

class GamePlayer {
  private:
	int clock;
	int quarter;
	int awayPoints;
	int homePoints;
	School* home;
	School* away;
	Roster* offense;
	Roster* defense;
	TeamStats* offStats;
	TeamStats* defStats;
	bool homePossession;
	int yardLine;
	int down;
	int lineToGain;

	bool printPlayByPlay = false;

	enum PlayType { OUTSIDERUN, INSIDERUN, DRAW, PLAYACTION, HAILMARY, PASS, KICK, PUNT, KICKOFF, ONSIDEKICK };

	enum OffensiveFormation { WR3TE1, TE2_SPLIT, TE2_STRONG, GOALLINE, TWINBACKS, WR4, EMPTY_5WR, EMPTY_4WR, PUNTFORM, KICKFG };

	// This MUST MATCH UP with the order of the OffensiveFormation enum!
	const std::vector<std::vector<Needs>> OffensivePersonnel {
		{ // 3WR 1TE
		  Needs { OL, 5 }, Needs { QB, 1 }, Needs { HB, 1 }, Needs { TE, 1 }, Needs { WR, 3 } },
		{ // 2TE Split
		  Needs { OL, 5 }, Needs { QB, 1 }, Needs { HB, 1 }, Needs { TE, 2 }, Needs { WR, 2 } },
		{ // 2TE Strong
		  Needs { OL, 5 }, Needs { QB, 1 }, Needs { HB, 1 }, Needs { TE, 2 }, Needs { WR, 2 } },
		{ // Goal Line
		  Needs { OL, 5 }, Needs { QB, 1 }, Needs { HB, 2 }, Needs { TE, 3 } },
		{ // Twin Backs
		  Needs { OL, 5 }, Needs { QB, 1 }, Needs { HB, 2 }, Needs { TE, 1 }, Needs { WR, 2 } },
		{ // 4 WR
		  Needs { OL, 5 }, Needs { QB, 1 }, Needs { HB, 1 }, Needs { WR, 4 } },
		{ // Empty set 5WR
		  Needs { OL, 5 }, Needs { QB, 1 }, Needs { WR, 5 } },
		{ // Empty set 4WR
		  Needs { OL, 5 }, Needs { QB, 1 }, Needs { WR, 4 }, Needs { TE, 1 } },
		{ // Punt
		  Needs { OL, 5 }, Needs { P, 1 }, Needs { TE, 2 }, Needs { HB, 1 }, Needs { CB, 2 } },
		{ // Kick field goal
		  Needs { OL, 5 }, Needs { K, 1 }, Needs { QB, 1 }, Needs { TE, 2 }, Needs { HB, 2 } }
	};

	const std::vector<std::vector<Needs>> DefensivePersonnel { { Needs { DL, 4 }, Needs { LB, 3 }, Needs { CB, 2 }, Needs { S, 2 } } };

	struct PlayResult {
		Player* carrier;
		Player* tackler;
		int yards;
		Player* thrower = nullptr;
		bool complete = false;
		bool intercepted = false;
	};

	std::string clockAsStr() {
		int minutes = (int)std::floor(clock / 60);
		int seconds = clock % 60;
		std::string str = (minutes < 10 ? " " : "") + std::to_string(minutes) + ":" + std::to_string(seconds) + (seconds < 10 ? "0 " : " ");
		str += quarter == 1 ? "1st" : quarter == 2 ? "2nd" : quarter == 3 ? "3rd" : "4th";
		str += " --- ";
		return str;
	}

	std::string yardLineAsStr() {
		std::string str = yardLine > 50 ? "(v) " : "(^) ";
		str += std::to_string(yardLine > 50 ? (-(yardLine - 100)) : yardLine) + " yard line";
		return str;
	}

	void printPlay(std::string msg) {
		if (!printPlayByPlay) return;
		std::cout << clockAsStr() << msg << std::endl;

		// using namespace std::chrono_literals;  // ns, us, ms, s, h, etc.
		// using std::chrono::system_clock;
		// std::this_thread::sleep_for(500ms);
	}

	void flipPossession() {
		down = 1;
		yardLine = std::abs(yardLine - 100);
		lineToGain = yardLine - 10;
		std::swap(offense, defense);
		std::swap(offStats, defStats);
		homePossession = !homePossession;
		std::string name = homePossession ? home->getName() : away->getName();
		printPlay("The " + name + " offense takes over at the " + yardLineAsStr() + ".");
	}

	void printStatus() {
		std::string suffix = down == 1 ? "st" : down == 2 ? "nd" : down == 3 ? "rd" : "th";
		int toGo = yardLine - lineToGain;
		std::string secondPart = lineToGain > 0 ? std::to_string(toGo) : "Goal";
		printPlay(std::to_string(down) + suffix + " and " + secondPart + ". Ball at the " + yardLineAsStr());
	}

	PlayType decidePlay() {
		// if (true) return INSIDERUN;
		// if (down == 4) { return yardLine > 40 ? PUNT : KICK; }
		if (down == 3 && yardLine - lineToGain > 5) return (std::rand() % 100 < 90 ? PASS : INSIDERUN);
		if (down == 4) {
			int fgDistance = yardLine + 17;
			if (fgDistance > 50) {
				if (yardLine > 40 || yardLine - lineToGain > 5) return PUNT;
				return (std::rand() & 100 < 70 ? PASS : INSIDERUN);
			} else
				return KICK;
		}
		if (yardLine - lineToGain < 4) return (std::rand() % 100 < 30 ? PASS : INSIDERUN);
		return (std::rand() % 100 < 60) ? PASS : (std::rand() % 100 < 50 ? OUTSIDERUN : INSIDERUN);
	}

	bool countdownClock() {
		clock -= 25;
		if (clock <= 0) {
			clock = 900;
			quarter++;
			if (quarter == 5) { return true; }
			if (quarter == 3) {
				printPlay("\n========== Halftime. ==========\n");
				flipPossession();
				yardLine = 75;
				lineToGain = 65;
			}
		}
		return false;
	}

	OffensiveFormation decideFormation(PlayType play) {
		if (play == KICK) return KICKFG;
		if (play == PUNT) return PUNTFORM;
		if (play == HAILMARY) return WR4;
		if (yardLine < 3) return GOALLINE;
		if (play == PASS && down > 2 && yardLine - lineToGain > 5) {
			// need to be more aggressive
			const OffensiveFormation forms[4] = { WR4, EMPTY_4WR, EMPTY_5WR, WR3TE1 };
			return forms[RNG::randomWeightedIndex(std::vector<double> { 10, 8, 10, 3 })];
		}
		if (play == OUTSIDERUN) {
			const OffensiveFormation forms[5] = { TWINBACKS, TE2_SPLIT, TE2_STRONG, WR3TE1, WR4 };
			return forms[RNG::randomWeightedIndex(std::vector<double> { 10, 10, 13, 7, 5 })];
		}
		if (play == INSIDERUN) {
			const OffensiveFormation forms[7] = { TWINBACKS, TE2_SPLIT, TE2_STRONG, WR3TE1, WR4, EMPTY_4WR, EMPTY_5WR };
			return forms[RNG::randomWeightedIndex(std::vector<double> { 8, 12, 10, 8, 5, 3, 2 })];
		}
		if (play == PASS) {
			const OffensiveFormation forms[7] = { TWINBACKS, TE2_SPLIT, TE2_STRONG, WR3TE1, WR4, EMPTY_4WR, EMPTY_5WR };
			return forms[RNG::randomWeightedIndex(std::vector<double> { 4, 4, 3, 10, 9, 7, 7 })];
		}
		return WR3TE1;
	}

	bool engageTackler(Player* ballCarrier, Player* tackler, double tacklerMod = 1) {
		// Ball carrier does a speed break or strength break depending on tackler stats
		int speedDiff = ballCarrier->getRating(SPEED) - tackler->getRating(SPEED);
		int strenDiff = ballCarrier->getRating(STRENGTH) - tackler->getRating(STRENGTH);
		double breakRating = 0;
		double tackleRating = 0;
		if (speedDiff > strenDiff) {
			breakRating = (ballCarrier->getRating(SPEED) + ballCarrier->getRating(BREAKTACKLE)) / 2;
			tackleRating = (tackler->getRating(SPEED) + tackler->getRating(TACKLE)) / 2;
		} else {
			breakRating = (ballCarrier->getRating(STRENGTH) + ballCarrier->getRating(BREAKTACKLE)) / 2;
			tackleRating = (tackler->getRating(STRENGTH) + tackler->getRating(BREAKTACKLE)) / 2;
		}
		int factor = std::round(0.2 * (breakRating - tackleRating)) + 35;
		//(tackleRating * tacklerMod) - (breakRating * (ballCarrier->gameState.energy / 100));
		bool tackled = (std::rand() % 100) > factor;
		if (tackled)
			printPlay(positionToStr(ballCarrier->getPosition()) + " " + ballCarrier->getName() + " tackled by " +
					  positionToStr(tackler->getPosition()) + " " + tackler->getName());
		else
			ballCarrier->gameState.energy -= 15;
		return tackled;
	}

	std::vector<Player*> getPlayersInZone(const std::vector<Player*>& playerSet, Zone zone) {
		std::vector<Player*> players;
		for (auto player : playerSet) {
			if (player->gameState.zone == zone) players.push_back(player);
		}
		return players;
	}
	std::vector<Player*> getPlayersAtPosition(const std::vector<Player*>& playerSet, Position pos) {
		std::vector<Player*> players;
		for (auto player : playerSet) {
			if (player->getPosition() == pos) players.push_back(player);
		}
		return players;
	}

	int getYardsFromCatch(Player* catcher) {
		if (catcher->gameState.zone == DEEP) return RNG::randomNumberNormalDist(25, 8);
		if (catcher->gameState.zone == MIDDLE) return RNG::randomNumberNormalDist(10, 2);
		if (catcher->gameState.zone == SHORTLEFT || catcher->gameState.zone == SHORTRIGHT) return RNG::randomNumberNormalDist(4, 1);
		std::cout << "ERR: invalid catcher zone: " << catcher->gameState.zone << std::endl;
		assert(false);
	}

	std::pair<std::vector<Player*>, std::vector<Player*>> applyFormation(OffensiveFormation form, PlayType play) {
		std::vector<Player*> offOnField = offense->getElevenMen(OffensivePersonnel[form]);
		std::vector<Player*> defOnField = defense->getElevenMen(DefensivePersonnel[0]);
		int wrCounter = 0; // used to spread out the WRs
		int hbCounter = 0;
		for (int i = 0; i < 11; ++i) {
			Position offPos = offOnField[i]->getPosition();

			if (offPos == OL) {
				offOnField[i]->gameState.target = defOnField[i % 4];
				offOnField[i]->gameState.zone = LINE;
				offOnField[i]->gameState.action = (play == INSIDERUN || play == OUTSIDERUN) ? RUNBLOCKING : PASSBLOCKING;
			}
			if (offPos == QB || offPos == HB || offPos == K || offPos == P) {
				if (offPos == HB) hbCounter++;
				offOnField[i]->gameState.zone = BACKFIELD;
				if (play == INSIDERUN || play == OUTSIDERUN) {
					if (offPos == QB) {
						offOnField[i]->gameState.action = HANDINGOFF;
						if (form == EMPTY_4WR || form == EMPTY_5WR) offOnField[i]->gameState.action = RUSHING;
					} else if (offPos == HB)
						offOnField[i]->gameState.action = hbCounter == 1 ? RUSHING : RUNBLOCKING;
				} else {
					if (offPos == QB) offOnField[i]->gameState.action = play == PASS ? PASSING : HANDINGOFF;
					else if (offPos == HB) {
						offOnField[i]->gameState.action = (std::rand() % 100 < 60) ? RUNNINGMIDDLE : PASSBLOCKING;
					} else
						offOnField[i]->gameState.action = KICKING;
				}
			}
			if (offPos == TE) {
				if (play == PASS) {
					const Action a[3] = { PASSBLOCKING, RUNNINGMIDDLE, RUNNINGDEEP };
					offOnField[i]->gameState.action = a[RNG::randomWeightedIndex(std::vector<double> { 8, 13, 6 })];
					if (offOnField[i]->gameState.action == PASSBLOCKING) { offOnField[i]->gameState.target = defOnField[std::rand() % 4]; }
				} else {
					offOnField[i]->gameState.action = RUNBLOCKING;
					offOnField[i]->gameState.target = defOnField[i - 2];
				}

				/*if (i == 8)
				{
						// This is the second tight end
						if (form == TE2_SPLIT)
								offOnField[i]->gameState.zone = LINELEFT;
						else
								offOnField[i]->gameState.zone = LINERIGHT;
				}
				else
				{
						offOnField[i]->gameState.zone = LINERIGHT;
				}*/
				offOnField[i]->gameState.zone = LINE;
			}
			if (offPos == WR) {
				if (wrCounter % 2 == 0) offOnField[i]->gameState.zone = SHORTLEFT;
				else
					offOnField[i]->gameState.zone = SHORTRIGHT;
				if (play == PASS) {
					const Action a[3] = { RUNNINGSHORT, RUNNINGMIDDLE, RUNNINGDEEP };
					offOnField[i]->gameState.action = a[RNG::randomWeightedIndex(std::vector<double> { 6, 7, 13 })];
				} else {
					offOnField[i]->gameState.action = RUNBLOCKING;
					offOnField[i]->gameState.zone = MIDDLE;
				}

				wrCounter++;
			}
		}

		for (int i = 0; i < 11; ++i) {
			if (i < 4) {
				defOnField[i]->gameState.zone = LINE;
				defOnField[i]->gameState.action = BLITZING;
			} else if (i < 9) {
				defOnField[i]->gameState.action = COVERING;
				defOnField[i]->gameState.target = offOnField[i + 2];
				defOnField[i]->gameState.zone = offOnField[i + 2]->gameState.zone;
				if (offOnField[i + 2]->gameState.zone == BACKFIELD) defOnField[i]->gameState.zone = MIDDLE;
			} else {
				defOnField[i]->gameState.action = ZONECOVERING;
				defOnField[i]->gameState.zone = DEEP;
				defOnField[i]->gameState.target = nullptr;
			}
		}
		return std::make_pair(offOnField, defOnField);
	}

	PlayResult executePlay(PlayType play, OffensiveFormation form) {
		std::pair<std::vector<Player*>, std::vector<Player*>> field = applyFormation(form, play);
		Player* ballCarrier;
		Player* passerMemory = nullptr;
		for (auto offensivePlayer : field.first) {
			if (offensivePlayer->gameState.action == RUSHING || offensivePlayer->gameState.action == PASSING ||
				offensivePlayer->gameState.action == KICKING) {
				ballCarrier = offensivePlayer;
				if (ballCarrier->gameState.action == RUSHING) {
					if (ballCarrier->getPosition() == QB) {
						printPlay("QB " + ballCarrier->getName() + " runs with the ball himself...");
					} else {
						printPlay(positionToStr(ballCarrier->getPosition()) + " " + ballCarrier->getName() + " takes the handoff...");
					}
				}
				break;
			}
		}
		if (play == KICK) {
			double accFactor = ballCarrier->getRating(KICKACCURACY) / 100.0;
			double powFactor = ballCarrier->getRating(KICKPOWER) / 100.0;
			double odds = -0.027 * (3 - (1.5 * powFactor)) * pow(yardLine, 2);
			odds += 80 + (20 * accFactor);
			printPlay("K " + ballCarrier->getName() + " goes out for the field goal...");
			if (std::rand() % 100 < odds) {
				if (homePossession) homePoints += 3;
				else
					awayPoints += 3;
				printPlay("The kick is good! " + scoreString());
				flipPossession();
				yardLine = 75;
				lineToGain = 65;
			} else {
				printPlay("The kick is no good!");
				flipPossession();
			}
			return { nullptr, nullptr, 0 };
		}
		if (play == PUNT) {
			int yds = std::round(0.4 * ballCarrier->getRating(PUNTPOWER)) + 10;
			yds += RNG::randomNumberNormalDist(0, 3);
			if (yds > yardLine) yds = yardLine;
			yardLine -= yds;
			printPlay("P " + ballCarrier->getName() + " punts the ball " + std::to_string(yds) + " yards" +
					  (yardLine == 0 ? " for a touchback" : ""));
			if (yardLine == 0) { yardLine = 20; }
			flipPossession();
			return { nullptr, nullptr, 0 };
		}
		for (auto offensivePlayer : field.first) {
			offensivePlayer->gameState.energy = 30 + (std::rand() % 70);
			offensivePlayer->gameState.clash = -1;
		}
		for (auto defensivePlayer : field.second) { defensivePlayer->gameState.clash = -1; }
		Rating olr = (play == PASS) ? PASSBLOCK : RUNBLOCK;
		Rating dlr = (play == PASS) ? PASSRUSH : RUNSTOP;
		for (int ticker = 0; true; ++ticker) {
			assert(!(ballCarrier->getPosition() == WR && ballCarrier->gameState.action == PASSING));
			// Theoretically we shouldn't care at this point what the play is, the
			// roles have been decided. One tick represents roughly one fifth of a
			// second. Let's work from the line and out

			std::vector<Player*> blockers = getPlayersInZone(field.first, LINE);
			std::vector<Player*> blitzers = getPlayersInZone(field.second, LINE);
			for (auto blitzer : blitzers) {
				if (blitzer->gameState.action != BLITZING) continue;
				std::vector<Player*> blockedBy;
				for (auto blocker : blockers)
					if (blocker->gameState.target == blitzer) blockedBy.push_back(blocker);
				if (blitzer->gameState.clash == -1) {
					// Initiate the clash!!
					// Blockers should still be ordered by OVR... ideally...

					double compositeBlockerRating = 0;
					for (int i = 0; i < (int)blockedBy.size(); ++i) compositeBlockerRating += blockedBy[i]->getRating(olr) / (i + 1);
					compositeBlockerRating *= 0.75;
					int clashPosition = (0.6 * blitzer->getRating(dlr)) - std::round(compositeBlockerRating) + 20;
					clashPosition += std::rand() % 55;
					if (clashPosition < 10) clashPosition = 10;
					blitzer->gameState.clash = clashPosition;
				} else {
					blitzer->gameState.clash += 1;
					if (blitzer->gameState.clash >= 75) {
						for (auto blocker : blockers) {
							blocker->gameState.zone = OUTOFPLAY;
							blocker->gameState.target = nullptr;
						}
						blitzer->gameState.zone = BACKFIELD;
						blitzer->gameState.clash = -1;
					}
				}
			}

			blitzers = getPlayersInZone(field.second, BACKFIELD);
			blockers = getPlayersInZone(field.first, BACKFIELD);
			for (auto blitzer : blitzers) {
				Player* blockedBy = nullptr;
				for (auto blocker : blockers) {
					if (blocker->gameState.target == blitzer) {
						blockedBy = blocker;
						break;
					}
				}
				if (blockedBy == nullptr) {
					// Nobody blocking me currently. Find someone
					for (auto blocker : blockers) {
						if (blocker->gameState.action == RUNBLOCKING ||
							blocker->gameState.action == PASSBLOCKING && blocker->gameState.target == nullptr) {
							blockedBy = blocker;
							blocker->gameState.target = blitzer;
							int clashPosition = (blitzer->getRating(dlr) * 0.6) - std::round(blocker->getRating(olr) * 0.75) + 20;
							clashPosition += std::rand() % 50;
							if (clashPosition < 10) clashPosition = 10;
							blitzer->gameState.clash = clashPosition;
						}
					}
					if (blockedBy == nullptr) {
						// Attempt sack/TFL
						if (play == PASS && std::rand() % 200 < ballCarrier->getRating(PASSVISION)) {
							printPlay("QB " + ballCarrier->getName() + " gets pressured and throws the ball away.");
							return PlayResult { ballCarrier, nullptr, 0, passerMemory };
						}
						if (engageTackler(ballCarrier, blitzer)) {
							if (ballCarrier->gameState.action == PASSING) {
								printPlay("That's a sack for the defense!");
								defStats->recordSack(blitzer);
								return PlayResult { ballCarrier, blitzer, RNG::randomNumberNormalDist(-6, 1) };
							}
							return PlayResult { ballCarrier, blitzer, RNG::randomNumberNormalDist(-1, 1) };
						} else {
							blitzer->gameState.zone = OUTOFPLAY;
							if (play == PASS) {
								// depending on the quarterback's capabilities, rush or keep tryina pass (TODO)
								printPlay("QB " + ballCarrier->getName() + " gets flushed out of the pocket and runs with the ball...");
								ballCarrier->gameState.action = RUSHING;
								ballCarrier->gameState.zone = LINE;
							}
						}
					}
				}
				if (blockedBy != nullptr) {
					blitzer->gameState.clash += 2;
					if (blitzer->gameState.clash >= 75) { blockedBy->gameState.zone = OUTOFPLAY; }
				} else {
					// We must have attempted a sack and failed
					blitzer->gameState.zone = OUTOFPLAY;
				}
			}

			if (ballCarrier->gameState.action == RUSHING) {
				ballCarrier->gameState.tick++;
				if (ballCarrier->getRating(SPEED) > (std::rand() % 175)) ballCarrier->gameState.tick++;
				if (ballCarrier->gameState.zone == BACKFIELD && ballCarrier->gameState.tick >= 17) { ballCarrier->gameState.zone = LINE; }
			}

			if (ballCarrier->gameState.zone == LINE) {
				// get defender on line who is losing clash the worst
				// poor HB vision could make him pick a worse defender
				blitzers = getPlayersInZone(field.second, LINE);
				if (blitzers.size() <= 2) ballCarrier->gameState.zone = MIDDLE;
				else {
					int worstClash = blitzers[0]->gameState.clash;
					Player* worstBlitzer = blitzers[0];
					for (auto blitzer : blitzers) {
						if (blitzer->gameState.clash < worstClash && blitzer->gameState.action == BLITZING) {
							if (ballCarrier->getRating(RUNVISION) < std::rand() % 120) continue;
							worstClash = blitzer->gameState.clash;
							worstBlitzer = blitzer;
						}
					}
					if (engageTackler(ballCarrier, worstBlitzer, worstClash / 100.0)) {
						return PlayResult { ballCarrier, worstBlitzer, RNG::randomNumberNormalDist(4, 1) };
					}
					ballCarrier->gameState.zone = MIDDLE;
				}
			}

			// this is the point at which the runner could actually be a receiver
			if (ballCarrier->gameState.zone == MIDDLE) {
				// attempt tackle w/ manned linebacker
				blitzers = getPlayersInZone(field.second, MIDDLE);
				for (auto blitzer : blitzers) {
					if (blitzer->gameState.target == ballCarrier)
						if (engageTackler(ballCarrier, blitzer))
							return PlayResult { ballCarrier, blitzer, RNG::randomNumberNormalDist(7, 2), passerMemory, true };
				}
				// chance to get tackled by other middle defenders
				for (auto blitzer : blitzers) {
					// do a mini-clash
					int diff = 50;
					if (blitzer->gameState.target != nullptr) diff = blitzer->getRating(RUNSTOP) - blitzer->gameState.target->getRating(RUNBLOCK);
					if (std::rand() % 100 < 50 + diff)
						if (engageTackler(ballCarrier, blitzer))
							return PlayResult { ballCarrier, blitzer, RNG::randomNumberNormalDist(7, 2), passerMemory, true };
				}
				ballCarrier->gameState.zone = DEEP;
			}

			if (ballCarrier->gameState.zone == DEEP) {
				blitzers = getPlayersInZone(field.second, DEEP);
				for (auto blitzer : blitzers) {
					// do a mini-clash
					int diff = 50;
					if (blitzer->gameState.target != nullptr) diff = blitzer->getRating(RUNSTOP) - blitzer->gameState.target->getRating(RUNBLOCK);
					if (std::rand() % 100 < 50 + diff)
						if (engageTackler(ballCarrier, blitzer)) {
							return PlayResult { ballCarrier, blitzer, RNG::randomNumberNormalDist(15, 4), passerMemory, true };
						}
				}
				return PlayResult { ballCarrier, nullptr, 100, passerMemory, true }; // touchdown bitches
			}

			for (Player* receiver : field.first) {
				if (!receiver->gameState.isReceiving()) continue;
				bool inZone = receiver->gameState.isInTargetZone();
				if (ticker == 0) inZone = false;
				receiver->advanceTick();
				if (!inZone && receiver->gameState.isInTargetZone()) {
					// receiver has newly reached their target zone, initiate a clash
					// first, assign a zone defender w/ no target
					std::vector<Player*> coveredBy;
					for (auto defender : getPlayersInZone(field.second, receiver->gameState.zone)) {
						if (defender->gameState.target == nullptr && defender->gameState.action == ZONECOVERING) {
							defender->gameState.target = receiver;
							break;
						}
					}
					// now, actually collect everyone covering the receiver
					for (auto defender : field.second)
						if (defender->gameState.target == receiver) coveredBy.push_back(defender);
					// TODO: sort vector by best player
					// finally, initiate a clash
					if (coveredBy.empty()) {
						// uncovered, the defense is bad lmao
						receiver->gameState.clash = 75;
					} else {
						double compositeCoverRating = 0;
						for (int i = 0; i < (int)coveredBy.size(); ++i) compositeCoverRating += coveredBy[i]->getRating(PASSCOVER) / (i + 1);
						compositeCoverRating *= 0.6;
						receiver->gameState.coverRating = compositeCoverRating;
						int clashPosition = std::round(receiver->getRating(GETTINGOPEN) * 0.6) - std::round(compositeCoverRating) + 40;
						clashPosition += std::rand() % 55;
						if (clashPosition < 10) clashPosition = 10;
						receiver->gameState.clash = clashPosition;
					}
				}

				// general tick forward for all receivers in target zone
				if (receiver->gameState.isInTargetZone()) { receiver->gameState.clash += 2; }
			}

			for (auto player : field.second) {
				if (player->gameState.action == COVERING && player->gameState.target != nullptr &&
					player->gameState.target->gameState.isReceiving()) {
					player->gameState.zone = player->gameState.target->gameState.zone;
				}
			}

			if (ballCarrier->gameState.action == PASSING) {
				// random chance to seek a receiver
				if (std::rand() % 100 < ballCarrier->getRating(PASSVISION)) {
					std::vector<Player*> receiversDeep;
					std::vector<Player*> receiversMiddle;
					std::vector<Player*> receiversShort;
					for (auto player : field.first) {
						if (player->gameState.isReceiving() && player->gameState.isInTargetZone()) {
							if (player->gameState.zone == DEEP) receiversDeep.push_back(player);
							else if (player->gameState.zone == MIDDLE)
								receiversMiddle.push_back(player);
							else
								receiversShort.push_back(player);
						}
					}
					RNG::shuffle(receiversDeep);
					RNG::shuffle(receiversMiddle);
					RNG::shuffle(receiversShort);
					receiversDeep.insert(receiversDeep.end(), receiversMiddle.begin(), receiversMiddle.end());
					receiversDeep.insert(receiversDeep.end(), receiversShort.begin(), receiversShort.end());

					for (auto target : receiversDeep) {
						int apparentOpenness = target->gameState.clash;
						int modBound = std::round(30 - (ballCarrier->getRating(PASSVISION) * 0.3));
						apparentOpenness += RNG::randomNumberUniformDist(-modBound, modBound);
						if (apparentOpenness >= 75) {
							// attempt to throw!
							std::vector<Player*> coverers;
							for (auto player : field.second)
								if (player->gameState.target == target) coverers.push_back(player);
							std::string passDesc = "short";
							if (target->gameState.zone == MIDDLE) passDesc = "medium";
							if (target->gameState.zone == DEEP) passDesc = "deep";
							printPlay("QB " + ballCarrier->getName() + " throws a " + passDesc + " pass to " + positionToStr(target->getPosition()) +
									  " " + target->getName() + "...");
							// now see if the QB accurately read the receiver's openness
							if (target->gameState.clash < 75) {
								// nope sorry!
								// slim chance for the receiver to bail out the QB
								if (((std::rand() % 200) + 30) < target->getRating(CATCH)) {
									printPlay("The receiver was covered but he still came down with "
											  "the ball!");
									// get arbitrary defender w/ receiver as target
									Player* tackler;
									for (auto player : field.second)
										if (player->gameState.target == target) tackler = player;
									return PlayResult { target, tackler, getYardsFromCatch(target), ballCarrier, true };
								} else {
									// moderate chance for an interception...
									Player* interceptor = *select_randomly(coverers.begin(), coverers.end());
									if (std::rand() % 200 < interceptor->getRating(CATCH) * 0.5) {
										printPlay(positionToStr(interceptor->getPosition()) + " " + interceptor->getName() +
												  " had the receiver covered and intercepted the ball!");
										yardLine -= getYardsFromCatch(target);
										if (yardLine <= 0) yardLine = 20;
										return PlayResult { interceptor, nullptr, 0, ballCarrier, false, true };
									}
									printPlay("Incomplete - the receiver was covered up.");
									return PlayResult { target, nullptr, 0, ballCarrier };
								}
							} else {
								// run separate odds of missed throw + deflected throw + dropped
								// throw bigger throw penalties for deeper passes
								double penaltyFactor = 1;
								if (target->gameState.zone == MIDDLE) penaltyFactor = 0.9;
								if (target->gameState.zone == DEEP) penaltyFactor = 0.75;
								// missed throw?
								if (std::rand() % 115 > 80 + (ballCarrier->getRating(PASSACCURACY) * 0.2 * penaltyFactor)) {
									Player* interceptor = *select_randomly(coverers.begin(), coverers.end());
									if (std::rand() % 300 < interceptor->getRating(CATCH) * 0.5) {
										printPlay("The QB was off-target and " + positionToStr(interceptor->getPosition()) + " " +
												  interceptor->getName() + " intercepted the ball!");
										yardLine -= getYardsFromCatch(target);
										if (yardLine <= 0) yardLine = 20;
										return PlayResult { interceptor, nullptr, 0, ballCarrier, false, true };
									}
									printPlay("Incomplete - the QB threw an inaccurate ball.");
									return PlayResult { target, nullptr, 0, ballCarrier };
								}
								// deflected ball?
								if (std::rand() % 150 < 0.5 * target->gameState.coverRating) {
									Player* interceptor = *select_randomly(coverers.begin(), coverers.end());
									if (std::rand() % 300 < interceptor->getRating(CATCH) * 0.5) {
										printPlay(positionToStr(interceptor->getPosition()) + " " + interceptor->getName() +
												  " made a play on the ball and intercepted it!");
										yardLine -= getYardsFromCatch(target);
										if (yardLine <= 0) yardLine = 20;
										return PlayResult { interceptor, nullptr, 0, ballCarrier, false, true };
									}
									printPlay("Incomplete - the pass was deflected by the defense.");
									// pick random defender to award the pass defense to
									defStats->recordPassDefense(*select_randomly(coverers.begin(), coverers.end()));
									return PlayResult { target, nullptr, 0, ballCarrier };
								}
								if (std::rand() % 100 > 70 + target->getRating(CATCH) * 0.3) {
									printPlay("Incomplete - the receiver dropped the ball.");
									offStats->recordDrop(target);
									return PlayResult { target, nullptr, 0, ballCarrier };
								}
								printPlay("Pass completed!");
								passerMemory = ballCarrier;
								ballCarrier->gameState.zone = OUTOFPLAY;
								ballCarrier->gameState.action = HANDINGOFF; // basically nothing
								ballCarrier = target;
								ballCarrier->gameState.action = RUSHING;
								// attempt to tackle new ballcarrier w/ all targets
								for (auto defender : field.second) {
									if (defender->gameState.target == ballCarrier) {
										if (engageTackler(ballCarrier, defender, 0.8)) {
											return PlayResult { ballCarrier, defender, getYardsFromCatch(ballCarrier), passerMemory, true };
										}
									}
								}
								// successfully broke all tackles
								Zone z = ballCarrier->gameState.zone;
								if (z == SHORTLEFT || z == SHORTRIGHT) ballCarrier->gameState.zone = MIDDLE;
								else if (z == MIDDLE)
									ballCarrier->gameState.zone = DEEP;
								else if (z == DEEP)
									return PlayResult { ballCarrier, nullptr, 100, passerMemory, true };
								else {
									std::cout << "ERR: invalid receiver zone: " << ballCarrier->gameState.zone << std::endl;
									assert(false);
								}
								break; // lmao weird bugs happen without this
							}
						}
					}
				}
			}
		}
	}

	bool gainYards(int yards) {
		if (yards == 0) printPlay("No gain on the play");
		else
			printPlay(std::to_string(std::abs(yards)) + " yard " + (yards < 0 ? "loss" : "gain") + " on the play");
		yardLine -= yards;
		if (yardLine <= 0) return true;
		down++;
		if (yardLine > lineToGain) {
			if (down == 5) flipPossession();
		} else {
			down = 1;
			lineToGain = yardLine - 10;
		}
		return false;
	}

	void gameLoop() {
		if (countdownClock()) return; // Game is over
		PlayType play = decidePlay();
		OffensiveFormation form = decideFormation(play);
		PlayResult result = executePlay(play, form);
		if (result.carrier != nullptr) {
			int gain = result.yards;
			if (gain > yardLine) gain = yardLine;
			defStats->recordYardsAllowed(gain);
			if (result.thrower != nullptr) {
				// it was a PASS!
				offStats->recordPass(result.thrower, result.carrier, gain, result.complete);
				if (result.intercepted) {
					offStats->recordINTThrown(result.thrower);
					defStats->recordINTCaught(result.carrier);
					flipPossession();
				}
				assert(result.thrower->getPosition() == QB);
			} else {
				offStats->recordRush(result.carrier, gain);
			}
			if (result.tackler != nullptr && gain < yardLine) {
				defStats->recordTackle(result.tackler);
				if (gain < 0) defStats->recordTFL(result.tackler);
			}

			if (!result.intercepted && gainYards(gain)) {
				if (result.thrower != nullptr) offStats->recordPassingTD(result.thrower, result.carrier);
				else
					offStats->recordRushingTD(result.carrier);
				if (homePossession) homePoints += 7;
				else
					awayPoints += 7;
				yardLine = 25;
				lineToGain = 15;
				printPlay("TOUCHDOWN! " + scoreString());
				flipPossession();
			}
		}

		printStatus();
		gameLoop();
	}

	std::string scoreString() {
		return "The score is " + away->getName() + ": " + std::to_string(awayPoints) + ", " + home->getName() + ": " + std::to_string(homePoints);
	}

  public:
	GamePlayer(School* awaySchool, School* homeSchool) : away { awaySchool }, home { homeSchool } {
		clock = 900;
		quarter = 1;
		homePossession = false;
		offense = awaySchool->getRoster();
		defense = homeSchool->getRoster();
		yardLine = 75;
		down = 1;
		lineToGain = 65;
		homePoints = 0;
		awayPoints = 0;
		offStats = new TeamStats();
		defStats = new TeamStats();
	}

	GameResult startRealTimeGameLoop(bool playByPlay) {
		printPlayByPlay = playByPlay;
		if (playByPlay)
			std::cout << "========== " << str_upper(away->getRankedName()) << " vs. " << str_upper(home->getRankedName()) << " ==========\n\n";
		printPlay(away->getName() + " to start with the ball at the " + yardLineAsStr());
		gameLoop();
		if (homePossession) std::swap(offStats, defStats);
		offStats->points = awayPoints;
		defStats->points = homePoints;
		if (playByPlay)
			std::cout << "GAME OVER! Final score is " << away->getName() << ": " << awayPoints << ", " << home->getName() << ": " << homePoints
					  << "\n";
		return GameResult { offStats, defStats };
	}
};
#pragma once

#include "school.h"
#include "numberMaker.h"

using Field = std::pair<std::vector<Player*>, std::vector<Player*>>;

enum PlayType { RUN, PASS, KICK, PUNT, KICKOFF, ONSIDEKICK };

enum PlayOutcome {
    BALL_RUSHED,
    BALL_PASSED_COMPLETE,
    BALL_PASSED_INCOMPLETE,
    PASSER_SACKED,
    FIELD_GOAL_SCORED,
    FIELD_GOAL_MISSED,
    BALL_PUNTED
};

enum IncompleteReason {
    PASS_COMPLETED,
    PASS_INCOMPLETE_MISSED,
    PASS_INCOMPLETE_DROPPED,
    PASS_INCOMPLETE_DEFLECTED,
    PASS_INCOMPLETE_INTERCEPTED
};

struct PlayResult {
    PlayOutcome outcome;
    Player* carrier = nullptr;
    Player* defender = nullptr;
    int yards = 0;
    Player* thrower = nullptr;
    IncompleteReason incompleteReason;
    bool specialTeamsPlay = false;
    std::vector<std::string> messages;
};

std::vector<Player*> getPlayersPerformingAction(std::vector<Player*>& players, Action action) {
    std::vector<Player*> filteredPlayers;
    for (auto player : players) {
        if (player->gameState.action == action)
            filteredPlayers.push_back(player);
    }
    return filteredPlayers;
}

double getCompositeRating(std::vector<Player*> players, Rating rating) {
    SortByRating sbr(rating);
    std::sort(players.begin(), players.end(), sbr);
    double totalRating = 0;
    double factor = 1.0;
    for (auto& player : players) {
        totalRating += player->getRating(rating) * factor;
        factor -= 0.05; // Return on investment decreases by 5% per player
    }
    return totalRating;
}

class GamePlayExecutor {

    static int rerollRunYards(double advantage) { return NumberMaker::getRunYardsGained(advantage); }
    static int rerollPassYards(double advantage) { return NumberMaker::getPassYardsGained(); }

    static bool engageTackler(Player* ballCarrier, Player* tackler, std::vector<std::string>& messages) {
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
        bool tackled = (std::rand() % 100) > factor;
        if (tackled) {
            messages.push_back(ballCarrier->getPositionedName() + " tackled by " + tackler->getPositionedName());
        } else {
            messages.push_back(ballCarrier->getPositionedName() + " breaks a tackle by " + tackler->getPositionedName());
        }
        return tackled;
    }

    template<class Func>
    static Player* runUntilTackled(Player* ballCarrier, std::vector<Player*> defenders, std::vector<double> defenderRatings,
        int& yardsGained, Func rerollYards, double rerollAdvantage, std::vector<std::string>& messages) {
        Player* tackler = nullptr;
        std::vector<bool> failedTacklers(defenders.size(), false);
        do {
            int tacklerIndex;
            do {
                tacklerIndex = RNG::randomWeightedIndex(defenderRatings);
            } while (failedTacklers[tacklerIndex]);
            tackler = defenders[tacklerIndex];

            if (engageTackler(ballCarrier, tackler, messages)) break;

            int newYards = rerollYards(rerollAdvantage);
            if (newYards > yardsGained) yardsGained = newYards;
            failedTacklers[tacklerIndex] = true;
            if (std::find(failedTacklers.begin(), failedTacklers.end(), false) == failedTacklers.end()) {
                yardsGained = 100;
                break;
            }

        } while (true);
        return tackler;
    }

public:
    /**
     * Takes an offensive play type and formation, and simulates the play, mutating
     * both global game state and the game state of all 22 players sent onto the field.
     * Returns a play result to document what happened.
     */
    static PlayResult executePlay(PlayType play, Field field, int yardLine) {
        Player* ballCarrier;
        std::vector<std::string> messages;
        for (auto offensivePlayer : field.first) {
            if (offensivePlayer->gameState.action == RUSHING || offensivePlayer->gameState.action == PASSING ||
                offensivePlayer->gameState.action == KICKING) {
                ballCarrier = offensivePlayer;
                if (ballCarrier->gameState.action == RUSHING) {
                    if (ballCarrier->getPosition() == QB) {
                        messages.push_back("QB " + ballCarrier->getName() + " runs with the ball himself...");
                    } else {
                        messages.push_back(positionToStr(ballCarrier->getPosition()) + " " + ballCarrier->getName() + " takes the handoff...");
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
            messages.push_back("K " + ballCarrier->getName() + " goes out for the field goal...");
            PlayOutcome kickResult;
            if (std::rand() % 100 < odds) {
                messages.push_back("The kick is good!");
                kickResult = FIELD_GOAL_SCORED;
            } else {
                messages.push_back("The kick is no good!");
                kickResult = FIELD_GOAL_MISSED;
            }
            return {
                outcome: kickResult,
                carrier : ballCarrier,
                yards : yardLine + 17,
                specialTeamsPlay : true,
                messages : messages
            };
        }
        if (play == PUNT) {
            int yds = std::round(0.4 * ballCarrier->getRating(PUNTPOWER)) + 10;
            yds += RNG::randomNumberNormalDist(0, 3);
            if (yds > yardLine) yds = yardLine;
            messages.push_back("P " + ballCarrier->getName() + " punts the ball " + std::to_string(yds) + " yards" +
                (yardLine == 0 ? " for a touchback" : ""));
            return {
                outcome: BALL_PUNTED,
                carrier : ballCarrier,
                yards : yds,
                specialTeamsPlay : true,
                messages : messages
            };
        }

        double compBlockerRating = getCompositeRating(
            getPlayersPerformingAction(field.first, BLOCKING),
            play == RUN ? RUNBLOCK : PASSBLOCK
        );
        double compBlitzerRating = getCompositeRating(
            getPlayersPerformingAction(field.second, BLITZING),
            play == RUN ? RUNSTOP : PASSRUSH
        );
        double compReceivingRating = getCompositeRating(
            getPlayersPerformingAction(field.first, RECEIVING),
            GETTINGOPEN
        );
        double compCoverageRating = getCompositeRating(
            getPlayersPerformingAction(field.second, COVERING),
            PASSCOVER
        );
        if (play == RUN) {
            std::vector<double> runStopRatings;
            for (auto& defender : field.second) runStopRatings.push_back(defender->getRating(RUNSTOP) + 70);
            double compositeDiff = (compBlockerRating / 9.0) - (compBlitzerRating / 11.0);
            int runYards = NumberMaker::getRunYardsGained(compositeDiff);
            Player* tackledBy = runUntilTackled(ballCarrier, field.second, runStopRatings, runYards, rerollRunYards, compositeDiff, messages);
            return {
                outcome: BALL_RUSHED,
                carrier : ballCarrier,
                defender : tackledBy,
                yards : runYards,
                messages : messages
            };
        } else {
            // Pass play
            messages.push_back(ballCarrier->getPositionedName() + " drops back to pass...");
            double olineStrength = RNG::randomNumberUniformDist(-10, 10) +
                (((compBlockerRating / 5.0) - (compBlitzerRating / 4.0)) * 0.75) + 45;
            double receivingAdvantage = ((compReceivingRating / 4.0) - (compCoverageRating / 7.0)) * 0.75;
            std::vector<Player*> receivers = getPlayersPerformingAction(field.first, RECEIVING);
            std::vector<double> openness;
            for (Player* receiver : receivers) {
                openness.push_back(receivingAdvantage +
                    RNG::randomNumberNormalDist(0, 4) + (receiver->getRating(GETTINGOPEN) / 8.0));
            }
            while (true) {
                olineStrength--;
                if (olineStrength <= 0) {
                    // Attempt sack
                    messages.push_back(ballCarrier->getPositionedName() + " was sacked by someone");
                    return {
                        outcome: PASSER_SACKED,
                        carrier : ballCarrier,
                        defender : nullptr,
                        yards : -2,
                        messages : messages
                    };
                }
                for (int i = 0; i < (int)openness.size(); i++) {
                    Player* receiver = receivers[i];
                    openness[i] ++;
                    // Does the QB even see if the receiver is open tho?
                    if (openness[i] >= 50 && RNG::randomNumberUniformDist(30, 200) < ballCarrier->getRating(PASSVISION)) {
                        // Attempt pass
                        std::vector<Player*> coverers = getPlayersPerformingAction(field.second, COVERING);
                        std::vector<double> coverageRatings;
                        for (auto& coverer : coverers) coverageRatings.push_back(coverer->getRating(PASSCOVER) + 40);

                        // First check throw accuracy
                        int passYards = NumberMaker::getPassYardsGained();
                        double depthPenalty = 1.0;
                        if (passYards > 10) depthPenalty = 0.9;
                        if (passYards > 25) depthPenalty = 0.75;
                        double accuracy = depthPenalty * ((1 + (ballCarrier->getRating(PASSACCURACY) / 100.0)) / 2.0);
                        if (RNG::randomNumberUniformDist() > accuracy) {
                            // Off-target pass. Check for interception
                            double covererIndex = RNG::randomWeightedIndex(coverageRatings);
                            double intOdds = coverers[covererIndex]->getRating(CATCH);
                            if (RNG::randomNumberUniformDist(0, 400) < intOdds) {
                                messages.push_back("Intercepted by " + coverers[covererIndex]->getPositionedName() + "!");
                                return {
                                    outcome: BALL_PASSED_INCOMPLETE,
                                    defender : coverers[covererIndex],
                                    thrower : ballCarrier,
                                    incompleteReason : PASS_INCOMPLETE_INTERCEPTED,
                                    messages : messages
                                };
                            }
                            messages.push_back("Incomplete - intended for " + receiver->getPositionedName()
                                + ". The throw was off-target.");
                            return {
                                outcome: BALL_PASSED_INCOMPLETE,
                                thrower : ballCarrier,
                                incompleteReason : PASS_INCOMPLETE_MISSED,
                                messages : messages
                            };
                        }

                        // Now check for a pass deflection/int
                        double covererIndex = RNG::randomWeightedIndex(coverageRatings);
                        Player* coverer = coverers[covererIndex];
                        double intOdds = coverer->getRating(CATCH) / 8.0;
                        double deflectOdds = intOdds + coverer->getRating(PASSCOVER);
                        double x = RNG::randomNumberUniformDist(0, 350);
                        if (x < intOdds) {
                            // Interception
                            messages.push_back("Intercepted by " + coverer->getPositionedName() + "!");
                            return {
                                outcome: BALL_PASSED_INCOMPLETE,
                                defender : coverer,
                                thrower : ballCarrier,
                                incompleteReason : PASS_INCOMPLETE_INTERCEPTED,
                                messages : messages
                            };
                        } else if (x < deflectOdds) {
                            // Pass deflection
                            messages.push_back("Incomplete - intended for " + receiver->getPositionedName()
                                + ". " + coverer->getPositionedName() + " deflected the ball.");
                            return {
                                outcome: BALL_PASSED_INCOMPLETE,
                                defender : coverer,
                                thrower : ballCarrier,
                                incompleteReason : PASS_INCOMPLETE_DEFLECTED,
                                messages : messages
                            };
                        } else {
                            // Pass is on-target and undefended. Now just check if receiver can catch
                            double catchOdds = receiver->getRating(CATCH);
                            if (RNG::randomNumberUniformDist(-100, 101) > catchOdds) {
                                messages.push_back("Incomplete - intended for " + receiver->getPositionedName() +
                                    ". The receiver dropped the ball.");
                                return {
                                    outcome: BALL_PASSED_INCOMPLETE,
                                    thrower : ballCarrier,
                                    incompleteReason : PASS_INCOMPLETE_DROPPED,
                                    messages : messages
                                };
                            }

                            // Pass is caught - enter open-field tackle mode, starting with initial coverer
                            messages.push_back("Completed pass to " + receiver->getPositionedName() + "!");
                            Player* tackler = runUntilTackled(receiver, coverers, coverageRatings, passYards, rerollPassYards, 0, messages);
                            return {
                                outcome: BALL_PASSED_COMPLETE,
                                carrier : receiver,
                                yards : passYards,
                                thrower : ballCarrier,
                                messages : messages
                            };
                        }
                    }
                }
            }
        }
    }
};
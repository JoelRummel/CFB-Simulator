#pragma once

#include "school.h"


class GameManager {
    int clock;
    int quarter;
    int awayPoints;
    int homePoints;
    School* home;
    School* away;

    bool homePossession;
    bool homeGetsNextPossession;
    int yardLine;
    int down;
    int lineToGain;

    std::string scoreString() {
        return "The score is " + away->getName() + ": " + std::to_string(awayPoints) + ", " + home->getName() + ": " + std::to_string(homePoints);
    }

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

    void scoreOffensiveTD() {
        if (homePossession) homePoints += 7;
        else awayPoints += 7;
        printPlay("TOUCHDOWN! " + scoreString());
        yardLine = 25;
        lineToGain = 15;
        flipPossession();
    }

public:
    bool printPlayByPlay;

    GameManager() {
        clock = 900;
        quarter = 1;
        homePossession = false;
        homeGetsNextPossession = true;
        yardLine = 75;
        down = 1;
        lineToGain = 65;
        homePoints = 0;
        awayPoints = 0;
    }

    void setCompetingSchools(School* homeSchool, School* awaySchool) {
        home = homeSchool;
        away = awaySchool;
    }

    int getHomeScore() { return homePoints; }
    int getAwayScore() { return awayPoints; }

    bool homeHasPossession() { return homePossession; }
    int getYardLine() { return yardLine; }
    int getDown() { return down; }
    int getLineToGain() { return lineToGain; }
    int getYardsToGo() { return yardLine - lineToGain; }

    void scoreFieldGoal() {
        if (homePossession) homePoints += 3;
        else awayPoints += 3;
        printPlay(scoreString());
        yardLine = 25;
        lineToGain = 15;
        flipPossession();
    }

    void puntBall(int yards) {
        if (yards > yardLine) yards = yardLine;
        yardLine -= yards;
        if (yardLine == 0) { yardLine = 20; }
        flipPossession();
    }

    void printPlay(std::string msg) {
        if (printPlayByPlay) std::cout << clockAsStr() << msg << std::endl;

        // using namespace std::chrono_literals;  // ns, us, ms, s, h, etc.
        // using std::chrono::system_clock;
        // std::this_thread::sleep_for(500ms);
    }

    /**
     * Gives possession of the ball to whichever team didn't receive the ball at the last
     * possession reset. This also places the ball at either 75 yards or 25 yards away
     * from the goal line, depending on whether or not the game is in overtime.
     * Should be used at halftime, and at each new overtime period.
     */
    void resetPossession() {
        down = 1;
        yardLine = (quarter < 5) ? 75 : 25;
        lineToGain = yardLine - 10;
        if ((homePossession && !homeGetsNextPossession) || (!homePossession && homeGetsNextPossession)) {
            homePossession = !homePossession;
        }
        homeGetsNextPossession = !homeGetsNextPossession;
        std::string name = homePossession ? home->getName() : away->getName();
        printPlay("The " + name + " offense takes over at the " + yardLineAsStr() + ".");
    }

    /**
     * Flips possession of the ball "in place", meaning that field position isn't
     * changed. Generally used for turnovers. Can also be used after a score if
     * field position is manually changed.
     */
    void flipPossession() {
        down = 1;
        yardLine = std::abs(yardLine - 100);
        lineToGain = yardLine - 10;
        homePossession = !homePossession;
        std::string name = homePossession ? home->getName() : away->getName();
        printPlay("The " + name + " offense takes over at the " + yardLineAsStr() + ".");
    }

    /**
     * Prints the offense's current down, field position, and yards needed for a 1st down.
     */
    void printStatus() {
        std::string suffix = down == 1 ? "st" : down == 2 ? "nd" : down == 3 ? "rd" : "th";
        int toGo = yardLine - lineToGain;
        std::string secondPart = lineToGain > 0 ? std::to_string(toGo) : "Goal";
        printPlay(std::to_string(down) + suffix + " and " + secondPart + ". Ball at the " + yardLineAsStr());
    }

    /**
     * Takes 25 seconds off the clock, advancing the quarter/half as necessary.
     * Also takes care of possession change at halftime.
     */
    bool countdownClock() {
        clock -= 25;
        if (clock <= 0) {
            clock = 900;
            quarter++;
            if (quarter == 5) { return true; }
            if (quarter == 3) {
                printPlay("\n========== Halftime. ==========\n");
                resetPossession();
            }
        }
        return false;
    }

    /**
     * Advances the ball by the given number of yards, adjusting the
     * down number as necessary. Turns the ball over if the result was a failure
     * to convert on 4th down. Returns true if a touchdown is scored.
     */
    bool gainYards(int yards) {
        if (yards == 0) printPlay("No gain on the play");
        else
            printPlay(std::to_string(std::abs(yards)) + " yard " + (yards < 0 ? "loss" : "gain") + " on the play");
        yardLine -= yards;
        if (yardLine <= 0) {
            scoreOffensiveTD();
            return true;
        }
        down++;
        if (yardLine > lineToGain) {
            if (down == 5) flipPossession();
        } else {
            down = 1;
            lineToGain = yardLine - 10;
        }
        return false;
    }
};
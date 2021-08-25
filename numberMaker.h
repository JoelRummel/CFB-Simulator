#pragma once

#include "util.h"

class NumberMaker {

public:
    static int getRunYardsGained(double blockingAdvantage) {
        double x = RNG::randomNumberUniformDist();
        if (x < 0.95) {
            return std::round(6 * std::log(x + 0.1) + 8 + (blockingAdvantage / 10.0));
        } else {
            return std::round(std::pow(25, 7 * (x - 0.78)) - 37.8 + (blockingAdvantage / 10.0));
        }
    }

    static int getPassYardsGained() {
        double x = RNG::randomNumberUniformDist();
        if (x < 0.15) {
            return std::round((80 * x) - 12);
        } else if (x < 0.9) {
            return std::round(18 * std::pow(x, 2));
        } else {
            return std::round(std::pow(2, 7.71 * x) - 108.12);
        }
    }
};
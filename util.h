#pragma once

#include <algorithm>
#include <cassert>
#include <random>
#include <string>
#include <vector>

class RNG {
  public:
	static std::random_device rd;
	static std::mt19937 gen;

	static int randomWeightedIndex(const std::vector<double>& in) {
		std::discrete_distribution<> d(in.begin(), in.end());
		return d(gen);
	}

	static int randomNumberNormalDist(double mean, double stdev) {
		std::normal_distribution<> d { mean, stdev };
		return d(gen);
	}

	static int randomNumberUniformDist(int lower, int upper) {
		if (lower > upper) assert(false);
		std::uniform_int_distribution<> distrib(lower, upper);
		return distrib(gen);
	}

	template<typename T>
	static void shuffle(std::vector<T>& in) {
		std::shuffle(in.begin(), in.end(), gen);
	}
};

std::random_device RNG::rd;
std::mt19937 RNG::gen(RNG::rd());

// Stolen from SO
template<typename Iter, typename RandomGenerator>
Iter select_randomly(Iter start, Iter end, RandomGenerator& g) {
	// PROBLEM RIGHT HERE
	assert(start != end);
	std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
	std::advance(start, dis(g));
	return start;
}

// Also stolen from SO
template<typename Iter>
Iter select_randomly(Iter start, Iter end) {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	return select_randomly(start, end, gen);
}

template<typename T>
T pickNotMine(T mine, T opOne, T opTwo, T fail) {
	if (mine == opOne) return opTwo;
	else if (mine == opTwo)
		return opOne;
	else
		return fail;
}

template<typename T>
T pickMyPartner(const std::vector<std::pair<T, T>>& pairings, T me) {
	for (auto& pair : pairings) {
		if (pair.first == me) return pair.second;
		if (pair.second == me) return pair.first;
	}
	return T();
}

std::string str_upper(std::string in) {
	for (auto& c : in) c = toupper(c);
	return in;
}
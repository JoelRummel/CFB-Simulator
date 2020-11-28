#pragma once

#include <algorithm>
#include <cassert>
#include <iterator>
#include <random>
#include <sstream>
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

template<typename Out>
void split(const std::string& s, char delim, Out result) {
	std::istringstream iss(s);
	std::string item;
	while (std::getline(iss, item, delim)) { *result++ = item; }
}

std::vector<std::string> split(const std::string& s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, std::back_inserter(elems));
	return elems;
}

enum Conference {
	BIGTENEAST,
	BIGTENWEST,
	PAC12NORTH,
	PAC12SOUTH,
	BIG12,
	ACCATLANTIC,
	ACCCOASTAL,
	SECEAST,
	SECWEST,
	INDEPENDENT,

	MACEAST,
	MACWEST,
	CUSAEAST,
	CUSAWEST,
	AACEAST,
	AACWEST,
	SUNBELTEAST,
	SUNBELTWEST,
	MWCMOUNTAIN,
	MWCWEST
};

bool isP5(Conference conf) {
	switch (conf) {
	case BIGTENWEST:
	case BIGTENEAST:
	case BIG12:
	case ACCATLANTIC:
	case ACCCOASTAL:
	case PAC12NORTH:
	case PAC12SOUTH:
	case SECEAST:
	case SECWEST: return true;
	default: return false;
	}
}

std::string divisionName(Conference conf) {
	switch (conf) {
	case BIGTENEAST: return "Big Ten East";
	case BIGTENWEST: return "Big Ten West";
	case BIG12: return "Big 12";
	case ACCATLANTIC: return "ACC Atlantic";
	case ACCCOASTAL: return "ACC Coastal";
	case PAC12NORTH: return "PAC-12 North";
	case PAC12SOUTH: return "PAC-12 South";
	case SECEAST: return "SEC East";
	case SECWEST: return "SEC West";
	case MACEAST: return "MAC East";
	case MACWEST: return "MAC West";
	case AACEAST: return "AAC East";
	case AACWEST: return "AAC West";
	case SUNBELTEAST: return "Sun Belt East";
	case SUNBELTWEST: return "Sun Belt West";
	case MWCMOUNTAIN: return "MWC Mountain";
	case MWCWEST: return "MWC West";
	case CUSAEAST: return "C-USA East";
	case CUSAWEST: return "C-USA West";
	case INDEPENDENT: return "Independent";
	default: return "";
	}
}

int numNonConGames(Conference conf) {
	switch (conf) {
	case BIGTENWEST:
	case BIGTENEAST:
	case BIG12:
	case PAC12NORTH:
	case PAC12SOUTH: return 3;
	default: return 4;
	}
}

Conference getOppositeDivision(Conference conf) {
	if (conf == BIG12 || conf == INDEPENDENT) return conf;
	if (conf == BIGTENEAST) return BIGTENWEST;
	if (conf == BIGTENWEST) return BIGTENEAST;
	if (conf == SECEAST) return SECWEST;
	if (conf == SECWEST) return SECEAST;
	if (conf == PAC12NORTH) return PAC12SOUTH;
	if (conf == PAC12SOUTH) return PAC12NORTH;
	if (conf == ACCATLANTIC) return ACCCOASTAL;
	if (conf == ACCCOASTAL) return ACCATLANTIC;
	if (conf == MACEAST) return MACWEST;
	if (conf == MACWEST) return MACEAST;
	if (conf == CUSAEAST) return CUSAWEST;
	if (conf == CUSAWEST) return CUSAEAST;
	if (conf == AACEAST) return AACWEST;
	if (conf == AACWEST) return AACEAST;
	if (conf == SUNBELTEAST) return SUNBELTWEST;
	if (conf == SUNBELTWEST) return SUNBELTEAST;
	if (conf == MWCMOUNTAIN) return MWCWEST;
	if (conf == MWCWEST) return MWCMOUNTAIN;
	assert(false);
}
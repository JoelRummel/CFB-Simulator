#pragma once

#include "util.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <unordered_map>

struct State {
	std::string name;
	std::string acro;
	int signees;
};

struct City {
	std::string name;
	std::string state;
	double latitude;
	double longitude;
	int population;

	std::string formalName() {
		return name + ", " + state;
	}

	static double distance(City* city1, City* city2) {
		return distanceEarth(city1->latitude, city1->longitude, city2->latitude, city2->longitude);
	}
};

struct SchoolData {
	std::string name;
	std::string mascot;
	Conference division;
	std::string state;
	std::string city;
	int prestige;
	int stadiumCapacity;
	int budget;
	int nflRating;
	int academicRating;
};

class GlobalData {
private:
	static std::vector<std::string> getCsvLine(std::fstream& in) {
		std::string line;
		std::getline(in, line); // clear header row
		return split(line, ',');
	}

	struct NameData {
		NameData() {}
		int totalFrequency = 0;
		std::vector<std::pair<int, std::string>> names;
		void readInData(std::string filename) {
			std::fstream in(filename);
			std::string line;
			while (in.good()) {
				std::getline(in, line);
				std::istringstream iss(line);
				std::string name = "";
				std::string next;
				iss >> next;
				while (next != "|") {
					name += next + " ";
					iss >> next;
				}
				name.pop_back();
				int freq;
				iss >> freq;
				names.push_back(std::make_pair(freq, name));
				totalFrequency += freq;
			}
		}
		std::string getRandomName() {
			int pointer = -1;
			for (int index = std::rand() % totalFrequency; index > 0; index) {
				pointer++;
				index -= names[pointer].first;
			}
			if (pointer == -1) pointer = 0;
			return names[pointer].second;
		}
	};

	struct CoachNames {
		std::vector<std::pair<std::string, std::string>> names;
		void readInData(std::string fileName) {
			std::fstream in(fileName);
			while (in.good()) {
				std::string firstName, lastName;
				while (true) {
					char c;
					in >> c;
					if (c == ',') break;
					lastName += c;
				}
				std::getline(in, firstName);
				firstName.erase(firstName.begin());
				names.push_back(std::make_pair(firstName, lastName));
			}
		}

		std::string getRandomName(bool last = false) {
			if (last) return names[std::rand() % names.size()].second;
			else
				return names[std::rand() % names.size()].first;
		}
	};

	template <typename T>
	class DataParser {
	public:
		std::vector<T> data;

		virtual T parseLine(std::vector<std::string> row) = 0;

		void readInData(std::string filename) {
			std::fstream in(filename);
			std::string line;
			std::getline(in, line); // clear header row
			while (in.good()) {
				T datum = parseLine(getCsvLine(in));
				data.push_back(datum);
			}
		}
	};

	class StateData : public DataParser<State> {
		int totalSignees;

		State parseLine(std::vector<std::string> row) {
			State state;
			state.name = row[0];
			state.acro = row[1];
			state.signees = std::stoi(row[2]);
			return state;
		}

	public:
		StateData() { totalSignees = 0; }

		State* pickRandomState() {
			if (totalSignees == 0) {
				for (State& state : data) {
					totalSignees += state.signees;
				}
			}
			int pointer = -1;
			for (int index = std::rand() % totalSignees; index > 0; index) {
				pointer++;
				index -= data[pointer].signees;
			}
			if (pointer == -1) pointer = 0;
			return &(data[pointer]);
		}

		std::string stateNameToCode(std::string name) {
			for (auto& state : data) {
				if (state.name == name) return state.acro;
			}
			throw "Invalid state name: " + name;
		}
	};

	class CityData : public DataParser<City> {
		std::unordered_map<std::string, int> totalStatePop;
		City parseLine(std::vector<std::string> row) {
			City city;
			city.name = row[0];
			city.state = row[1];
			city.latitude = std::stod(row[2]);
			city.longitude = std::stod(row[3]);
			city.population = std::stoi(row[4]);
			return city;
		}

	public:
		City* pickRandomCity(std::string state) {
			if (totalStatePop.find(state) == totalStatePop.end()) {
				totalStatePop[state] = 0;
				for (City& city : data) {
					if (city.state == state) totalStatePop[state] += city.population;
				}
			}
			int pointer = -1;
			for (int index = std::rand() % totalStatePop[state]; index > 0; index) {
				pointer++;
				if (data[pointer].state != state) continue;
				index -= data[pointer].population;
			}
			if (pointer == -1) pointer = 0;
			return &(data[pointer]);
		}

		City* getCityByName(std::string stateCode, std::string name) {
			for (auto& city : data) {
				if (city.state == stateCode && city.name == name)
					return &city;
			}
			return nullptr;
		}
	};

	inline static NameData firstNames;
	inline static NameData lastNames;
	inline static CoachNames coachNames;
	inline static StateData stateData;
	inline static CityData cityData;

	class SchoolsData : public DataParser<SchoolData> {
		SchoolData parseLine(std::vector<std::string> row) {
			SchoolData sd{
				row[0], row[1], BIG12, row[3], row[4], std::stoi(row[5]), std::stoi(row[6]), std::stoi(row[7]), std::stoi(row[8]),
				std::stoi(row[9])
			};
			if (row[2] == "Big Ten East") sd.division = BIGTENEAST;
			else if (row[2] == "Big Ten West")
				sd.division = BIGTENWEST;
			else if (row[2] == "SEC East")
				sd.division = SECEAST;
			else if (row[2] == "SEC West")
				sd.division = SECWEST;
			else if (row[2] == "Pac-12 North")
				sd.division = PAC12NORTH;
			else if (row[2] == "Pac-12 South")
				sd.division = PAC12SOUTH;
			else if (row[2] == "ACC Coastal")
				sd.division = ACCCOASTAL;
			else if (row[2] == "ACC Atlantic")
				sd.division = ACCATLANTIC;
			else if (row[2] == "MAC West")
				sd.division = MACWEST;
			else if (row[2] == "MAC East")
				sd.division = MACEAST;
			else if (row[2] == "C-USA East")
				sd.division = CUSAEAST;
			else if (row[2] == "C-USA West")
				sd.division = CUSAWEST;
			else if (row[2] == "AAC East")
				sd.division = AACEAST;
			else if (row[2] == "AAC West")
				sd.division = AACWEST;
			else if (row[2] == "Sun Belt East")
				sd.division = SUNBELTEAST;
			else if (row[2] == "Sun Belt West")
				sd.division = SUNBELTWEST;
			else if (row[2] == "MWC Mountain")
				sd.division = MWCMOUNTAIN;
			else if (row[2] == "MWC West")
				sd.division = MWCWEST;
			else if (row[2] == "Independent")
				sd.division = INDEPENDENT;
			return sd;
		}
	};
	inline static SchoolsData schoolsData;

public:
	static void loadEverything() {
		firstNames.readInData("data/firstNames.txt");
		lastNames.readInData("data/lastNames.txt");
		coachNames.readInData("data/coachNames.txt");
		schoolsData.readInData("data/schools.csv");
		stateData.readInData("data/states.csv");
		cityData.readInData("data/cities.csv");
	}

	static std::string getRandomName() { return firstNames.getRandomName() + " " + lastNames.getRandomName(); }
	static std::string getRandomCoachName() { return coachNames.getRandomName() + " " + coachNames.getRandomName(true); }
	static std::vector<SchoolData>& getSchoolsData() { return schoolsData.data; }
	static City* getRandomCity() {
		State* state = stateData.pickRandomState();
		return cityData.pickRandomCity(state->acro);
	}
	static std::string stateNameToCode(std::string name) { return stateData.stateNameToCode(name); }
	static City* getCityByName(std::string stateCode, std::string name) { return cityData.getCityByName(stateCode, name); }
};
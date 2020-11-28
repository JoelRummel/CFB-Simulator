#pragma once

#include "school.h"
#include "util.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

class GlobalData {
  private:
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
					if (c == ',') {
						in >> c;
						break;
					}
					lastName += c;
				}
				std::getline(in, firstName);
				names.push_back(std::make_pair(firstName, lastName));
			}
		}

		std::string getRandomName(bool last = false) {
			if (last) return names[std::rand() % names.size()].second;
			else
				return names[std::rand() % names.size()].first;
		}
	};

	inline static NameData firstNames;
	inline static NameData lastNames;
	inline static CoachNames coachNames;

  public:
	struct SchoolsData {
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
		std::vector<SchoolData> schoolData;
		void readInData(std::string fileName) {
			std::fstream in(fileName);
			std::string line;
			std::getline(in, line); // clear header row
			while (in.good()) {
				std::getline(in, line);
				std::vector<std::string> row = split(line, ',');
				SchoolData sd {
					row[0],           row[1], BIG12, row[3], row[4], std::stoi(row[5]), std::stoi(row[6]), std::stoi(row[7]), std::stoi(row[8]),
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
				schoolData.push_back(sd);
			}
			in.close();
		}
	};
	inline static SchoolsData schoolsData;

	static void loadEverything() {
		firstNames.readInData("data/firstNames.txt");
		lastNames.readInData("data/lastNames.txt");
		coachNames.readInData("data/coachNames.txt");
		schoolsData.readInData("data/schools.csv");
	}

	static std::string getRandomName() { return firstNames.getRandomName() + " " + lastNames.getRandomName(); }
	static std::string getRandomCoachName() { return coachNames.getRandomName() + " " + coachNames.getRandomName(); }
	static std::vector<SchoolsData::SchoolData> getSchoolsData() { return schoolsData.schoolData; }
};
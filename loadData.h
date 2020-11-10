#pragma once

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

	inline static NameData firstNames;
	inline static NameData lastNames;

  public:
	static void loadEverything() {
		firstNames.readInData("firstNames.txt");
		lastNames.readInData("lastNames.txt");
	}

	static std::string getRandomName() { return firstNames.getRandomName() + " " + lastNames.getRandomName(); }
};
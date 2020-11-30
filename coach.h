#pragma once

#include "loadData.h"
#include "util.h"

enum class CoachType { QB, WR, RB, OL, DL, LB, DB, ST, OC, DC, HC };

std::string coachTypeToString(CoachType t) {
	switch (t) {
	case CoachType::QB: return "Quarterback Coach";
	case CoachType::WR: return "Receiver Coach";
	case CoachType::RB: return "Running Back Coach";
	case CoachType::OL: return "Offensive Line Coach";
	case CoachType::DL: return "Defensive Line Coach";
	case CoachType::LB: return "Linebacker Coach";
	case CoachType::DB: return "Defensive Back Coach";
	case CoachType::ST: return "Special Teams Coordinator";
	case CoachType::OC: return "Offensive Coordinator";
	case CoachType::DC: return "Defensive Coordinator";
	case CoachType::HC: return "Head Coach";
	}
	return "ERRRRRRRRROR";
}

class School;
struct Vacancy {
	School* school = nullptr;
	std::string schoolName; // for alma mater checking
	CoachType type;
	int salary;
};

struct CoachingHistory {
	Vacancy job;
	int yearStart;
	int yearEnd;
	bool fired;
};

class Coach {
	std::string name;
	int age;
	std::string almaMater;
	CoachType primaryType;
	Vacancy currentJob;
	std::vector<CoachingHistory> history;
	int yearsInCurrentJob;
	double priorityMoney;     // 0-1
	double priorityStability; // 0-1
	double priorityAlumni;    // 0-1
	double priorityNFL;       // 0-1, independent of the other three metrics

	double ovrPublic; // 0-100

	int ovrDevelopment; // 0-100
	int ovrGametime;    // 0-100
	int ovrRecruiting;  // 0-100
	int ovrCulture;     // 0-100

  public:
	// Constructs the coach based on a prestige factor from 0-10
	Coach(int prestige) {
		// TODO: pick a random alma mater from the GlobalData list of schools
		name = GlobalData::getRandomCoachName();
		ovrDevelopment = 40 + (prestige * 5) + RNG::randomNumberNormalDist(0, 7);
		ovrGametime = 40 + (prestige * 5) + RNG::randomNumberNormalDist(0, 7);
		ovrRecruiting = 40 + (prestige * 5) + RNG::randomNumberNormalDist(0, 7);
		ovrCulture = 0;
		ovrPublic = std::round((ovrDevelopment + ovrGametime + ovrRecruiting) / 3.0);
		primaryType = (CoachType)(std::rand() % 8);

		priorityAlumni = (std::rand() % 100) / 100.0;
		priorityMoney = (std::rand() % 100) / 100.0;
		priorityStability = (std::rand() % 100) / 100.0;
		priorityNFL = (std::rand() % 100) / 100.0;
		// Normalize the first three metrics
		double sum = priorityAlumni + priorityMoney + priorityStability;
		priorityAlumni /= sum;
		priorityMoney /= sum;
		priorityStability /= sum;
	}
	Coach(int prestige, CoachType type) : Coach(prestige) { primaryType = type; }

	std::string getName() { return name; }
	double getPublicOvr() { return ovrPublic; }

	double getPreferenceLevel(Vacancy v) {
		double b = v.salary / 1000000;
		if (v.school == currentJob.school) b += (b * priorityStability) / (yearsInCurrentJob + 1);
		if (v.schoolName == almaMater) b += (b * priorityAlumni);
		return b;
	}

	int pickJob(std::vector<Vacancy>& vacancies) {
		double highestPreference = isEmployed() ? getPreferenceLevel(currentJob) : -100000;
		int bestJob = -1;
		for (int i = 0; i < (int)vacancies.size(); i++) {
			if (vacancies[i].type != CoachType::HC) {
				if (vacancies[i].type == CoachType::OC) {
					if (primaryType != CoachType::QB && primaryType != CoachType::RB && primaryType != CoachType::OL && primaryType != CoachType::WR)
						continue;
				} else if (vacancies[i].type == CoachType::DC) {
					if (primaryType != CoachType::DL && primaryType != CoachType::DB && primaryType != CoachType::LB) continue;
				} else if (vacancies[i].type != primaryType)
					continue;
			}
			double p = getPreferenceLevel(vacancies[i]);
			if (p > highestPreference) {
				bestJob = i;
				highestPreference = p;
			}
		}
		if (bestJob != -1) {
			resign();
			currentJob = vacancies[bestJob];
		}
		return bestJob;
	}

	void takeJob(Vacancy v) {
		assert(!isEmployed());
		currentJob = v;
	}

	bool isEmployed() { return (currentJob.school != nullptr); }

	void resign(bool fired = false) {
		int yearStart, yearEnd;
		if (history.size() == 0) yearStart = 2020; // TODO: this breaks if the coach was created later
		else
			yearStart = history.back().yearEnd + 1;
		yearEnd = yearStart + yearsInCurrentJob;
		CoachingHistory h { currentJob, yearStart, yearEnd };
		history.push_back(h);
		yearsInCurrentJob = 0;
		currentJob.school = nullptr;
	}

	void incrementYear() {
		yearsInCurrentJob++;
		age++;
	}
};
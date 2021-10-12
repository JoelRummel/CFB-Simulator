#pragma once

#include "../loadData.h"
#include "../players/player.h"
#include "../util.h"

enum class CoachType { QB, WR, RB, OL, DL, LB, DB, ST, OC, DC, HC, UN };

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
	case CoachType::UN: return "Unemployed";
	}
	return "ERRRRRRRRROR";
}

CoachType getSecondLevelCoachType(Position p) {
	switch (p) {
	case QB:
	case HB:
	case WR:
	case TE:
	case OL: return CoachType::OC;
	case DL:
	case LB:
	case CB:
	case S: return CoachType::DC;
	default: return CoachType::ST;
	}
}

CoachType getPositionalCoachType(Position p) {
	switch (p) {
	case QB: return CoachType::QB;
	case HB: return CoachType::RB;
	case WR:
	case TE: return CoachType::WR;
	case OL: return CoachType::OL;
	case DL: return CoachType::DL;
	case LB: return CoachType::LB;
	case CB:
	case S: return CoachType::DB;
	default: return CoachType::ST;
	}
}

class School;
struct Vacancy {
	School* school = nullptr;
	std::string schoolName; // for alma mater checking
	CoachType type;
	int salary;
	int prestige;
};
struct Contract {
	int yearsTotal;
	int yearsRemaining;
	int originalPrestige;
	std::vector<int> prestigeTargets; // These are the minimum prestiges per each extension
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
	double priorityPrestige;  // 0-1
	double priorityNFL;       // 0-1, independent of the other three metrics
	double tempMoney;
	double tempStability;
	double tempAlumni;
	double tempPrestige;

	int ovrPublic; // 0-100

	int ovrDevelopment; // 0-100
	int ovrGametime;    // 0-100
	int ovrRecruiting;  // 0-100
	int ovrCulture;     // 0-100

	void normalizePriorities() {
		double sum = priorityAlumni + priorityMoney + priorityStability + priorityPrestige;
		priorityAlumni /= sum;
		priorityMoney /= sum;
		priorityStability /= sum;
		priorityPrestige /= sum;
	}

public:
	Contract currentContract;
	// Constructs the coach randomly, bumping up the public OVR randomly if the simulator is getting initialized
	Coach(bool initial = false) {
		// TODO: pick a random alma mater from the GlobalData list of schools
		name = GlobalData::getRandomCoachName();

		ovrDevelopment = 40;
		ovrGametime = 40;
		ovrRecruiting = 40;
		int* rats[3] = { &ovrDevelopment, &ovrGametime, &ovrRecruiting };
		ovrPublic = 40;

		double rand = RNG::randomNumberUniformDist();
		int ovr = std::max(std::min(std::floor(60 * std::pow(rand, 4)), 59.0), std::round(15 * rand)); // The plus 40 is implied
		for (int i = ovr * 3; i > 0; i) {
			int chunk = RNG::randomNumberUniformDist(1, 3);
			if (chunk > i) chunk = i;
			int bucket = RNG::randomNumberUniformDist(0, 2);
			for (int j = 0; j < 3; j++) {
				if (*(rats[(bucket + j) % 3]) + chunk <= 99) {
					*(rats[(bucket + j) % 3]) += chunk;
					break;
				}
				if (j == 2) {
					chunk--;
					j = 0;
					assert(chunk > 0);
				}
			}
			i -= chunk;
		}

		if (initial) {
			// Bring the public OVR closer to the actual OVR, with a chance to overrate the coach
			double closerFactor = (std::rand() % 115) / 100.0;
			ovrPublic += std::round((((ovrDevelopment + ovrGametime + ovrRecruiting) / 3.0) - 40) * closerFactor);
			ovrPublic = std::min(ovrPublic, 99);
		}
		primaryType = (CoachType)(std::rand() % 8);

		priorityAlumni = (std::rand() % 100) / 100.0;
		priorityMoney = (std::rand() % 100) / 100.0;
		priorityStability = (std::rand() % 100) / 100.0;
		priorityPrestige = (std::rand() % 100) / 100.0;
		priorityNFL = (std::rand() % 100) / 100.0;
		tempAlumni = priorityAlumni;
		tempMoney = priorityMoney;
		tempStability = priorityStability;
		tempPrestige = priorityPrestige;
		if (initial) {
			// Make prestige a bigger deal for initial coaches in order to better reflect the current state of the world
			priorityPrestige += (1 - priorityPrestige) * 0.9;
		}
		// Normalize the first four metrics
		normalizePriorities();

		currentJob.type = CoachType::UN;
	}
	Coach(bool isInitial, CoachType type) : Coach(isInitial) { primaryType = type; }

	std::string getName() { return name; }
	int getPublicOvr() { return ovrPublic; }
	int getActualOvr() { return std::round((ovrDevelopment + ovrGametime + ovrRecruiting) / 3.0); }
	int getOvrDevelopment() { return ovrDevelopment; }
	int getOvrRecruiting() { return ovrRecruiting; }
	int getOvrGametime() { return ovrGametime; }
	std::string getTypeString() { return coachTypeToString(currentJob.type); }
	CoachType getJobType() { return currentJob.type; }
	School* getEmployer() { return currentJob.school; }

	// Assessment: 0-1 inclusive. This function moves the public OVR needle towards the assessment percentage.
	void givePublicAssessment(double assessment) {
		double rating = 40;
		// Pick different assessment curves depending on job level
		int m = 40;
		int b = 0;
		if (currentJob.type == CoachType::HC) {
			m = 50;
			b = 10;
		} else if (currentJob.type == CoachType::OC || currentJob.type == CoachType::DC || currentJob.type == CoachType::ST) {
			m = 45;
			b = 5;
		}
		rating += std::floor((m * assessment) + b);

		double diff = rating - ovrPublic;
		double acc = 0.5;
		ovrPublic += std::round(acc * diff);
		ovrPublic = std::min(ovrPublic, 99);
		ovrPublic = std::max(ovrPublic, 40);
	}

	double getPreferenceLevel(Vacancy v) {
		double b = v.salary / 1000000; // expected to be somewhere from roughly 6-50 (more often 10-30) for HC jobs, 1.3-4.2 for positional jobs
		b += (b * priorityPrestige * (v.prestige / 10));
		if (v.school == currentJob.school) b += (b * priorityStability) / (yearsInCurrentJob + 1);
		if (v.schoolName == almaMater) b += (b * priorityAlumni);
		return b;
	}

	// Todo: eliminate this function, it is redundant and worthless now
	int pickJob(std::vector<Vacancy>& vacancies) {
		int favorite = pickFavoriteJob(vacancies);
		return favorite;
	}

	int pickFavoriteJob(std::vector<Vacancy>& vacancies) {
		if (isEmployed() && yearsInCurrentJob == 0) return -1;
		double highestPreference = isEmployed() ? getPreferenceLevel(currentJob) : -1000000;
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
		return bestJob;
	}

	void takeJob(Vacancy v) {
		assert(!isEmployed());
		currentJob = v;
		yearsInCurrentJob = 0;
	}

	bool isEmployed() { return (currentJob.school != nullptr); }

	bool resign(bool fired = false) {
		bool wasEmployed = isEmployed();
		int yearStart, yearEnd;
		if (history.size() == 0) yearStart = 2020; // TODO: this breaks if the coach was created later
		else
			yearStart = history.back().yearEnd + 1;
		yearEnd = yearStart + yearsInCurrentJob;
		CoachingHistory h{ currentJob, yearStart, yearEnd, fired };
		history.push_back(h);
		yearsInCurrentJob = 0;
		currentJob.school = nullptr;
		currentJob.type = CoachType::UN;
		return wasEmployed;
	}

	void incrementYear() {
		if (tempMoney != -1) {
			priorityMoney = tempMoney;
			priorityAlumni = tempAlumni;
			priorityPrestige = tempPrestige;
			priorityStability = tempStability;
			normalizePriorities();
		}
		if (isEmployed()) yearsInCurrentJob++;
		age++;
	}

	std::vector<CoachingHistory> getHistory() { return history; }
};

std::ostream& operator<<(std::ostream& in, Coach* c) {
	printf("%-27s%-20s%d/%d public OVR ( %d DVLP | %d RCRT | %d GAME ) Year %d of %d", c->getTypeString().c_str(), c->getName().c_str(),
		c->getPublicOvr(), c->getActualOvr(), c->getOvrDevelopment(), c->getOvrRecruiting(), c->getOvrGametime(),
		c->currentContract.yearsTotal - c->currentContract.yearsRemaining + 1, c->currentContract.yearsTotal);
	return in;
}
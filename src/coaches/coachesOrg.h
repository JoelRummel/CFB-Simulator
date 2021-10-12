#pragma once

#include "coach.h"
#include "../school.h"

struct SortByPublicOvr {
	bool operator()(Coach* a, Coach* b) { return (a->getPublicOvr() > b->getPublicOvr()); }
};

class CoachesOrganization {
	std::vector<Coach*> coaches; // the realm of all coaches

public:
	Coach* generateCoach(bool isInitial) {
		Coach* c = new Coach(isInitial);
		coaches.push_back(c);
		return c;
	}

	Coach* generateCoach(CoachType type) {
		Coach* c = new Coach(false, type);
		coaches.push_back(c);
		return c;
	}

	void initializeAllCoaches() {
		coaches.reserve(1600);
		for (int i = 0; i < 1600; i++) generateCoach(true);
	}

	Vacancy createVacancy(School* school, CoachType type) {
		int budget = school->getBudget();
		if (type != CoachType::HC) {
			if (type == CoachType::DC || type == CoachType::OC) budget /= 3.5;
			else if (type == CoachType::ST)
				budget /= 5;
			else
				budget /= 7;
		}
		Vacancy v{ school, school->getName(), type, budget, school->getPrestige() };
		return v;
	}

	std::vector<Vacancy> getAllVacancies(std::vector<School*> allSchools) {
		std::vector<Vacancy> vacancies;
		for (School* school : allSchools) {
			for (int i = 0; i < 11; i++) {
				CoachType type = (CoachType)i;
				if (school->isVacant(type)) { vacancies.push_back(createVacancy(school, type)); }
			}
		}
		return vacancies;
	}

	void fillAllVacancies(std::vector<School*> allSchools) {
		std::vector<Vacancy> vacancies = getAllVacancies(allSchools);
		SortByPublicOvr sbpo;
		std::sort(coaches.begin(), coaches.end(), sbpo);
		for (int i = 0; i < (int)coaches.size(); i++) {
			Coach* coach = coaches[i];
			int jobTaken = coach->pickJob(vacancies);
			if (jobTaken == -1) continue;
			// The school now has a chance to snipe someone better
			std::vector<Coach*> snipeSet(coaches.begin() + i, (coaches.size() > i + 30) ? (coaches.begin() + i + 30) : (coaches.end()));
			Coach* sniped = vacancies[jobTaken].school->snipeCoach(snipeSet, vacancies, jobTaken);
			if (sniped != coach) i--;
			coach = sniped;

			if (coach->isEmployed()) {
				// lmao they took a better gig
				Vacancy v = createVacancy(coach->getEmployer(), coach->getJobType());
				vacancies.push_back(v);
				coach->getEmployer()->loseCoach(coach->getJobType());
				coach->resign();
			}
			coach->takeJob(vacancies[jobTaken]);
			vacancies[jobTaken].school->signCoach(coach, vacancies[jobTaken].type);
			vacancies.erase(vacancies.begin() + jobTaken);
			if (vacancies.empty()) break;
		}
		for (Vacancy v : vacancies) {
			// No coaches wanted these jobs. Create bare minimum coaches
			Coach* walkon = generateCoach(v.type);
			walkon->takeJob(createVacancy(v.school, v.type));
			v.school->signCoach(walkon, v.type);
		}
	}

	void printCoachesByJobType(CoachType t) {
		SortByPublicOvr sbpo;
		std::sort(coaches.begin(), coaches.end(), sbpo);
		for (Coach* coach : coaches) {
			if (coach->getJobType() == t) {
				if (t != CoachType::UN) printf("%-20s", coach->getEmployer()->getName().c_str());
				std::cout << coach << "\n";
			}
		}
	}

	void printCoachHistoryByName(std::string coachName) {
		for (Coach* coach : coaches) {
			if (coach->getName() == coachName) {
				std::cout << coachName << " (" << coach->getPublicOvr() << "/" << coach->getActualOvr() << " OVR)\n";
				std::cout << "Current job: " << coach->getTypeString();
				if (coach->isEmployed()) std::cout << " at " << coach->getEmployer()->getName();
				std::cout << "\n\n========== HISTORY:\n";
				for (CoachingHistory history : coach->getHistory()) {
					std::cout << history.yearStart << " - " << history.yearEnd << ": " << coachTypeToString(history.job.type) << " at "
						<< history.job.schoolName;
					if (history.fired) std::cout << " (fired)";
					std::cout << "\n";
				}
				return;
			}
		}
	}

	void advanceYear() {
		for (Coach* coach : coaches) coach->incrementYear();
	}

	~CoachesOrganization() {
		for (auto c : coaches) delete c;
	}
};
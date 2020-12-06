#pragma once

#include "coach.h"
#include "school.h"

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
		for (int i = 0; i < 1500; i++) generateCoach(true);
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
		Vacancy v { school, school->getName(), type, budget, school->getPrestige() };
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
		for (Coach* coach : coaches) {
			int jobTaken = coach->pickJob(vacancies);
			if (jobTaken == -1) continue;
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

	void advanceYear() {
		for (Coach* coach : coaches) coach->incrementYear();
	}

	~CoachesOrganization() {
		for (auto c : coaches) delete c;
	}
};
#pragma once
#include "coach.h"

// Should be a member of each School

enum class HiringAction { HIRED, FIRED, EXTENDED, DEPARTED };

struct HiringHistory {
	int year;
	Coach* coach;
	CoachType role;
	HiringAction action;
	int contractLength;

	void print() {
		std::cout << year << ": " << coach->getName() << " was ";
		switch (action) {
		case HiringAction::HIRED: std::cout << "hired as " << coachTypeToString(role) << " on a " << contractLength << " year contract"; break;
		case HiringAction::FIRED: std::cout << "fired as " << coachTypeToString(role); break;
		case HiringAction::EXTENDED: std::cout << "extended for " << contractLength << " years as " << coachTypeToString(role); break;
		case HiringAction::DEPARTED: std::cout << "hired away, opening up the " << coachTypeToString(role) << " position"; break;
		}
		std::cout << "\n";
	}
};

class CoachingLogs {
	int year = 2020;
	std::vector<std::vector<HiringHistory>> incomingHistory;
	std::vector<std::vector<HiringHistory>> outgoingHistory;

public:
	CoachingLogs() {
		incomingHistory.emplace_back();
		outgoingHistory.emplace_back();
	}

	void advanceYear() {
		year++;
		incomingHistory.emplace_back();
		outgoingHistory.emplace_back();
	}

	void recordHire(Coach* coach, CoachType role, int contractLength) {
		HiringHistory h{ year, coach, role, HiringAction::HIRED, contractLength };
		incomingHistory.back().push_back(h);
	}

	void recordExtension(Coach* coach, CoachType role, int contractLength) {
		HiringHistory h{ year, coach, role, HiringAction::EXTENDED, contractLength };
		incomingHistory.back().push_back(h);
	}

	void recordFire(Coach* coach, CoachType role) {
		HiringHistory h{ year, coach, role, HiringAction::FIRED, -1 };
		outgoingHistory.back().push_back(h);
	}

	void recordLoss(Coach* coach, CoachType role) {
		HiringHistory h{ year, coach, role, HiringAction::DEPARTED, -1 };
		outgoingHistory.back().push_back(h);
	}

	void printHiringFiringRecords() {
		std::cout << std::endl;
		for (int i = 0; i < (int)incomingHistory.size(); i++) {
			for (HiringHistory& item : outgoingHistory[i]) item.print();
			for (HiringHistory& item : incomingHistory[i]) item.print();
		}
	}
};
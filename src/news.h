#pragma once

#include "school.h"

#include <queue>

enum class Topic {
	// Lead-up to game
	RIVALRY_MATCHUP,   // Rival game scheduled
	PLAYOFFS_SET,      // Playoff semifinals have been set
	FINALS_SET,        // Playoff finals have been set
	BIG_MATCHUP,       // Matchup between two highly ranked teams
	CONFERENCE_OPENER, // Big team opening conference play
	SEASON_OPENER,     // Big team opening season play
	RANKED_G5_MATCHUP, // Matchup involving highly ranked G5 team
	STANDARD_MATCHUP,  // Matchup involving highly ranked team

	// Game result
	NATIONAL_CHAMPIONS,   // National champs!
	CONFERENCE_CHAMPIONS, // Conference champs!
	BIG_UPSET,            // Highly ranked team lost to much lower ranked team
	BEATDOWN,             // Hugely lopsided game
	NARROW_WIN,           // Win by tiny margin between very differently ranked teams
	OFFENSIVE_THRILLER,   // Many points scored in this game
	DEFENSIVE_SLUGFEST,   // Few points scored in this game
	OVERTIME_THRILLER,    // Game went to overtime. Bonus for extra OTs
	DEFEND_STANDING,      // Highly ranked team holds off challenger

	// Big picture season standing
	STRONG_SEASON,      // Team's season going better than expected
	POOR_SEASON,        // Team's season going worse than expected
	SEASON_AS_EXPECTED, // High prestige team is doing well
	STRONG_OFFENSE,     // Lower prestige team is highly ranked thanks to offense
	STRONG_DEFENSE,     // Same as above, but defense instead
	STAR_PLAYER,        // Player on a team is racking up crazy stats
	POOR_OFFENSE,       // Higher prestige team has offensive woes
	POOR_DEFENSE,       // Higher prestige team has defensive woes
	BAD_PLAYER,         // Skill position starter is doing poorly, ie low TD-INT ratio

};

class News {
private:
	struct Story {
		Topic topic;
		std::vector<School*> schools;  // Usually either one or two schools depicting the story
		std::vector<Player*> players;  // Usually a single player relevant to the story
		std::vector<std::string> info; // Other misc info, most often scorelines
		double priority;
		std::pair<std::string, std::string> headline;

		Story(Topic t, double prio, std::vector<School*> s = {}, std::vector<Player*> p = {}, std::vector<std::string> i = {}) {
			topic = t;
			priority = prio;
			schools = s;
			players = p;
			info = i;

			std::vector<std::string> headerOptions;
			std::vector<std::string> bodyOptions;

			std::vector<std::string> sn(s.size());
			for (int i = 0; i < (int)s.size(); i++) sn[i] = s[i]->getRankedName();
			switch (t) {
			case Topic::RIVALRY_MATCHUP:
				priority = 25;
				if (s[0]->getName() == "Ohio State" || s[0]->getName() == "Michigan")
					headerOptions = { "Hate week is here", "'The Game' on the horizon", "Wolverines and Buckeyes clash again" };
				if (s[0]->getName() == "Auburn" || s[0]->getName() == "Alabama")
					headerOptions = { "Auburn and Alabama look to settle the score", "Foy-ODK trophy up for grabs",
									  "'Iron Bowl' competitors seek bragging rights" };
				break;
			case Topic::PLAYOFFS_SET:
				priority = 25;
				headerOptions = { "Playoffs bracket released", "Playoff Committee decisions made", "Top four teams decided" };
				bodyOptions = { sn[0] + ", " + sn[1] + ", " + sn[2] + ", and " + sn[3] + " have received bids for the national title",
								sn[0] + " plays " + sn[3] + ", while " + sn[1] + " faces off against " + sn[2] };
				break;
			case Topic::FINALS_SET:
				priority = 25;
				headerOptions = { "Championship game set", "Playoff finals are here", "Playoff finalists face off this week", "Championship opponents decided" };
				bodyOptions = { sn[0] + " and " + sn[1] + " survive their semifinal opponents and look to clinch the national championship",
								sn[0] + " and " + sn[1] + " are headed to a duel for the national title" };
				break;
			case Topic::BIG_MATCHUP:

				priority = 15;
				headerOptions = { "Two titans clash", "Highly anticipated showdown awaits", "Hype builds for big game", "Bragging rights await", "Big matchup this weekend" };
				bodyOptions = { "Regular season game between " + sn[0] + " and " + sn[1] + " looks sure to excite many fans",
								"Playoff seeding is on the line as " + sn[0] + " and " + sn[1] + " clash this weekend",
								"Two confident teams in " + sn[0] + " and " + sn[1] + " head for a collision, look to hang on to playoff hopes" };
				break;

			case Topic::CONFERENCE_OPENER:
				priority = 4;
				headerOptions = { sn[0] + " begins conference play", sn[0] + " to make conference debut", "Start of conference play for " + sn[0] };
				bodyOptions = { sn[0] + " opens conference play versus " + sn[1] + " this weekend",
								sn[0] + " to begin campaign for conference title against " + sn[1],
								sn[0] + " faces first conference test of the season against " + sn[1] };
				break;

			case Topic::SEASON_OPENER:
				priority = 3;
				headerOptions = { "Excitement builds for " + sn[0], sn[0] + " begins a hopeful season", sn[0] + " expectations soar", "Teams prepare for season opener" };
				bodyOptions = { sn[0] + " looking ready to open the season versus " + sn[1],
								sn[0] + " kicks off the season against " + sn[1] + " as fans hope for big results",
								sn[0] + " looks to open the season with a win as they prepare to play " + sn[1] };

			case Topic::RANKED_G5_MATCHUP:
				priority = 14;
				headerOptions = { sn[0] + " seeks respect", "Big test coming for " + sn[0], sn[0] + " faces biggest challenge yet" };
				bodyOptions = { sn[0] + " goes to the national spotlight as they prepare to play " + sn[1],
								sn[0] + " has much to prove against a tough opponent in " + sn[1],
								sn[0] + " hopes to elevate themselves as they face a tough " + sn[1] + " team" };
				break;

			case Topic::STANDARD_MATCHUP:
				priority = 2;
				headerOptions = { sn[1] + " dreams of an upset", sn[0] + " feeling confident", sn[1] + " takes aim at " + sn[0], sn[1] + " hoping to cause problems vs. " + sn[0] };
				bodyOptions = { sn[1] + " has nothing to lose and everything to gain as they prepare to face " + sn[0],
								"It's a long shot for the underdog " + sn[1] + " versus a powerful " + sn[0] + " team",
								sn[0] + " is the heavy favorite for their matchup against " + sn[1] + ", but anything is possible",
								sn[0] + " hopes to breeze past " + sn[1] + " with little drama" };
				break;

			case Topic::NATIONAL_CHAMPIONS:
				priority = 1000;
				headerOptions = { "National champions!", "A winner is crowned", "On top of the world",
								sn[0] + " celebrates natty win", sn[1] + " falls short vs. " + sn[0], "Nothing left to prove" };
				bodyOptions = { sn[0] + " takes down " + sn[1] + " " + i[0] + " and becomes national champions",
								sn[0] + " secure yearlong bragging rights as they best " + sn[1] + " " + i[0] + " for the national title",
								sn[1] + "'s dreams are shattered as they fall to " + sn[0] + " " + i[0],
								"The score is " + i[0] + " as " + sn[0] + " defeats " + sn[1] + " for the championship" };

			case Topic::CONFERENCE_CHAMPIONS:
				priority = 20;
				headerOptions = { i[0] + " champion is crowned", sn[0] + " wins the " + i[0], sn[0] + " team are " + i[0] + " champions", sn[0] + " takes conference title" };
				bodyOptions = { sn[0] + " conquers the " + i[0] + " with a " + i[1] + " win over " + sn[1],
								sn[0] + " outlasts " + sn[1] + " " + i[1] + " for the " + i[0] + " title",
								sn[0] + " deemed conference champions after defeating " + sn[1] + " " + i[1] };
			}
			headline = { *select_randomly(headerOptions.begin(), headerOptions.end()), *select_randomly(bodyOptions.begin(), bodyOptions.end()) };
		}

		std::pair<std::string, std::string> getHeadline() const { return headline; }
	};

	struct StoryCompare {
		bool operator()(Story s1, Story s2) { return s1.priority > s2.priority; }
	};

	double getTopicPriority(Topic t) { return 0; }
	double getStoryPriorityMod(std::vector<School*> s) { return 0; }

	std::priority_queue<Story, std::vector<Story>, StoryCompare> storyStack;

public:
	void addStory(Topic t, std::vector<School*> s = {}, std::vector<Player*> p = {}, std::vector<std::string> i = {}) {
		storyStack.emplace(t, getTopicPriority(t) * getStoryPriorityMod(s), s, p, i);
	}

	void clearStories() {
		while (storyStack.size() > 0) storyStack.pop();
	}

	std::vector<std::pair<std::string, std::string>> getStories(int beginRange, int endRange) {
		std::priority_queue<Story, std::vector<Story>, StoryCompare> tempStack = storyStack;
		std::vector<std::pair<std::string, std::string>> stories;
		for (int i = 0; i < beginRange; i++) { tempStack.pop(); }
		for (int i = beginRange; i < endRange && !tempStack.empty(); i++) {
			stories.push_back(tempStack.top().getHeadline());
			tempStack.pop();
		}
		return stories;
	}
};
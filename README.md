# CFB Simulator
A simple command-line college football simulator.

This project is in an unfinished state. Many features still need work.

## Compiling
You can use CMake to build executable binaries:

```
mkdir build
cd build
cmake ..
make all
```

After that, you can run the main program as well as tests:

```
./main
./cfbSimTests
```

## Objective statement
The objective of this project is to create a thorough replica of the mechanics of college football that is as accurate as reasonably possible. Games like NCAA 14 are fun to play, but they're not built to simulate particularly accurate (or even likely) season outcomes, nor are they built to do it at a robust speed.

In particular, this simulator was conceived in order to answer questions like:
- On average, how many years does it take before Michigan beats Ohio State?
- What is the expected percentage share of national championships Alabama will hold over the next century?
- Is it even possible for a team like Western Kentucky or Texas State to win a national championship?

## Current state of the program
As of last update to this document, a general overview of the project's current features includes:
- Play-by-play game simulator
- Dynamic, randomized scheduling
- Random roster generation based on a starting "prestige" for each school
- A crude (unrealistic) ranking system
- Full-scale coaching trees, from head coaches to special teams coordinators to linebacker coaches
- College football playoffs/championship games

## Roadmap
Large items on the roadmap include:
- Finished game simulator
  - Fumbles, interception returns, punt/kickoff returns, two point conversions
  - Overtime
  - Home/away team advantage
  - Smarter AI decision making, such as when to go for it on 4th down
  - Player rotations and injuries
  - Penalties/ejections
- Finished offseason
  - Individual coach firings (instead of simply firing entire staffs)
  - Early graduation for the NFL draft
- Bowl games and G5 conference championship games
- Awards such as the Heisman

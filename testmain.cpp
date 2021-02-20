#include "recruit.h"
#include <iostream>

struct SortByOvr {
    bool operator()(Recruit a, Recruit b) { return (a.getUnderlyingPlayer()->getOVR() > b.getUnderlyingPlayer()->getOVR()); }
};

int main() {

    srand(time(NULL));

    GlobalData::loadEverything();

    for (int i = 0; i < 30; i++) {
        Recruit recruit = recruitFactory();
        Player* p = recruit.getUnderlyingPlayer();
        printf("%-20s%-3s%-3d%s %-20s\n", p->getName().c_str(), positionToStr(p->getPosition()).c_str(), p->getOVR(), recruit.getPaddedStarString().c_str(), p->getHometown()->formalName().c_str());
    }

    for (int i = 0; i < 10; i++) {
        City* c1 = recruitFactory().getUnderlyingPlayer()->getHometown();
        City* c2 = recruitFactory().getUnderlyingPlayer()->getHometown();

        const double dist = distanceEarth(c1->latitude, c1->longitude, c2->latitude, c2->longitude);
        std::cout << c1->formalName() << " is " << dist << "km away from " << c2->formalName() << std::endl;
    }

    std::vector<Recruit> recruits;
    for (int i = 0; i < 2800; i++) {
        recruits.push_back(recruitFactory());
    }
    SortByOvr sbo;
    std::sort(recruits.begin(), recruits.end(), sbo);
    for (int i = 0; i < 50; i++) {
        Recruit recruit = recruits[i];
        Player* p = recruit.getUnderlyingPlayer();
        printf("%-20s%-3s%-3d%s %-20s\n", p->getName().c_str(), positionToStr(p->getPosition()).c_str(), p->getOVR(), recruit.getPaddedStarString().c_str(), p->getHometown()->formalName().c_str());
    }

    // target distribution: 30-40ish 5 stars, 320-350ish 4 stars
    int count = 0;
    int stars = 5;
    for (Recruit r : recruits) {
        if (r.getStars() < stars) {
            std::cout << stars << " stars: " << count << std::endl;
            count = 0;
            stars -= 1;
        }
        count++;
    }
    std::cout << "0 stars: " << count << std::endl;

    return 0;
}
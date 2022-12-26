#ifndef TEAM_H
#define TEAM_H
#include <vector>

class Worm;
class Team
{
public:
    std::vector<Worm*> vecMembers;
    int nCurrentMember = 0;		// Index into vector for current worms turn
    int nTeamSize = 0;

    bool IsTeamAlive();
    Worm* GetNextMember();
};

#endif // TEAM_H

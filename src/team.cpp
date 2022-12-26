#include "team.h"
#include "worm.h"

bool Team::IsTeamAlive()
{
    // Iterate through all team members, if any of them have >0 health, return true;
    bool bAllDead = false;
    for (auto w : vecMembers)
        bAllDead |= (w->fHealth > 0.0f);
    return bAllDead;
}

Worm *Team::GetNextMember()
{
    // Return a pointer to the next team member that is valid for control
    do {
        nCurrentMember++;
        if (nCurrentMember >= nTeamSize) nCurrentMember = 0;
    } while (vecMembers[nCurrentMember]->fHealth <= 0);
    return vecMembers[nCurrentMember];

}

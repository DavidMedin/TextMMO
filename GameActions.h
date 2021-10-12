#include <stdarg.h>
#include "ecs.h"
#include "server.h"
void Look(Entity looker);
void SpawnGoblin();
void AttackString(Entity attacker,List tokens);
void Attack(Entity attacker,int hand,Entity defender);
void TellEveryone(const char* format,...);
void PickUp(Entity picker,int hand,Entity pickee);

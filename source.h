#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <log.h>

#include "ecs.h"
#include "vec.h"
//#include "Content/equipable.h"
#include "termInput.h"
#include <server.h>
#include "Content/humanoid.h"
#include "Content/lookable.h"
#include "Content/inventory.h"
#include "Content/card.h"
#include "Content/meatbag.h"
#include "Content/item.h"
#include "Content/deck.h"
CompID deleteID;
void DeleteDefered(Entity entity);

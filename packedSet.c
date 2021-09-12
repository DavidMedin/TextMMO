#include "packedSet.h"
extern unsigned int deletedCount;
extern List deleted;//A list of arrays of shorts that represents entities that are deleted.

PackedSet CreatePackedSet(unsigned int itemSize, unsigned short itemPoolCount) {
    PackedSet set = {0};
    set.itemPoolCount = itemPoolCount;
    set.itemSize = itemSize;
    PushBack(&set.sparse.list,malloc(sizeof(unsigned short)*itemPoolCount),sizeof(unsigned short));
    PushBack(&set.packed.list,malloc(itemSize*itemPoolCount),itemSize);
    return set;
}
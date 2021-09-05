#include "packedSet.h"
extern unsigned int deletedCount;
extern List deleted;//A list of arrays of shorts that represents entities that are deleted.

PackedSet CreatePackedSet(unsigned int itemSize, unsigned int itemPoolCount) {
    PackedSet set = {0};
    set.itemPoolCount = itemPoolCount;
    set.itemSize = itemSize;
    PushBack(&set.sparse,malloc(sizeof(int)*itemPoolCount),sizeof(int));
    PushBack(&set.packed,malloc(itemSize*itemPoolCount),itemSize);
    return set;
}
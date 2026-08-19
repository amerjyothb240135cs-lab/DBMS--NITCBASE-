#include "BlockAccess.h"

#include <cstring>

RecId BlockAccess::linearSearch(int relId, char *attrName, Attribute attrVal, int op) {
    RecId prevRecId;

    int ret = RelCacheTable::getSearchIndex(relId, &prevRecId);

    if (ret != SUCCESS) {
        return RecId{-1, -1};
    }

    int block;
    int slot;

    if (prevRecId.block == -1 && prevRecId.slot == -1) {
        RelCatEntry relCatEntry;

        ret = RelCacheTable::getRelCatEntry(relId, &relCatEntry);

        if (ret != SUCCESS) {
            return RecId{-1, -1};
        }

        block = relCatEntry.firstBlk;
        slot = 0;
    } else {
        block = prevRecId.block;
        slot = prevRecId.slot + 1;
    }

    AttrCatEntry attrCatEntry;

    ret = AttrCacheTable::getAttrCatEntry(
        relId,
        attrName,
        &attrCatEntry
    );

    if (ret != SUCCESS) {
        return RecId{-1, -1};
    }

    while (block != -1) {

        RecBuffer recBuffer(block);

        HeadInfo head;

        ret = recBuffer.getHeader(&head);

        if (ret != SUCCESS) {
            return RecId{-1, -1};
        }

        unsigned char slotMap[BLOCK_SIZE];

        ret = recBuffer.getSlotMap(slotMap);

        if (ret != SUCCESS) {
            return RecId{-1, -1};
        }

        if (slot >= head.numSlots) {
            block = head.rblock;
            slot = 0;
            continue;
        }

        if (slotMap[slot] == SLOT_UNOCCUPIED) {
            slot++;
            continue;
        }

        Attribute record[125];

        ret = recBuffer.getRecord(record, slot);

        if (ret != SUCCESS) {
            return RecId{-1, -1};
        }

        int cmpVal;

        cmpVal = compareAttrs(
            record[attrCatEntry.offset],
            attrVal,
            attrCatEntry.attrType
        );

        if (
            (op == NE && cmpVal != 0) ||
            (op == LT && cmpVal < 0) ||
            (op == LE && cmpVal <= 0) ||
            (op == EQ && cmpVal == 0) ||
            (op == GT && cmpVal > 0) ||
            (op == GE && cmpVal >= 0)
        ) {
            RecId currentRecId;

            currentRecId.block = block;
            currentRecId.slot = slot;

            RelCacheTable::setSearchIndex(
                relId,
                &currentRecId
            );

            return currentRecId;
        }

        slot++;
    }

    return RecId{-1, -1};
}

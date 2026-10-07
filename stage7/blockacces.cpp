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

        Attribute record[head.numAttrs];

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
int BlockAccess::renameRelation(char oldName[ATTR_SIZE], char newName[ATTR_SIZE]) {

    RelCacheTable::resetSearchIndex(RELCAT_RELID);

    Attribute newRelationName;
    strcpy(newRelationName.sVal, newName);

    RecId recId = linearSearch(RELCAT_RELID, "RelName", newRelationName, EQ);

    if (recId.block != -1 && recId.slot != -1)
        return E_RELEXIST;

    RelCacheTable::resetSearchIndex(RELCAT_RELID);

    Attribute oldRelationName;
    strcpy(oldRelationName.sVal, oldName);

    recId = linearSearch(RELCAT_RELID, "RelName", oldRelationName, EQ);

    if (recId.block == -1 && recId.slot == -1)
        return E_RELNOTEXIST;

    RecBuffer relCatBuffer(recId.block);

    Attribute relCatEntryRecord[RELCAT_NO_ATTRS];

    int retVal = relCatBuffer.getRecord(relCatEntryRecord, recId.slot);

    if (retVal != SUCCESS)
        return retVal;

    strcpy(relCatEntryRecord[RELCAT_REL_NAME_INDEX].sVal, newName);

    retVal = relCatBuffer.setRecord(relCatEntryRecord, recId.slot);

    if (retVal != SUCCESS)
        return retVal;

    int numAttrs = relCatEntryRecord[RELCAT_NO_ATTRIBUTES_INDEX].nVal;

    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);

    for (int i = 0; i < numAttrs; i++) {

        RecId attrRecId =
            linearSearch(ATTRCAT_RELID, "RelName", oldRelationName, EQ);

        if (attrRecId.block == -1 && attrRecId.slot == -1)
            break;

        RecBuffer attrCatBuffer(attrRecId.block);

        Attribute attrCatEntryRecord[ATTRCAT_NO_ATTRS];

        retVal = attrCatBuffer.getRecord(attrCatEntryRecord, attrRecId.slot);

        if (retVal != SUCCESS)
            return retVal;

        strcpy(attrCatEntryRecord[ATTRCAT_REL_NAME_INDEX].sVal, newName);

        retVal = attrCatBuffer.setRecord(attrCatEntryRecord, attrRecId.slot);

        if (retVal != SUCCESS)
            return retVal;
    }

    return SUCCESS;
}
int BlockAccess::renameAttribute(char relName[ATTR_SIZE],
                                 char oldName[ATTR_SIZE],
                                 char newName[ATTR_SIZE]) {

    RelCacheTable::resetSearchIndex(RELCAT_RELID);

    Attribute relNameAttr;
    strcpy(relNameAttr.sVal, relName);

    RecId recId = linearSearch(RELCAT_RELID, "RelName", relNameAttr, EQ);

    if (recId.block == -1 && recId.slot == -1)
        return E_RELNOTEXIST;

    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);

    RecId attrToRenameRecId{-1, -1};
    Attribute attrCatEntryRecord[ATTRCAT_NO_ATTRS];

    while (true) {

        recId = linearSearch(ATTRCAT_RELID, "RelName", relNameAttr, EQ);

        if (recId.block == -1 && recId.slot == -1)
            break;

        RecBuffer attrCatBuffer(recId.block);

        int retVal = attrCatBuffer.getRecord(attrCatEntryRecord, recId.slot);

        if (retVal != SUCCESS)
            return retVal;

        if (strcmp(attrCatEntryRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, oldName) == 0)
            attrToRenameRecId = recId;

        if (strcmp(attrCatEntryRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, newName) == 0)
            return E_ATTREXIST;
    }

    if (attrToRenameRecId.block == -1 && attrToRenameRecId.slot == -1)
        return E_ATTRNOTEXIST;

    RecBuffer attrCatBuffer(attrToRenameRecId.block);

    int retVal = attrCatBuffer.getRecord(
        attrCatEntryRecord,
        attrToRenameRecId.slot
    );

    if (retVal != SUCCESS)
        return retVal;

    strcpy(attrCatEntryRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, newName);

    retVal = attrCatBuffer.setRecord(
        attrCatEntryRecord,
        attrToRenameRecId.slot
    );

    if (retVal != SUCCESS)
        return retVal;

    return SUCCESS;
}
int BlockAccess::insert(int relId, Attribute *record) {

    RelCatEntry relCatEntry;

    int ret = RelCacheTable::getRelCatEntry(
        relId,
        &relCatEntry
    );

    if (ret != SUCCESS)
        return ret;

    int blockNum = relCatEntry.firstBlk;

    RecId rec_id = {-1, -1};

    int numOfSlots = relCatEntry.numSlotsPerBlk;
    int numOfAttributes = relCatEntry.numAttrs;

    int prevBlockNum = -1;

    while (blockNum != -1) {

        RecBuffer recBuffer(blockNum);

        HeadInfo head;

        ret = recBuffer.getHeader(&head);

        if (ret != SUCCESS)
            return ret;

        unsigned char slotMap[BLOCK_SIZE];

        ret = recBuffer.getSlotMap(slotMap);

        if (ret != SUCCESS)
            return ret;

        for (int i = 0; i < head.numSlots; i++) {

            if (slotMap[i] == SLOT_UNOCCUPIED) {
                rec_id.block = blockNum;
                rec_id.slot = i;
                break;
            }
        }

        if (rec_id.block != -1)
            break;

        prevBlockNum = blockNum;
        blockNum = head.rblock;
    }

    if (rec_id.block == -1) {

        if (relId == RELCAT_RELID)
            return E_MAXRELATIONS;

        RecBuffer recBuffer;

        int ret = recBuffer.getBlockNum();

        if (ret == E_DISKFULL)
            return E_DISKFULL;

        if (ret < 0)
            return ret;

        rec_id.block = ret;
        rec_id.slot = 0;

        HeadInfo head;

        head.blockType = REC;
        head.pblock = -1;
        head.lblock = prevBlockNum;
        head.rblock = -1;
        head.numEntries = 0;
        head.numAttrs = numOfAttributes;
        head.numSlots = numOfSlots;

        ret = recBuffer.setHeader(&head);

        if (ret != SUCCESS)
            return ret;

        unsigned char slotMap[BLOCK_SIZE];

        for (int i = 0; i < numOfSlots; i++)
            slotMap[i] = SLOT_UNOCCUPIED;

        ret = recBuffer.setSlotMap(slotMap);

        if (ret != SUCCESS)
            return ret;

        if (prevBlockNum != -1) {

            RecBuffer prevRecBuffer(prevBlockNum);

            HeadInfo prevHead;

            ret = prevRecBuffer.getHeader(&prevHead);

            if (ret != SUCCESS)
                return ret;

            prevHead.rblock = rec_id.block;

            ret = prevRecBuffer.setHeader(&prevHead);

            if (ret != SUCCESS)
                return ret;

        }
        else {

            relCatEntry.firstBlk = rec_id.block;

            ret = RelCacheTable::setRelCatEntry(
                relId,
                &relCatEntry
            );

            if (ret != SUCCESS)
                return ret;
        }

        relCatEntry.lastBlk = rec_id.block;

        ret = RelCacheTable::setRelCatEntry(
            relId,
            &relCatEntry
        );

        if (ret != SUCCESS)
            return ret;
    }

    RecBuffer recBuffer(rec_id.block);

    ret = recBuffer.setRecord(
        record,
        rec_id.slot
    );

    if (ret != SUCCESS)
        return ret;

    unsigned char slotMap[BLOCK_SIZE];

    ret = recBuffer.getSlotMap(slotMap);

    if (ret != SUCCESS)
        return ret;

    slotMap[rec_id.slot] = SLOT_OCCUPIED;

    ret = recBuffer.setSlotMap(slotMap);

    if (ret != SUCCESS)
        return ret;

    HeadInfo head;

    ret = recBuffer.getHeader(&head);

    if (ret != SUCCESS)
        return ret;

    head.numEntries++;

    ret = recBuffer.setHeader(&head);

    if (ret != SUCCESS)
        return ret;

    relCatEntry.numRecs++;

    ret = RelCacheTable::setRelCatEntry(
        relId,
        &relCatEntry
    );

    if (ret != SUCCESS)
        return ret;

    return SUCCESS;
}

#include "OpenRelTable.h"

#include <cstring>
#include <cstdlib>

OpenRelTableMetaInfo OpenRelTable::tableMetaInfo[MAX_OPEN];

OpenRelTable::OpenRelTable() {

    for (int i = 0; i < MAX_OPEN; ++i) {
        RelCacheTable::relCache[i] = nullptr;
        AttrCacheTable::attrCache[i] = nullptr;
        tableMetaInfo[i].free = true;
    }

    RecBuffer relCatBlock(RELCAT_BLOCK);

    Attribute relCatRecord[RELCAT_NO_ATTRS];

    relCatBlock.getRecord(
        relCatRecord,
        RELCAT_SLOTNUM_FOR_RELCAT
    );

    RelCacheEntry relCacheEntry;

    RelCacheTable::recordToRelCatEntry(
        relCatRecord,
        &relCacheEntry.relCatEntry
    );

    relCacheEntry.recId.block = RELCAT_BLOCK;
    relCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_RELCAT;

    RelCacheTable::relCache[RELCAT_RELID] =
        (RelCacheEntry*)malloc(sizeof(RelCacheEntry));

    *(RelCacheTable::relCache[RELCAT_RELID]) = relCacheEntry;

    relCatBlock.getRecord(
        relCatRecord,
        RELCAT_SLOTNUM_FOR_ATTRCAT
    );

    RelCacheEntry attrCatRelCacheEntry;

    RelCacheTable::recordToRelCatEntry(
        relCatRecord,
        &attrCatRelCacheEntry.relCatEntry
    );

    attrCatRelCacheEntry.recId.block = RELCAT_BLOCK;
    attrCatRelCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_ATTRCAT;

    RelCacheTable::relCache[ATTRCAT_RELID] =
        (RelCacheEntry*)malloc(sizeof(RelCacheEntry));

    *(RelCacheTable::relCache[ATTRCAT_RELID]) = attrCatRelCacheEntry;

    RecBuffer attrCatBlock(ATTRCAT_BLOCK);

    Attribute attrCatRecord[ATTRCAT_NO_ATTRS];

    AttrCacheEntry *head = nullptr;
    AttrCacheEntry *prev = nullptr;

    for (int i = 0; i < RELCAT_NO_ATTRS; ++i) {

        attrCatBlock.getRecord(attrCatRecord, i);

        AttrCacheEntry *attrCacheEntry =
            (AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));

        AttrCacheTable::recordToAttrCatEntry(
            attrCatRecord,
            &attrCacheEntry->attrCatEntry
        );

        attrCacheEntry->recId.block = ATTRCAT_BLOCK;
        attrCacheEntry->recId.slot = i;
        attrCacheEntry->next = nullptr;

        if (head == nullptr) {
            head = attrCacheEntry;
        }
        else {
            prev->next = attrCacheEntry;
        }

        prev = attrCacheEntry;
    }

    AttrCacheTable::attrCache[RELCAT_RELID] = head;

    head = nullptr;
    prev = nullptr;

    for (int i = RELCAT_NO_ATTRS;
         i < RELCAT_NO_ATTRS + ATTRCAT_NO_ATTRS;
         ++i) {

        attrCatBlock.getRecord(attrCatRecord, i);

        AttrCacheEntry *attrCacheEntry =
            (AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));

        AttrCacheTable::recordToAttrCatEntry(
            attrCatRecord,
            &attrCacheEntry->attrCatEntry
        );

        attrCacheEntry->recId.block = ATTRCAT_BLOCK;
        attrCacheEntry->recId.slot = i;
        attrCacheEntry->next = nullptr;

        if (head == nullptr) {
            head = attrCacheEntry;
        }
        else {
            prev->next = attrCacheEntry;
        }

        prev = attrCacheEntry;
    }

    AttrCacheTable::attrCache[ATTRCAT_RELID] = head;

    tableMetaInfo[RELCAT_RELID].free = false;
    strcpy(tableMetaInfo[RELCAT_RELID].relName, RELCAT_RELNAME);

    tableMetaInfo[ATTRCAT_RELID].free = false;
    strcpy(tableMetaInfo[ATTRCAT_RELID].relName, ATTRCAT_RELNAME);
}

int OpenRelTable::getFreeOpenRelTableEntry() {

    for (int i = 2; i < MAX_OPEN; ++i) {
        if (tableMetaInfo[i].free) {
            return i;
        }
    }

    return E_CACHEFULL;
}

int OpenRelTable::getRelId(char relName[ATTR_SIZE]) {

    for (int i = 0; i < MAX_OPEN; ++i) {
        if (!tableMetaInfo[i].free &&
            strcmp(tableMetaInfo[i].relName, relName) == 0) {
            return i;
        }
    }

    return E_RELNOTOPEN;
}

int OpenRelTable::openRel(char relName[ATTR_SIZE]) {

    int relId = getRelId(relName);

    if (relId != E_RELNOTOPEN) {
        return relId;
    }

    relId = getFreeOpenRelTableEntry();

    if (relId == E_CACHEFULL) {
        return E_CACHEFULL;
    }

    RelCacheTable::resetSearchIndex(RELCAT_RELID);

    Attribute relNameVal;
    strcpy(relNameVal.sVal, relName);

    RecId relcatRecId =
        BlockAccess::linearSearch(
            RELCAT_RELID,
            (char*)"relName",
            relNameVal,
            EQ
        );

    if (relcatRecId.block == -1 && relcatRecId.slot == -1) {
        return E_RELNOTEXIST;
    }

    RecBuffer relCatBlock(relcatRecId.block);

    Attribute relCatRecord[RELCAT_NO_ATTRS];

    relCatBlock.getRecord(
        relCatRecord,
        relcatRecId.slot
    );

    RelCacheEntry *relCacheEntry =
        (RelCacheEntry*)malloc(sizeof(RelCacheEntry));

    RelCacheTable::recordToRelCatEntry(
        relCatRecord,
        &relCacheEntry->relCatEntry
    );

    relCacheEntry->recId = relcatRecId;

    RelCacheTable::relCache[relId] = relCacheEntry;

    AttrCacheEntry *listHead = nullptr;
    AttrCacheEntry *prev = nullptr;

    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);

    while (true) {

        RecId attrcatRecId =
            BlockAccess::linearSearch(
                ATTRCAT_RELID,
                (char*)"relName",
                relNameVal,
                EQ
            );

        if (attrcatRecId.block == -1 && attrcatRecId.slot == -1) {
            break;
        }

        RecBuffer attrCatBlock(attrcatRecId.block);

        Attribute attrCatRecord[ATTRCAT_NO_ATTRS];

        attrCatBlock.getRecord(
            attrCatRecord,
            attrcatRecId.slot
        );

        AttrCacheEntry *attrCacheEntry =
            (AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));

        AttrCacheTable::recordToAttrCatEntry(
            attrCatRecord,
            &attrCacheEntry->attrCatEntry
        );

        attrCacheEntry->recId = attrcatRecId;
        attrCacheEntry->next = nullptr;

        if (listHead == nullptr) {
            listHead = attrCacheEntry;
        }
        else {
            prev->next = attrCacheEntry;
        }

        prev = attrCacheEntry;
    }

    AttrCacheTable::attrCache[relId] = listHead;

    tableMetaInfo[relId].free = false;
    strcpy(tableMetaInfo[relId].relName, relName);

    return relId;
}

int OpenRelTable::closeRel(int relId) {

    if (relId == RELCAT_RELID || relId == ATTRCAT_RELID) {
        return E_NOTPERMITTED;
    }

    if (relId < 0 || relId >= MAX_OPEN) {
        return E_OUTOFBOUND;
    }

    if (tableMetaInfo[relId].free) {
        return E_RELNOTOPEN;
    }

    if (RelCacheTable::relCache[relId] != nullptr) {
        free(RelCacheTable::relCache[relId]);
        RelCacheTable::relCache[relId] = nullptr;
    }

    AttrCacheEntry *current =
        AttrCacheTable::attrCache[relId];

    while (current != nullptr) {
        AttrCacheEntry *next = current->next;
        free(current);
        current = next;
    }

    AttrCacheTable::attrCache[relId] = nullptr;

    tableMetaInfo[relId].free = true;

    return SUCCESS;
}

OpenRelTable::~OpenRelTable() {

    for (int i = 2; i < MAX_OPEN; ++i) {
        if (!tableMetaInfo[i].free) {
            OpenRelTable::closeRel(i);
        }
    }

    if (RelCacheTable::relCache[RELCAT_RELID] != nullptr) {
        free(RelCacheTable::relCache[RELCAT_RELID]);
        RelCacheTable::relCache[RELCAT_RELID] = nullptr;
    }

    if (RelCacheTable::relCache[ATTRCAT_RELID] != nullptr) {
        free(RelCacheTable::relCache[ATTRCAT_RELID]);
        RelCacheTable::relCache[ATTRCAT_RELID] = nullptr;
    }

    AttrCacheEntry *current =
        AttrCacheTable::attrCache[RELCAT_RELID];

    while (current != nullptr) {
        AttrCacheEntry *next = current->next;
        free(current);
        current = next;
    }

    AttrCacheTable::attrCache[RELCAT_RELID] = nullptr;

    current =
        AttrCacheTable::attrCache[ATTRCAT_RELID];

    while (current != nullptr) {
        AttrCacheEntry *next = current->next;
        free(current);
        current = next;
    }

    AttrCacheTable::attrCache[ATTRCAT_RELID] = nullptr;
}

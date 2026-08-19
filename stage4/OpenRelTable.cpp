#include "OpenRelTable.h"

#include <cstring>
#include <cstdlib>

OpenRelTable::OpenRelTable() {

    // initialize relCache and attrCache with nullptr
    for (int i = 0; i < MAX_OPEN; ++i) {
        RelCacheTable::relCache[i] = nullptr;
        AttrCacheTable::attrCache[i] = nullptr;
    }

    /************ Setting up Relation Cache entries ************/

    /**** setting up Relation Catalog relation in Relation Cache Table ****/

    RecBuffer relCatBlock(RELCAT_BLOCK);

    Attribute relCatRecord[RELCAT_NO_ATTRS];

    relCatBlock.getRecord(
        relCatRecord,
        RELCAT_SLOTNUM_FOR_RELCAT
    );

    struct RelCacheEntry relCacheEntry;

    RelCacheTable::recordToRelCatEntry(
        relCatRecord,
        &relCacheEntry.relCatEntry
    );

    relCacheEntry.recId.block = RELCAT_BLOCK;
    relCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_RELCAT;

    RelCacheTable::relCache[RELCAT_RELID] =
        (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));

    *(RelCacheTable::relCache[RELCAT_RELID]) = relCacheEntry;


    /**** setting up Attribute Catalog relation in Relation Cache Table ****/

    relCatBlock.getRecord(
        relCatRecord,
        RELCAT_SLOTNUM_FOR_ATTRCAT
    );

    struct RelCacheEntry attrCatRelCacheEntry;

    RelCacheTable::recordToRelCatEntry(
        relCatRecord,
        &attrCatRelCacheEntry.relCatEntry
    );

    attrCatRelCacheEntry.recId.block = RELCAT_BLOCK;
    attrCatRelCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_ATTRCAT;

    RelCacheTable::relCache[ATTRCAT_RELID] =
        (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));

    *(RelCacheTable::relCache[ATTRCAT_RELID]) = attrCatRelCacheEntry;


    /************ Setting up Attribute Cache entries ************/

    RecBuffer attrCatBlock(ATTRCAT_BLOCK);

    Attribute attrCatRecord[ATTRCAT_NO_ATTRS];

    /**** Relation Catalog attributes ****/

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


    /**** Attribute Catalog attributes ****/

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
}


int OpenRelTable::getRelId(char relName[ATTR_SIZE]) {

    if (strcmp(relName, RELCAT_RELNAME) == 0) {
        return RELCAT_RELID;
    }

    if (strcmp(relName, ATTRCAT_RELNAME) == 0) {
        return ATTRCAT_RELID;
    }

    return E_RELNOTOPEN;
}


OpenRelTable::~OpenRelTable() {

    for (int i = 0; i < MAX_OPEN; ++i) {

        if (RelCacheTable::relCache[i] != nullptr) {
            free(RelCacheTable::relCache[i]);
            RelCacheTable::relCache[i] = nullptr;
        }

        AttrCacheEntry *current =
            AttrCacheTable::attrCache[i];

        while (current != nullptr) {

            AttrCacheEntry *next = current->next;

            free(current);

            current = next;
        }

        AttrCacheTable::attrCache[i] = nullptr;
    }
}

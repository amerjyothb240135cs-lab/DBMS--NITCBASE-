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
  // (we need to populate relation cache with entries for the relation catalog
  //  and attribute catalog.)

  /**** setting up Relation Catalog relation in the Relation Cache Table****/
  RecBuffer relCatBlock(RELCAT_BLOCK);

  Attribute relCatRecord[RELCAT_NO_ATTRS];
  relCatBlock.getRecord(relCatRecord, RELCAT_SLOTNUM_FOR_RELCAT);

  struct RelCacheEntry relCacheEntry;
  RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry.relCatEntry);
  relCacheEntry.recId.block = RELCAT_BLOCK;
  relCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_RELCAT;

  // allocate this on the heap because we want it to persist outside this function
  RelCacheTable::relCache[RELCAT_RELID] =
      (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));

  *(RelCacheTable::relCache[RELCAT_RELID]) = relCacheEntry;

  /**** setting up Attribute Catalog relation in the Relation Cache Table ****/

  // set up the relation cache entry for the attribute catalog similarly
  // from the record at RELCAT_SLOTNUM_FOR_ATTRCAT

  // set the value at RelCacheTable::relCache[ATTRCAT_RELID]

  relCatBlock.getRecord(relCatRecord, RELCAT_SLOTNUM_FOR_ATTRCAT);

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


  /**** setting up Students relation in the Relation Cache Table ****/

  // Students is stored at slot 2 in the Relation Catalog
  // RELATIONCAT is at slot 0 and ATTRIBUTECAT is at slot 1

  relCatBlock.getRecord(relCatRecord, 2);

  struct RelCacheEntry studentsRelCacheEntry;
  RelCacheTable::recordToRelCatEntry(
      relCatRecord,
      &studentsRelCacheEntry.relCatEntry
  );

  studentsRelCacheEntry.recId.block = RELCAT_BLOCK;
  studentsRelCacheEntry.recId.slot = 2;

  RelCacheTable::relCache[2] =
      (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));

  *(RelCacheTable::relCache[2]) = studentsRelCacheEntry;


  /************ Setting up Attribute cache entries ************/
  // (we need to populate attribute cache with entries for the relation catalog
  //  and attribute catalog.)

  /**** setting up Relation Catalog relation in the Attribute Cache Table ****/
  RecBuffer attrCatBlock(ATTRCAT_BLOCK);

  Attribute attrCatRecord[ATTRCAT_NO_ATTRS];

  // iterate through all the attributes of the relation catalog and create a linked
  // list of AttrCacheEntry (slots 0 to 5)
  // for each of the entries, set
  //    attrCacheEntry.recId.block = ATTRCAT_BLOCK;
  //    attrCacheEntry.recId.slot = i   (0 to 5)
  //    and attrCacheEntry.next appropriately
  // NOTE: allocate each entry dynamically using malloc

  // set the next field in the last entry to nullptr

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

  /**** setting up Attribute Catalog relation in the Attribute Cache Table ****/

  // set up the attributes of the attribute cache similarly.
  // read slots 6-11 from attrCatBlock and initialise recId appropriately

  // set the value at AttrCacheTable::attrCache[ATTRCAT_RELID]

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


  /**** setting up Students relation in the Attribute Cache Table ****/

  // Students has 4 attributes:
  // RollNumber, Name, Marks and Batch
  // These are stored in slots 12-15 of the Attribute Catalog

  head = nullptr;
  prev = nullptr;

  for (int i = RELCAT_NO_ATTRS + ATTRCAT_NO_ATTRS;
       i < RELCAT_NO_ATTRS + ATTRCAT_NO_ATTRS + 4;
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

  AttrCacheTable::attrCache[2] = head;
}

int OpenRelTable::getRelId(char relName[ATTR_SIZE]) {

  if (strcmp(relName, RELCAT_RELNAME) == 0) {
    return RELCAT_RELID;
  }

  if (strcmp(relName, ATTRCAT_RELNAME) == 0) {
    return ATTRCAT_RELID;
  }

  if (strcmp(relName, "Students") == 0) {
    return 2;
  }

  return E_RELNOTOPEN;
}

OpenRelTable::~OpenRelTable() {
  // free all the memory that you allocated in the constructor

  for (int i = 0; i < MAX_OPEN; ++i) {

    if (RelCacheTable::relCache[i] != nullptr) {
      free(RelCacheTable::relCache[i]);
      RelCacheTable::relCache[i] = nullptr;
    }

    AttrCacheEntry *current = AttrCacheTable::attrCache[i];

    while (current != nullptr) {
      AttrCacheEntry *next = current->next;
      free(current);
      current = next;
    }

    AttrCacheTable::attrCache[i] = nullptr;
  }
}

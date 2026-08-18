#include "BlockBuffer.h"

#include <cstdlib>
#include <cstring>

// Constructor

BlockBuffer::BlockBuffer(int blockNum) {
    this->blockNum = blockNum;
}

// Calls the parent constructor

RecBuffer::RecBuffer(int blockNum)
    : BlockBuffer(blockNum) {
}

// Load the block header into the argument pointer

int BlockBuffer::getHeader(struct HeadInfo *head) {
    unsigned char buffer[BLOCK_SIZE];

    // Read the block
    Disk::readBlock(buffer, this->blockNum);

    memcpy(&head->numSlots, buffer + 24, 4);
    memcpy(&head->numEntries, buffer + 16, 4);
    memcpy(&head->numAttrs, buffer + 20, 4);
    memcpy(&head->rblock, buffer + 12, 4);
    memcpy(&head->lblock, buffer + 8, 4);

    return SUCCESS;
}

// Load the record at slotNum into the argument pointer

int RecBuffer::getRecord(union Attribute *rec, int slotNum) {
    struct HeadInfo head;

    getHeader(&head);

    int attrCount = head.numAttrs;
    int slotCount = head.numSlots;

    unsigned char buffer[BLOCK_SIZE];

    Disk::readBlock(buffer, this->blockNum);

    /*
       Record offset =
       HEADER_SIZE + slotMapSize + recordSize * slotNum
    */

    int recordSize = attrCount * ATTR_SIZE;

    unsigned char *slotPointer =
        buffer + HEADER_SIZE + slotCount +
        (recordSize * slotNum);

    memcpy(rec, slotPointer, recordSize);

    return SUCCESS;
}

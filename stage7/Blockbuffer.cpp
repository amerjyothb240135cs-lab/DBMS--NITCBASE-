#include "BlockBuffer.h"

#include <cstdlib>
#include <cstring>

BlockBuffer::BlockBuffer(char blockType) {
    int block=0;
    
    if(blockType=='R')
    block=REC;
    else if(blockType=='I')
    block=IND_INTERNAL;
    else if(blockType=='L')
    block=IND_LEAF;
    
    int blockNum=getFreeBlock(block);

    // set the blockNum field of the object to that of the allocated block
    // number if the method returned a valid block number,
    // otherwise set the error code returned as the block number.
    
    this->blockNum=blockNum;
}

BlockBuffer::BlockBuffer(int blockNum) {
    this->blockNum = blockNum;
}

RecBuffer::RecBuffer() : BlockBuffer('R') {}

RecBuffer::RecBuffer(int blockNum)
    : BlockBuffer(blockNum) {
}

int BlockBuffer::getBlockNum() {
    return this->blockNum;
}

int BlockBuffer::getHeader(struct HeadInfo *head) {

    unsigned char *bufferPtr;

    int ret = loadBlockAndGetBufferPtr(&bufferPtr);

    if (ret != SUCCESS) {
        return ret;
    }

    memcpy(&head->numSlots, bufferPtr + 24, 4);
    memcpy(&head->numEntries, bufferPtr + 16, 4);
    memcpy(&head->numAttrs, bufferPtr + 20, 4);
    memcpy(&head->rblock, bufferPtr + 12, 4);
    memcpy(&head->lblock, bufferPtr + 8, 4);

    return SUCCESS;
}

int BlockBuffer::setHeader(struct HeadInfo *head) {

    unsigned char *bufferPtr;

    int ret = loadBlockAndGetBufferPtr(&bufferPtr);

    if (ret != SUCCESS)
        return ret;

    struct HeadInfo *bufferHeader = (struct HeadInfo *)bufferPtr;

    bufferHeader->blockType = head->blockType;
    bufferHeader->pblock = head->pblock;
    bufferHeader->lblock = head->lblock;
    bufferHeader->rblock = head->rblock;
    bufferHeader->numEntries = head->numEntries;
    bufferHeader->numAttrs = head->numAttrs;
    bufferHeader->numSlots = head->numSlots;

    ret = StaticBuffer::setDirtyBit(this->blockNum);

    if (ret != SUCCESS)
        return ret;

    return SUCCESS;
}

int RecBuffer::getRecord(union Attribute *rec, int slotNum) {

    struct HeadInfo head;

    int ret = getHeader(&head);

    if (ret != SUCCESS) {
        return ret;
    }

    int attrCount = head.numAttrs;
    int slotCount = head.numSlots;

    unsigned char *bufferPtr;

    ret = loadBlockAndGetBufferPtr(&bufferPtr);

    if (ret != SUCCESS) {
        return ret;
    }

    int recordSize = attrCount * ATTR_SIZE;

    unsigned char *slotPointer =
        bufferPtr + HEADER_SIZE + slotCount +
        (recordSize * slotNum);

    memcpy(rec, slotPointer, recordSize);

    return SUCCESS;
}

int RecBuffer::getSlotMap(unsigned char *slotMap) {
    unsigned char *bufferPtr;

    int ret = loadBlockAndGetBufferPtr(&bufferPtr);

    if (ret != SUCCESS) {
        return ret;
    }

    struct HeadInfo head;

    ret = getHeader(&head);

    if (ret != SUCCESS) {
        return ret;
    }

    int slotCount = head.numSlots;

    unsigned char *slotMapInBuffer =
        bufferPtr + HEADER_SIZE;

    memcpy(slotMap, slotMapInBuffer, slotCount);

    return SUCCESS;
}

int RecBuffer::setSlotMap(unsigned char *slotMap) {

    unsigned char *bufferPtr;

    int ret = loadBlockAndGetBufferPtr(&bufferPtr);

    if (ret != SUCCESS)
        return ret;

    struct HeadInfo head;

    ret = getHeader(&head);

    if (ret != SUCCESS)
        return ret;

    int numSlots = head.numSlots;

    unsigned char *slotMapInBuffer =
        bufferPtr + HEADER_SIZE;

    memcpy(slotMapInBuffer, slotMap, numSlots);

    ret = StaticBuffer::setDirtyBit(this->blockNum);

    if (ret != SUCCESS)
        return ret;

    return SUCCESS;
}

int RecBuffer::setRecord(union Attribute *rec, int slotNum) {

    unsigned char *bufferPtr;

    int ret = loadBlockAndGetBufferPtr(&bufferPtr);

    if (ret != SUCCESS)
        return ret;

    struct HeadInfo head;

    ret = getHeader(&head);

    if (ret != SUCCESS)
        return ret;

    int numAttrs = head.numAttrs;
    int numSlots = head.numSlots;

    if (slotNum < 0 || slotNum >= numSlots)
        return E_OUTOFBOUND;

    int recordSize = ATTR_SIZE * numAttrs;

    unsigned char *slotPointer =
        bufferPtr + HEADER_SIZE + numSlots +
        (slotNum * recordSize);

    memcpy(slotPointer, rec, recordSize);

    ret = StaticBuffer::setDirtyBit(this->blockNum);

    if (ret != SUCCESS)
        return ret;

    return SUCCESS;
}

int compareAttrs(union Attribute attr1, union Attribute attr2, int attrType) {
    double diff;

    if (attrType == STRING) {
        diff = strcmp(attr1.sVal, attr2.sVal);
    } else {
        diff = attr1.nVal - attr2.nVal;
    }

    if (diff > 0)
        return 1;

    if (diff < 0)
        return -1;

    return 0;
}

int BlockBuffer::loadBlockAndGetBufferPtr(unsigned char **buffPtr) {

    int bufferNum = StaticBuffer::getBufferNum(this->blockNum);

    if (bufferNum == E_OUTOFBOUND)
        return E_OUTOFBOUND;

    if (bufferNum == E_BLOCKNOTINBUFFER) {

        bufferNum = StaticBuffer::getFreeBuffer(this->blockNum);

        if (bufferNum == E_OUTOFBOUND)
            return E_OUTOFBOUND;

        int ret = Disk::readBlock(
            StaticBuffer::blocks[bufferNum],
            this->blockNum
        );

        if (ret != SUCCESS)
            return ret;

    } else {

        for (int i = 0; i < BUFFER_CAPACITY; i++) {
            if (!StaticBuffer::metainfo[i].free &&
                i != bufferNum) {
                StaticBuffer::metainfo[i].timeStamp++;
            }
        }

        StaticBuffer::metainfo[bufferNum].timeStamp = 0;
    }

    *buffPtr = StaticBuffer::blocks[bufferNum];

    return SUCCESS;
}
int BlockBuffer::setBlockType(int blockType) {

    unsigned char *bufferPtr;

    int ret = loadBlockAndGetBufferPtr(&bufferPtr);

    if (ret != SUCCESS)
        return ret;

    *((int32_t *)bufferPtr) = blockType;

    StaticBuffer::blockAllocMap[this->blockNum] = blockType;

    ret = StaticBuffer::setDirtyBit(this->blockNum);

    if (ret != SUCCESS)
        return ret;

    return SUCCESS;
}
int BlockBuffer::getFreeBlock(int blockType) {

    int blockNum = -1;

    for (int i = BLOCK_ALLOCATION_MAP_SIZE; i < DISK_BLOCKS; i++) {
        if (StaticBuffer::blockAllocMap[i] == UNUSED_BLK) {
            blockNum = i;
            break;
        }
    }

    if (blockNum == -1)
        return E_DISKFULL;

    this->blockNum = blockNum;

    int bufferNum = StaticBuffer::getFreeBuffer(this->blockNum);

    if (bufferNum < 0)
        return bufferNum;

    struct HeadInfo head;
     head.blockType = UNUSED_BLK;
    head.pblock = -1;
    head.lblock = -1;
    head.rblock = -1;
    head.numEntries = 0;
    head.numAttrs = 0;
    head.numSlots = 0;

    int ret = setHeader(&head);

    if (ret != SUCCESS)
        return ret;

    ret = setBlockType(blockType);

    if (ret != SUCCESS)
        return ret;

    return this->blockNum;
}

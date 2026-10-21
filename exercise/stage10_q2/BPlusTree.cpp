#include "BPlusTree.h"

#include <cstring>

extern int comparisons;

RecId BPlusTree::bPlusSearch(int relId, char attrName[ATTR_SIZE], Attribute attrVal, int op) {
    IndexId searchIndex;

    int ret = AttrCacheTable::getSearchIndex(relId, attrName, &searchIndex);

    if (ret != SUCCESS)
        return RecId{-1, -1};

    AttrCatEntry attrCatEntry;

    ret = AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatEntry);

    if (ret != SUCCESS)
        return RecId{-1, -1};

    if (attrCatEntry.rootBlock == -1)
        return RecId{-1, -1};

    int block, index;

    if (searchIndex.block == -1 && searchIndex.index == -1) {
        block = attrCatEntry.rootBlock;
        index = 0;
    }
    else {
        block = searchIndex.block;
        index = searchIndex.index + 1;

        IndLeaf leaf(block);
        HeadInfo leafHead;

        ret = leaf.getHeader(&leafHead);

        if (ret != SUCCESS)
            return RecId{-1, -1};

        if (index >= leafHead.numEntries) {
            block = leafHead.rblock;
            index = 0;

            if (block == -1)
                return RecId{-1, -1};
        }
    }

    while (StaticBuffer::getStaticBlockType(block) == IND_INTERNAL) {
        IndInternal internalBlk(block);
        HeadInfo intHead;

        ret = internalBlk.getHeader(&intHead);

        if (ret != SUCCESS || intHead.numEntries <= 0)
            return RecId{-1, -1};

        InternalEntry intEntry;

        if (op == NE || op == LT || op == LE) {
            ret = internalBlk.getEntry(&intEntry, 0);

            if (ret != SUCCESS)
                return RecId{-1, -1};

            block = intEntry.lChild;
        }
        else {
            bool found = false;

            for (int i = 0; i < intHead.numEntries; i++) {
                ret = internalBlk.getEntry(&intEntry, i);

                if (ret != SUCCESS)
                    return RecId{-1, -1};

                comparisons++;
                int cmpVal = compareAttrs(
                    intEntry.attrVal,
                    attrVal,
                    attrCatEntry.attrType
                );

                if (((op == EQ || op == GE) && cmpVal >= 0) ||
                    (op == GT && cmpVal > 0)) {
                    block = intEntry.lChild;
                    found = true;
                    break;
                }
            }

            if (!found) {
                ret = internalBlk.getEntry(
                    &intEntry, intHead.numEntries - 1
                );

                if (ret != SUCCESS)
                    return RecId{-1, -1};

                block = intEntry.rChild;
            }
        }
    }

    while (block != -1) {
        if (StaticBuffer::getStaticBlockType(block) != IND_LEAF)
            return RecId{-1, -1};

        IndLeaf leafBlk(block);
        HeadInfo leafHead;

        ret = leafBlk.getHeader(&leafHead);

        if (ret != SUCCESS)
            return RecId{-1, -1};

        Index leafEntry;

        while (index < leafHead.numEntries) {
            ret = leafBlk.getEntry(&leafEntry, index);

            if (ret != SUCCESS)
                return RecId{-1, -1};

            comparisons++;
            int cmpVal = compareAttrs(
                leafEntry.attrVal,
                attrVal,
                attrCatEntry.attrType
            );

            if ((op == EQ && cmpVal == 0) ||
                (op == LE && cmpVal <= 0) ||
                (op == LT && cmpVal < 0) ||
                (op == GT && cmpVal > 0) ||
                (op == GE && cmpVal >= 0) ||
                (op == NE && cmpVal != 0)) {

                searchIndex.block = block;
                searchIndex.index = index;

                ret = AttrCacheTable::setSearchIndex(
                    relId, attrName, &searchIndex
                );

                if (ret != SUCCESS)
                    return RecId{-1, -1};

                return RecId{leafEntry.block, leafEntry.slot};
            }
            else if ((op == EQ || op == LE || op == LT) && cmpVal > 0) {
                return RecId{-1, -1};
            }

            index++;
        }

        if (op != NE)
            break;

        block = leafHead.rblock;
        index = 0;
    }

    return RecId{-1, -1};
}

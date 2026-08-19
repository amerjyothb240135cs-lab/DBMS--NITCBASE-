#include "Algebra.h"

#include <cstring>
#include <cstdio>
#include <cstdlib>

bool isNumber(char *str);

int Algebra::select(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE],
                    char attr[ATTR_SIZE], int op, char strVal[ATTR_SIZE]) {

    int srcRelId = OpenRelTable::getRelId(srcRel);

    if (srcRelId == E_RELNOTOPEN) {
        return E_RELNOTOPEN;
    }

    AttrCatEntry attrCatEntry;

    int ret = AttrCacheTable::getAttrCatEntry(
        srcRelId,
        attr,
        &attrCatEntry
    );

    if (ret == E_ATTRNOTEXIST) {
        return E_ATTRNOTEXIST;
    }

    if (ret != SUCCESS) {
        return ret;
    }

    int type = attrCatEntry.attrType;
    Attribute attrVal;

    if (type == NUMBER) {
        if (isNumber(strVal)) {
            attrVal.nVal = atof(strVal);
        } else {
            return E_ATTRTYPEMISMATCH;
        }
    } else if (type == STRING) {
        strcpy(attrVal.sVal, strVal);
    }

    ret = RelCacheTable::resetSearchIndex(srcRelId);

    if (ret != SUCCESS) {
        return ret;
    }

    RelCatEntry relCatEntry;

    ret = RelCacheTable::getRelCatEntry(
        srcRelId,
        &relCatEntry
    );

    if (ret != SUCCESS) {
        return ret;
    }

    printf("|");

    for (int i = 0; i < relCatEntry.numAttrs; ++i) {
        AttrCatEntry tempAttrCatEntry;

        ret = AttrCacheTable::getAttrCatEntry(
            srcRelId,
            i,
            &tempAttrCatEntry
        );

        if (ret != SUCCESS) {
            return ret;
        }

        printf(" %s |", tempAttrCatEntry.attrName);
    }

    printf("\n");

    while (true) {

        RecId searchRes = BlockAccess::linearSearch(
            srcRelId,
            attr,
            attrVal,
            op
        );

        if (searchRes.block != -1 && searchRes.slot != -1) {

            RecBuffer recBuffer(searchRes.block);

            Attribute record[125];

            ret = recBuffer.getRecord(
                record,
                searchRes.slot
            );

            if (ret != SUCCESS) {
                return ret;
            }

            printf("|");

            for (int i = 0; i < relCatEntry.numAttrs; ++i) {

                AttrCatEntry tempAttrCatEntry;

                ret = AttrCacheTable::getAttrCatEntry(
                    srcRelId,
                    i,
                    &tempAttrCatEntry
                );

                if (ret != SUCCESS) {
                    return ret;
                }

                if (tempAttrCatEntry.attrType == NUMBER) {
                    printf(" %g |",
                           record[i].nVal);
                } else {
                    printf(" %s |",
                           record[i].sVal);
                }
            }

            printf("\n");

        } else {
            break;
        }
    }

    return SUCCESS;
}

bool isNumber(char *str) {
    int len;
    float ignore;

    int ret = sscanf(str, "%f %n", &ignore, &len);

    return ret == 1 && len == strlen(str);
}

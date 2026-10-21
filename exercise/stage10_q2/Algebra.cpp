#include "Algebra.h"

#include <cstring>
#include <cstdio>
#include <cstdlib>

int comparisons = 0;

bool isNumber(char *str);

int Algebra::insert(char relName[ATTR_SIZE], int nAttrs, char record[][ATTR_SIZE]) {

    if (strcmp(relName, RELCAT_RELNAME) == 0 ||
        strcmp(relName, ATTRCAT_RELNAME) == 0)
        return E_NOTPERMITTED;

    int relId = OpenRelTable::getRelId(relName);

    if (relId == E_RELNOTOPEN)
        return E_RELNOTOPEN;

    RelCatEntry relCatEntry;

    int retVal = RelCacheTable::getRelCatEntry(
        relId,
        &relCatEntry
    );

    if (retVal != SUCCESS)
        return retVal;

    if (relCatEntry.numAttrs != nAttrs)
        return E_NATTRMISMATCH;

    union Attribute recordValues[nAttrs];

    for (int i = 0; i < nAttrs; i++) {

        AttrCatEntry attrCatEntry;

        retVal = AttrCacheTable::getAttrCatEntry(
            relId,
            i,
            &attrCatEntry
        );

        if (retVal != SUCCESS)
            return retVal;

        int type = attrCatEntry.attrType;

        if (type == NUMBER) {

            if (isNumber(record[i])) {
                recordValues[i].nVal = atof(record[i]);
            }
            else {
                return E_ATTRTYPEMISMATCH;
            }

        }
        else if (type == STRING) {

            strcpy(recordValues[i].sVal, record[i]);
        }
    }

    retVal = BlockAccess::insert(relId, recordValues);

    return retVal;
}

int Algebra::select(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE],
                    char attr[ATTR_SIZE], int op, char strVal[ATTR_SIZE]) {

    int srcRelId = OpenRelTable::getRelId(srcRel);

    if (srcRelId == E_RELNOTOPEN)
        return E_RELNOTOPEN;

    AttrCatEntry attrCatEntry;

    int ret = AttrCacheTable::getAttrCatEntry(
        srcRelId, attr, &attrCatEntry
    );

    if (ret != SUCCESS)
        return E_ATTRNOTEXIST;

    Attribute attrVal;

    int type = attrCatEntry.attrType;

    if (type == NUMBER) {
        if (!isNumber(strVal))
            return E_ATTRTYPEMISMATCH;

        attrVal.nVal = atof(strVal);
    }
    else if (type == STRING) {
        strcpy(attrVal.sVal, strVal);
    }

    RelCatEntry relCatEntry;

    ret = RelCacheTable::getRelCatEntry(srcRelId, &relCatEntry);

    if (ret != SUCCESS)
        return ret;

    int src_nAttrs = relCatEntry.numAttrs;

    char attr_names[src_nAttrs][ATTR_SIZE];
    int attr_types[src_nAttrs];

    for (int i = 0; i < src_nAttrs; i++) {
        AttrCatEntry entry;

        ret = AttrCacheTable::getAttrCatEntry(
            srcRelId, i, &entry
        );

        if (ret != SUCCESS)
            return ret;

        strcpy(attr_names[i], entry.attrName);
        attr_types[i] = entry.attrType;
    }

    ret = Schema::createRel(
        targetRel, src_nAttrs, attr_names, attr_types
    );

    if (ret != SUCCESS)
        return ret;

    int targetRelId = OpenRelTable::openRel(targetRel);

    if (targetRelId < 0) {
        Schema::deleteRel(targetRel);
        return targetRelId;
    }

    ret = RelCacheTable::resetSearchIndex(srcRelId);

    if (ret != SUCCESS) {
        Schema::closeRel(targetRel);
        Schema::deleteRel(targetRel);
        return ret;
    }
    
    ret = AttrCacheTable::resetSearchIndex(srcRelId, attr);

  if (ret != SUCCESS) {
    Schema::closeRel(targetRel);
    Schema::deleteRel(targetRel);
    return ret;
  }

    comparisons = 0;

    Attribute record[src_nAttrs];

    while (true) {
        ret = BlockAccess::search(
            srcRelId, record, attr, attrVal, op
        );

        if (ret == E_NOTFOUND)
            break;

        if (ret != SUCCESS) {
            Schema::closeRel(targetRel);
            Schema::deleteRel(targetRel);
            return ret;
        }

        ret = BlockAccess::insert(targetRelId, record);

        if (ret != SUCCESS) {
            Schema::closeRel(targetRel);
            Schema::deleteRel(targetRel);
            return ret;
        }
    }

    printf("Number of comparisons: %d\n", comparisons);

    ret = Schema::closeRel(targetRel);

    if (ret != SUCCESS)
        return ret;

    return SUCCESS;
}

bool isNumber(char *str) {
    int len;
    float ignore;

    int ret = sscanf(str, "%f %n", &ignore, &len);

    return ret == 1 && len == strlen(str);
}

int Algebra::project(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE]) {

    int srcRelId = OpenRelTable::getRelId(srcRel);

    if (srcRelId == E_RELNOTOPEN)
        return E_RELNOTOPEN;

    RelCatEntry relCatEntry;

    int ret = RelCacheTable::getRelCatEntry(
        srcRelId, &relCatEntry
    );

    if (ret != SUCCESS)
        return ret;

    int numAttrs = relCatEntry.numAttrs;

    char attrNames[numAttrs][ATTR_SIZE];
    int attrTypes[numAttrs];

    for (int i = 0; i < numAttrs; i++) {
        AttrCatEntry attrCatEntry;

        ret = AttrCacheTable::getAttrCatEntry(
            srcRelId, i, &attrCatEntry
        );

        if (ret != SUCCESS)
            return ret;

        strcpy(attrNames[i], attrCatEntry.attrName);
        attrTypes[i] = attrCatEntry.attrType;
    }

    ret = Schema::createRel(
        targetRel, numAttrs, attrNames, attrTypes
    );

    if (ret != SUCCESS)
        return ret;

    int targetRelId = OpenRelTable::openRel(targetRel);

    if (targetRelId < 0) {
        Schema::deleteRel(targetRel);
        return targetRelId;
    }

    RelCacheTable::resetSearchIndex(srcRelId);

    Attribute record[numAttrs];

    while ((ret = BlockAccess::project(srcRelId, record)) == SUCCESS) {

        ret = BlockAccess::insert(targetRelId, record);

        if (ret != SUCCESS) {
            Schema::closeRel(targetRel);
            Schema::deleteRel(targetRel);
            return ret;
        }
    }

    if (ret != E_NOTFOUND) {
        Schema::closeRel(targetRel);
        Schema::deleteRel(targetRel);
        return ret;
    }

    Schema::closeRel(targetRel);

    return SUCCESS;
}

int Algebra::project(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE],
                     int tar_nAttrs, char tar_Attrs[][ATTR_SIZE]) {

    int srcRelId = OpenRelTable::getRelId(srcRel);

    if (srcRelId == E_RELNOTOPEN)
        return E_RELNOTOPEN;

    RelCatEntry relCatEntry;

    int ret = RelCacheTable::getRelCatEntry(
        srcRelId, &relCatEntry
    );

    if (ret != SUCCESS)
        return ret;

    int src_nAttrs = relCatEntry.numAttrs;

    int attr_offset[tar_nAttrs];
    int attr_types[tar_nAttrs];

    for (int i = 0; i < tar_nAttrs; i++) {
        AttrCatEntry attrCatEntry;

        ret = AttrCacheTable::getAttrCatEntry(
            srcRelId, tar_Attrs[i], &attrCatEntry
        );

        if (ret != SUCCESS)
            return E_ATTRNOTEXIST;

        attr_offset[i] = attrCatEntry.offset;
        attr_types[i] = attrCatEntry.attrType;
    }

    ret = Schema::createRel(
        targetRel, tar_nAttrs, tar_Attrs, attr_types
    );

    if (ret != SUCCESS)
        return ret;

    int targetRelId = OpenRelTable::openRel(targetRel);

    if (targetRelId < 0) {
        Schema::deleteRel(targetRel);
        return targetRelId;
    }

    ret = RelCacheTable::resetSearchIndex(srcRelId);

    if (ret != SUCCESS) {
        Schema::closeRel(targetRel);
        Schema::deleteRel(targetRel);
        return ret;
    }

    Attribute record[src_nAttrs];

    while (true) {
        ret = BlockAccess::project(srcRelId, record);

        if (ret == E_NOTFOUND)
            break;

        if (ret != SUCCESS) {
            Schema::closeRel(targetRel);
            Schema::deleteRel(targetRel);
            return ret;
        }

        Attribute proj_record[tar_nAttrs];

        for (int i = 0; i < tar_nAttrs; i++) {
            proj_record[i] = record[attr_offset[i]];
        }

        ret = BlockAccess::insert(targetRelId, proj_record);

        if (ret != SUCCESS) {
            Schema::closeRel(targetRel);
            Schema::deleteRel(targetRel);
            return ret;
        }
    }

    ret = Schema::closeRel(targetRel);

    if (ret != SUCCESS)
        return ret;

    return SUCCESS;
}

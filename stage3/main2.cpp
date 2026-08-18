#include "Disk_Class/Disk.h"
#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Cache/RelCacheTable.h"
#include "Cache/AttrCacheTable.h"
#include <cstdio>

int main(int argc, char *argv[]) {
    Disk disk_run;
    StaticBuffer buffer;
    OpenRelTable cache;

    /*
    for i = 0 and i = 1 (i.e RELCAT_RELID and ATTRCAT_RELID)

      get the relation catalog entry using RelCacheTable::getRelCatEntry()
      printf("Relation: %s\n", relname);

      for j = 0 to numAttrs of the relation - 1
          get the attribute catalog entry for (rel-id i, attribute offset j)
           in attrCatEntry using AttrCacheTable::getAttrCatEntry()

          printf("  %s: %s\n", attrName, attrType);
    */

    for (int i = 0; i <= 1; i++) {

        RelCatEntry relCatEntry;

        int ret = RelCacheTable::getRelCatEntry(i, &relCatEntry);

        if (ret != SUCCESS) {
            printf("Error getting relation catalog entry\n");
            return 0;
        }

        printf("Relation: %s\n", relCatEntry.relName);

        for (int j = 0; j < relCatEntry.numAttrs; j++) {

            AttrCatEntry attrCatEntry;

            ret = AttrCacheTable::getAttrCatEntry(
                i,
                j,
                &attrCatEntry
            );

            if (ret != SUCCESS) {
                printf("Error getting attribute catalog entry\n");
                return 0;
            }
             const char *type = attrCatEntry.attrType == NUMBER ? "NUM" : "STR";
            printf("  %s: %s\n",
                   attrCatEntry.attrName,
                   type);
        }
    }

    return 0;
}

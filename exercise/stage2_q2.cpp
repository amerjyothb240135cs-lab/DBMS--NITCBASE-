#include <cstring>
#include <iostream>
#include "Disk_Class/Disk.h"
#include "Buffer/StaticBuffer.h"
#include "Buffer/BlockBuffer.h"
#include "Cache/OpenRelTable.h"
#include "FrontendInterface/FrontendInterface.h"

using namespace std;

int main()
{
    Disk diskRun;

    RecBuffer relBuffer(RELCAT_BLOCK);

    HeadInfo relHead;
    relBuffer.getHeader(&relHead);

    for (int i = 0; i < relHead.numEntries; i = i + 1)
    {
        Attribute relRec[RELCAT_NO_ATTRS];
        relBuffer.getRecord(relRec, i);

        cout << "Relation: "
             << relRec[RELCAT_REL_NAME_INDEX].sVal
             << endl;

        
    int block = ATTRCAT_BLOCK;

    while (block != INVALID_BLOCKNUM)
    {
        RecBuffer attrBuffer(block);

        HeadInfo head;
        attrBuffer.getHeader(&head);

        for (int i = 0; i < head.numEntries; i = i + 1)
        {
            Attribute rec[ATTRCAT_NO_ATTRS];
            attrBuffer.getRecord(rec, i);

            if (strcmp(relRec[RELCAT_REL_NAME_INDEX].sVal, "Students") == 0 &&
                strcmp(rec[ATTRCAT_ATTR_NAME_INDEX].sVal, "Class") == 0)
            {
                strcpy(rec[ATTRCAT_ATTR_NAME_INDEX].sVal, "Batch");
            }

            if (strcmp(rec[ATTRCAT_REL_NAME_INDEX].sVal, relRec[RELCAT_REL_NAME_INDEX].sVal) == 0)
            {
                cout << "  "
                     << rec[ATTRCAT_ATTR_NAME_INDEX].sVal
                     << " : ";

                if (rec[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER)
                    cout << "NUM";
                else
                    cout << "STR";

                cout << endl;
            }
        }

        block = head.rblock;
    }

        cout << endl;
    }

    return 0;
}

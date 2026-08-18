#include <iostream>
#include <cstring>

#include "Disk_Class/Disk.h"
#include "Cache/OpenRelTable.h"
#include "Buffer/BlockBuffer.h"

using namespace std;

int main(int argc, char *argv[])
{
    Disk diskRun;

    RecBuffer relCatBuffer(RELCAT_BLOCK);

    HeadInfo relCatHeader;
    relCatBuffer.getHeader(&relCatHeader);

    for (int i = 0; i < relCatHeader.numEntries; i = i + 1)
    {
        Attribute relCatRecord[RELCAT_NO_ATTRS];
        relCatBuffer.getRecord(relCatRecord, i);

        cout << "Relation: "
             << relCatRecord[RELCAT_REL_NAME_INDEX].sVal
             << endl;

        int block = ATTRCAT_BLOCK;

        while (block != INVALID_BLOCKNUM)
        {
            RecBuffer attrCatBuffer(block);

            HeadInfo attrCatHeader;
            attrCatBuffer.getHeader(&attrCatHeader);

            for (int j = 0; j < attrCatHeader.numEntries; j = j + 1)
            {
                Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
                attrCatBuffer.getRecord(attrCatRecord, j);

                if (strcmp(relCatRecord[RELCAT_REL_NAME_INDEX].sVal,
                           attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal) == 0)
                {
                    cout << "  "
                         << attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal
                         << " : ";

                    if (attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER)
                    {
                        cout << "NUM";
                    }
                    else
                    {
                        cout << "STR";
                    }

                    cout << endl;
                }
            }

            block = attrCatHeader.rblock;
        }

        cout << endl;
    }

    return 0;
}

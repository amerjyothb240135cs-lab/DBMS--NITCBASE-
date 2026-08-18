#include "Disk_Class/Disk.h"
#include "Cache/OpenRelTable.h"
#include "Buffer/BlockBuffer.h"
#include <iostream>
#include <cstring>
using namespace std;

int main(int argc, char *argv[])
{
    Disk diskRun;
    StaticBuffer buffer;
    RecBuffer relCatBuffer(RELCAT_BLOCK);
    RecBuffer attrCatBuffer(ATTRCAT_BLOCK);

    HeadInfo relCatHeader;
    HeadInfo attrCatHeader;

    relCatBuffer.getHeader(&relCatHeader);
    attrCatBuffer.getHeader(&attrCatHeader);

    for (int i = 0; i < relCatHeader.numEntries; i = i + 1)
    {
        Attribute relCatRecord[RELCAT_NO_ATTRS];

        relCatBuffer.getRecord(relCatRecord, i);

        cout << "Relation: "<< relCatRecord[RELCAT_REL_NAME_INDEX].sVal << endl;

        for (int j = 0; j < attrCatHeader.numEntries; j = j + 1)
        {
            Attribute attrCatRecord[ATTRCAT_NO_ATTRS];

            attrCatBuffer.getRecord(attrCatRecord, j);

            if (strcmp(relCatRecord[RELCAT_REL_NAME_INDEX].sVal,
                       attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal) == 0)
            {
                cout << "  "<< attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal << ": ";
        
        const char *attrType = attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER ? "NUM" : "STR";
        cout << attrType << endl;    
      }
         /*if (attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER)
                {
                    cout << "NUM";
                }
                else
                {
                    cout << "STR";
                }*/

            }
            cout << endl;
        }
return 0;
    }


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <vector>
#include <string>

#include "Record.h"

#define CH_MAX (4)
std::vector<RecordData *> RecordTable[CH_MAX];


void ResetRecord(){
    for(int iI = 0; iI < CH_MAX;++iI){
        for(RecordData *record:RecordTable[iI]){
            free(record);
        }
        RecordTable[iI].clear();
    }
}



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <vector>
#include <string>

#include "Record.h"

#define CH_MAX (4)
std::vector<RecordData *> RecordTable[CH_MAX];

static int PsgFNum[] = {
    0x6ae, 0x64e, 0x5f4, 0x59e, 0x54e, 0x502, 0x4ba, 0x476, 0x436, 0x3f8, 0x3c0, 0x38a, // 0
    0x357, 0x327, 0x2fa, 0x2cf, 0x2a7, 0x281, 0x25d, 0x23b, 0x21b, 0x1fc, 0x1e0, 0x1c5, // 1
    0x1ac, 0x194, 0x17d, 0x168, 0x153, 0x140, 0x12e, 0x11d, 0x10d, 0x0fe, 0x0f0, 0x0e3, // 2
    0x0d6, 0x0ca, 0x0be, 0x0b4, 0x0aa, 0x0a0, 0x097, 0x08f, 0x087, 0x07f, 0x078, 0x071, // 3
    0x06b, 0x065, 0x05f, 0x05a, 0x055, 0x050, 0x04c, 0x047, 0x043, 0x040, 0x03c, 0x039, // 4
    0x035, 0x032, 0x030, 0x02d, 0x02a, 0x028, 0x026, 0x024, 0x022, 0x020, 0x01e, 0x01c, // 5
    0x01b, 0x019, 0x018, 0x016, 0x015, 0x014, 0x013, 0x012, 0x011, 0x010, 0x00f, 0x00e, // 6
    0x00d, 0x00d, 0x00c, 0x00b, 0x00b, 0x00a, 0x009, 0x008, 0x007, 0x006, 0x005, 0x004  // 7
};


void ResetRecord(){
    for(int iI = 0; iI < CH_MAX;++iI){
        for(RecordData *record:RecordTable[iI]){
            free(record);
        }
        RecordTable[iI].clear();
    }
}

int CalcNote(int freq)
{
    int m = 0x7fffffff;
    int n = 0;

    for (int i = 0; i < 12 * 8; i++)
    {
        int a = freq - PsgFNum[i];

        if (m > a)
        {
            m = a;
            n = i;
        }
    }
    return n;
}

void WriteRecord(int chan, unsigned short freq, unsigned short vol, int frameCount)
{
    bool addflag = false;
    int note = (int)freq;
    if (chan < 4) {
        //note = CalcNote((int)freq);
    }
    if (RecordTable[chan].empty()) {
        addflag = true;
    }
    else {
        RecordData* oldrecord = RecordTable[chan][RecordTable[chan].size() - 1];
        if (note != oldrecord->note) {
            oldrecord->time = frameCount - oldrecord->time;
            addflag = true;
        }
        if (vol == 0x0f) {
            oldrecord->time = frameCount - oldrecord->time;
            addflag = true;
        }
    }
    if (addflag) {
        RecordData* record = (RecordData*)malloc(sizeof(RecordData));
        if (record != NULL)
        {
            record->note = note;
            record->vol = vol;
            record->time = frameCount;
            RecordTable[chan].push_back(record);
        }
    }
}


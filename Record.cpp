
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
    0x00d, 0x00d, 0x00c, 0x00b, 0x00b, 0x00a, 0x009, 0x008, 0x007, 0x006, 0x005, 0x004, // 7
    0x00,
};
static int fnumsize = sizeof(PsgFNum) / sizeof(int);

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
    int n = 0;
    int calcfreq = freq;

    for (int iI = 0; iI < fnumsize; iI++)
    {
        int max = PsgFNum[iI];
        int min = PsgFNum[iI+1];
        if ((calcfreq >= min) && (calcfreq < max))
        {
            n = iI;
            break;
        }
    }
    return n;
}
void callback()
{

}

void WriteRecord(int chan, unsigned short freq, unsigned short vol, int frameCount)
{
    if (chan == 0) {
        callback();
    }
    bool addflag = false;
    if (RecordTable[chan].empty()) {
        addflag = true;
    }
    else {
        RecordData* oldrecord = RecordTable[chan][RecordTable[chan].size() - 1];
        if(vol == 0x0f)
        {
            // mute data
            freq = 0;
            if (vol == oldrecord->vol) {
                oldrecord->elaptime = frameCount;
            }
            else {
                addflag = true;
            }
        }
        else if (freq != oldrecord->freq) {
            oldrecord->elaptime = frameCount;
            addflag = true;
        }
        else if (vol != oldrecord->vol) {
            oldrecord->elaptime = frameCount;
            addflag = true;
        }
        else {
            // freq/vol —¼•û‚Æ‚à“¯‚¶ƒf[ƒ^‚¾‚Á‚½
            oldrecord->elaptime = frameCount;
        }
    }
    if (addflag) {
        RecordData* record = (RecordData*)malloc(sizeof(RecordData));
        if (record != NULL)
        {
            record->freq = freq;
            record->note = 0;
            if (chan < 3 && freq != 0) {
                record->note = CalcNote((int)freq);
            }
            record->vol = vol;
            record->starttime = frameCount;
            record->elaptime = frameCount;
            RecordTable[chan].push_back(record);
        }
    }
}

#define WBUFF_SIZE   (1024)
char wbuf_tmp[WBUFF_SIZE];
char wbuf[WBUFF_SIZE];
char* note_str[] = {
    "C", "C+",
    "D", "D+",
    "E",
    "F", "F+",
    "G", "G+",
    "A", "A+",
    "B",
};
void DumpRecord(char* filename)
{
    FILE* fp = fopen(filename, "wt+");
    int wsize;
    if (fp != NULL) {
        for (int iI = 0; iI < CH_MAX; ++iI) {
            wsize = sprintf_s(wbuf, WBUFF_SIZE, ";Channel %d Start -----", iI);
            fwrite(wbuf, wsize, 1, fp);
            RecordData* oldrecord = NULL;
            for (RecordData* record : RecordTable[iI]) {
                int time = record->elaptime - record->starttime;
                if (iI < (CH_MAX - 1))
                {
                    // tone channel
                    sprintf_s(wbuf_tmp, WBUFF_SIZE, " ");
                    if (record->vol == 0x0f) {
                        //if(!((oldrecord != NULL) && (oldrecord->vol != record->vol)))
                        {
                            sprintf_s(wbuf_tmp, WBUFF_SIZE, "REST ");
                        }
                    }
                    else if (record->freq == 0) {
                        sprintf_s(wbuf_tmp, WBUFF_SIZE, "---- ");
                    }
                    else if((oldrecord != NULL) && (oldrecord->note != record->note))
                    {
                        int o = record->note / 12;
                        int n = record->note % 12;
                        sprintf_s(wbuf_tmp, WBUFF_SIZE, "o%d %s ", o, note_str[n]);
                    }

                    if (oldrecord == NULL) {
                        wsize = sprintf_s(wbuf, WBUFF_SIZE, "\n%s[V%d:%d]", wbuf_tmp, record->vol, time);
                    }
                    else if (oldrecord->note == record->note)
                    {
                        wsize = sprintf_s(wbuf, WBUFF_SIZE, "%s[V%d:%d] ", wbuf_tmp, record->vol, time);
                    }
                    else
                    {
                        wsize = sprintf_s(wbuf, WBUFF_SIZE, "\n%s[V%d:%d] ", wbuf_tmp, record->vol, time);
                    }
                    oldrecord = record;
                    // tone channel
                }
                else {
                    // noise channel
                    wsize = sprintf_s(wbuf, WBUFF_SIZE, " freq 0x%04x V%d %d\n", record->freq, record->vol, time);
                }
                fwrite(wbuf, wsize, 1, fp);
            }
            wsize = sprintf_s(wbuf, WBUFF_SIZE, "\n;Channel %d End -----\n", iI);
            fwrite(wbuf, wsize, 1, fp);
        }
        fclose(fp);
    }
}

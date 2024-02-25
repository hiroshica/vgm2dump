
#include "./Record.h"

#define CH_MAX (4)
typedef std::shared_ptr<RecordData> RecordDataP;
std::vector<RecordDataP> RecordTable[CH_MAX];
double BaseSpeed;

static int PsgFNum[] = {
    0x6ae,
    0x64e,
    0x5f4,
    0x59e,
    0x54e,
    0x502,
    0x4ba,
    0x476,
    0x436,
    0x3f8,
    0x3c0,
    0x38a, // 0
    0x357,
    0x327,
    0x2fa,
    0x2cf,
    0x2a7,
    0x281,
    0x25d,
    0x23b,
    0x21b,
    0x1fc,
    0x1e0,
    0x1c5, // 1
    0x1ac,
    0x194,
    0x17d,
    0x168,
    0x153,
    0x140,
    0x12e,
    0x11d,
    0x10d,
    0x0fe,
    0x0f0,
    0x0e3, // 2
    0x0d6,
    0x0ca,
    0x0be,
    0x0b4,
    0x0aa,
    0x0a0,
    0x097,
    0x08f,
    0x087,
    0x07f,
    0x078,
    0x071, // 3
    0x06b,
    0x065,
    0x05f,
    0x05a,
    0x055,
    0x050,
    0x04c,
    0x047,
    0x043,
    0x040,
    0x03c,
    0x039, // 4
    0x035,
    0x032,
    0x030,
    0x02d,
    0x02a,
    0x028,
    0x026,
    0x024,
    0x022,
    0x020,
    0x01e,
    0x01c, // 5
    0x01b,
    0x019,
    0x018,
    0x016,
    0x015,
    0x014,
    0x013,
    0x012,
    0x011,
    0x010,
    0x00f,
    0x00e, // 6
    0x00d,
    0x00d,
    0x00c,
    0x00b,
    0x00b,
    0x00a,
    0x009,
    0x008,
    0x007,
    0x006,
    0x005,
    0x004, // 7
    0x00,
};
static int fnumsize = sizeof(PsgFNum) / sizeof(int);

void StartRecord(double basespeed)
{
    BaseSpeed = ceil(basespeed);
}
void ResetRecord()
{
    for (int iI = 0; iI < CH_MAX; ++iI)
    {
#if false
        for(RecordData *record:RecordTable[iI]){
            free(record);
        }
#endif
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
        int min = PsgFNum[iI + 1];
        if ((calcfreq >= min) && (calcfreq < max))
        {
            n = iI;
            break;
        }
    }
    return n;
}
void debugcallback()
{
}

void WriteRecord(int chan, unsigned short freq, unsigned short vol, int frameCount)
{
    if (chan == 0)
    {
        debugcallback();
    }
    bool addflag = false;
    if (RecordTable[chan].empty())
    {
        addflag = true;
    }
    else
    {
        RecordDataP oldrecord = RecordTable[chan][RecordTable[chan].size() - 1];
        if (vol == 0x0f)
        {
            // mute data
            freq = 0;
            if (vol == oldrecord->vol)
            {
                oldrecord->elaptime = frameCount;
            }
            else
            {
                addflag = true;
            }
        }
        else if (freq != oldrecord->freq)
        {
            oldrecord->elaptime = frameCount;
            addflag = true;
        }
        else if (vol != oldrecord->vol)
        {
            oldrecord->elaptime = frameCount;
            addflag = true;
        }
        else
        {
            // freq/vol 両方とも同じデータだった
            oldrecord->elaptime = frameCount;
        }
    }
    if (addflag)
    {
        // RecordData* record = (RecordData*)malloc(sizeof(RecordData));
        std::shared_ptr<RecordData> record(new RecordData());
        if (record != NULL)
        {
            record->freq = freq;
            record->note = 0;
            if (chan < 3 && freq != 0)
            {
                record->note = CalcNote((int)freq);
            }
            record->vol = vol;
            record->starttime = frameCount;
            record->elaptime = frameCount;
            RecordTable[chan].push_back(record);
        }
    }
}

// #define WBUFF_SIZE   (1024)
// char wbuf_tmp[WBUFF_SIZE];
// char wbuf[WBUFF_SIZE];
const char *note_str[] = {
    "C",
    "C+",
    "D",
    "D+",
    "E",
    "F",
    "F+",
    "G",
    "G+",
    "A",
    "A+",
    "B",
};
void DumpRecord(char *filename)
{
    FILE *fp = fopen(filename, "wt+");
    int wsize;
    std::vector<NoteValueP> NoteValues;
    std::string str;

    if (fp != NULL)
    {
        for (int iI = 0; iI < CH_MAX; ++iI)
        {
            NoteValues.clear();
            NoteValueP newNoteValue = NULL;
            str.clear();
            int old_oct = -1;
            for (RecordDataP record : RecordTable[iI])
            {
                int time = record->elaptime - record->starttime;
                bool addf = false;
                if (newNoteValue == NULL)
                {
                    addf = true;
                }
                else if (newNoteValue->note == record->note)
                {
                    addf = false;
                    newNoteValue->time.push_back(time);
                    newNoteValue->vol.push_back(record->vol);
                    newNoteValue->totaltime += time;
                }
                else
                {
                    addf = true;
                }
                if (addf)
                {
                    if (newNoteValue != NULL)
                    {
                        NoteValues.push_back(newNoteValue);
                    }
                    newNoteValue = NoteValueP(new NoteValue());
                    newNoteValue->note = record->note;
                    newNoteValue->time.push_back(time);
                    newNoteValue->vol.push_back(record->vol);
                    newNoteValue->totaltime = time;
                }
            }
            // channel データ収集OK
            str += std::format(";Channel {} Start -----\n", iI);
            for (NoteValueP record : NoteValues)
            {
                if (record->note == 0)
                {
                    str += std::format(" REST [V{}:{}]\t\t;\n", record->vol[0], record->totaltime);
                }
                else {
                    int o = record->note / 12;
                    int n = record->note % 12;
                    if (old_oct != o) {
                        str += std::format(" o{} ", o);
                        old_oct = o;
                    }
                    str += std::format(" {} [V{}:{}]\t\t; ", note_str[n], record->vol[0], record->totaltime);
                    for (int iJ = 0; iJ < record->time.size(); ++iJ)
                    {
                        str += std::format("[V{}:{}] ", record->vol[iJ], record->time[iJ]);
                    }
                    str += "\n";
                }
            }
            str += std::format(";Channel {} End -----\n", iI);
            fwrite(str.c_str(), str.size(), 1, fp);
        }

#if 0
        for (int iI = 0; iI < CH_MAX; ++iI) {
            std::string outstr;
            std::string str;
            str = std::format(";Channel {} Start -----", iI);
            outstr = str;
            RecordDataP oldrecord = NULL;
            for (RecordDataP record : RecordTable[iI]) {
                int time = record->elaptime - record->starttime;
                str.clear();
                if (iI < (CH_MAX - 1))
                {
                    // tone channel
                    str = " ";
                    if (record->vol == 0x0f) {
                        //if(!((oldrecord != NULL) && (oldrecord->vol != record->vol)))
                        {
                            str += "REST ";
                        }
                    }
                    else if (record->freq == 0) {
                        str += "---- ";
                    }
                    else if((oldrecord != NULL) && (oldrecord->note != record->note))
                    {
                        int o = record->note / 12;
                        int n = record->note % 12;
                        str +=std::format("o{} {} ", o, note_str[n]);
                    }

                    if (oldrecord == NULL) {
                        outstr += std::format("\n{}[V{}:{}]", str, record->vol, time);
                    }
                    else if (oldrecord->note == record->note)
                    {
                        outstr += std::format("{}[V{}:{}] ", str, record->vol, time);
                    }
                    else
                    {
                        outstr += std::format("\n{}[V{}:{}] ", str, record->vol, time);
                    }
                    oldrecord = record;
                    // tone channel
                }
                else {
                    // noise channel
                    outstr += std::format(" freq 0x{:#x} V{} {}\n", record->freq, record->vol, time);
                }
                fwrite(outstr.c_str(), outstr.size(), 1, fp);
                outstr.clear();
            }
            outstr = std::format("\n;Channel {} End -----\n", iI);
            fwrite(outstr.c_str(), outstr.size(), 1, fp);
        }
#endif
        fclose(fp);
    }
}

#include <MMLRecord.h>



//------------------------------------------------------------------------------------
// clang-format off
//------------------------------------------------------------------------------------
static int PsgFNum[] = {
            0x6ae,0x64e,0x5f4,0x59e,0x54e,0x502,0x4ba,0x476,0x436,0x3f8,0x3c0,0x38a, // 0
            0x357,0x327,0x2fa,0x2cf,0x2a7,0x281,0x25d,0x23b,0x21b,0x1fc,0x1e0,0x1c5, // 1
            0x1ac,0x194,0x17d,0x168,0x153,0x140,0x12e,0x11d,0x10d,0x0fe,0x0f0,0x0e3, // 2
            0x0d6,0x0ca,0x0be,0x0b4,0x0aa,0x0a0,0x097,0x08f,0x087,0x07f,0x078,0x071, // 3
            0x06b,0x065,0x05f,0x05a,0x055,0x050,0x04c,0x047,0x043,0x040,0x03c,0x039, // 4
            0x035,0x032,0x030,0x02d,0x02a,0x028,0x026,0x024,0x022,0x020,0x01e,0x01c, // 5
            0x01b,0x019,0x018,0x016,0x015,0x014,0x013,0x012,0x011,0x010,0x00f,0x00e, // 6
            0x00d,0x00d,0x00c,0x00b,0x00b,0x00a,0x009,0x008,0x007,0x006,0x005,0x004  // 7
};
static int fnumsize = sizeof(PsgFNum) / sizeof(int);

static const char* note_str[] = {
    "c", "c+",
    "d", "d+",
    "e",
    "f", "f+",
    "g", "g+",
    "a", "a+",
    "b",
};
static const char* Chan_str[] =
{
    "G",
    "H",
    "I",
    "J"
};

//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// clang-format on
//------------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------------
MMLRecord::MMLRecord(int inframe, std::string dstname)
{
	m_BaseFrame = inframe;
    m_DstName = new std::string(dstname.c_str());
    for (int iI = 0; iI < CH_MAX; ++iI)
    {
        RecordTableP newtable(new RecordDataV());
        m_RecordTables.push_back(newtable);
    }
}
//------------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------------
MMLRecord::~MMLRecord()
{
    m_RecordTables.clear();
}

//------------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------------
static void debugcallback()
{
}
//------------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------------
int MMLRecord::CalcTime(int inframe)
{
    int retcode = inframe / m_BaseFrame;
    return retcode;
}
//------------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------------
int MMLRecord::CalcNote(int freq)
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

//------------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------------
void MMLRecord::WriteRecord(int chan, unsigned short freq, unsigned short vol, int frameCount)
{
    RecordTableP RecordTable = m_RecordTables[chan];
    if (chan == 3)
    {
        debugcallback();
    }
    bool addflag = false;
    if (RecordTable->empty())
    {
        addflag = true;
    }
    else
    {
        RecordDataP oldrecord = RecordTable->at(RecordTable->size() - 1);
        if ((vol == oldrecord->vol) && (freq == oldrecord->freq))
        {
            // ひとつ前と同じデータなので同じ扱い

            oldrecord->elaptime++;
        }
        else {
            addflag = true;
        }
    }
    if (addflag)
    {
        std::shared_ptr<RecordData> record(new RecordData());
        if (record != NULL)
        {
            record->freq = freq;
            record->note = 0;
            if (chan < (CH_MAX - 1) && freq != 0)
            {
                record->note = CalcNote((int)freq);
            }
            else {
                record->note = (int)freq;
            }
            record->vol = vol;
            //record->starttime = frameCount;
            record->elaptime = 1;
            RecordTable->push_back(record);
        }
    }

}

//------------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------------
int MMLRecord::checkVoiceData(NoteValueP noteValue)
{
    int voiceno = -1;
    if (noteValue->vol.size() > 1)
    {
        bool addf = true;
        for (int iI = 0; iI < VoiceValues.size(); iI++) {
            VoiceValueP srcVoice = VoiceValues[iI];
            if (srcVoice->vol.size() != noteValue->vol.size()) {
                continue;
            }
            bool samef = true;
            for (int iC = 0; iC < noteValue->vol.size(); ++iC)
            {
                if (srcVoice->vol[iC] != noteValue->vol[iC]) {
                    samef = false;
                    break;
                }
                if (srcVoice->time[iC] != noteValue->time[iC]) {
                    samef = false;
                    break;
                }
            }
            if (samef) {
                voiceno = iI;
                addf = false;
                break;
            }
        }
        if (addf) {
            VoiceValueP newVoice = VoiceValueP(new VoiceValue());
            newVoice->vol = noteValue->vol;
            newVoice->time = noteValue->time;
            voiceno = VoiceValues.size();
            VoiceValues.push_back(newVoice);
        }
    }
    return voiceno;
}
//------------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------------
void MMLRecord::DumpRecord()
{

    FILE* fp = fopen(m_DstName->c_str(), "wt+");
    int wsize;
    std::string str;

    if (fp != NULL)
    {
        for (int iI = 0; iI < CH_MAX; ++iI)
        {
            RecordTableP RecordTable = m_RecordTables[iI];

            NoteValues[iI].clear();
            NoteValueP newNoteValue = NULL;
            for (int LP = 0; LP < RecordTable->size(); ++LP)
            {
                if (LP > 480) {
                    debugcallback();
                }
                RecordDataP record = RecordTable->at(LP);
                int time = record->elaptime;    // -record->starttime;
                bool addf = false;
                if (iI < (CH_MAX - 1))
                {
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
                }
                else {
                    addf = true;
                }
                if (addf)
                {
                    if (newNoteValue != NULL)
                    {
                        if (newNoteValue->note != 0) {
                            newNoteValue->voiceno = checkVoiceData(newNoteValue);
                        }
                        else {
                            newNoteValue->voiceno = -1;
                        }
                    }
                    newNoteValue = NoteValueP(new NoteValue());
                    newNoteValue->note = record->note;
                    newNoteValue->time.push_back(time);
                    newNoteValue->vol.push_back(record->vol);
                    newNoteValue->totaltime = time;
                    NoteValues[iI].push_back(newNoteValue);
                }
            }
            if (newNoteValue != NULL)
            {
                if (newNoteValue->note != 0) {
                    newNoteValue->voiceno = checkVoiceData(newNoteValue);
                }
                else {
                    newNoteValue->voiceno = -1;
                }
            }

            // channel データ収集OK
        }
        // defalu setup
        str += "#platform mdsdrv\n\n";
        str += "; voice datas\n";
        // voice data out
        for (int iI = 0; iI < VoiceValues.size(); ++iI)
        {
            VoiceValueP record = VoiceValues[iI];
            str += std::format("@{} psg\n", iI);
            for (int iJ = 0; iJ < record->vol.size(); ++iJ) {
                int time = CalcTime(record->time[iJ]);
                str += std::format(" {}:{} ", 15 - record->vol[iJ], time);
            }
            str += "\n";
        }
        fwrite(str.c_str(), str.size(), 1, fp);
        for (int iI = 0; iI < CH_MAX; ++iI)
        {
            str.clear();
            str += std::format(";Channel {} Start -----\n{}\n", iI, Chan_str[iI]);
            int calctime = 0;
            int calcVol = -1;
            int old_oct = -1;
            int old_voice = -1;
            for (NoteValueP record : NoteValues[iI])
            {
                int totaltime = CalcTime(record->totaltime);
                calctime += record->totaltime;
                if (record->note == 0)
                {
                    str += std::format(" r:{}", totaltime);
                }
                else {
                    if (iI < (CH_MAX - 1))
                    {
                        int o = (record->note / 12) + 1;
                        int n = record->note % 12;
                        if (old_oct != o) {
                            str += std::format(" o{}", o);
                            old_oct = o;
                        }
                        {
                            if ((old_voice != record->voiceno) && (record->vol[0] != 0x000f))
                            {
                                if (record->voiceno != -1) {
                                    str += std::format(" @{}", record->voiceno);
                                    old_voice = record->voiceno;
                                }
                            }
                            if (calcVol != record->vol[0]) {
                                str += std::format(" V{}{}:{}", 15 - record->vol[0], note_str[n], totaltime);
                                calcVol = record->vol[0];
                            }
                            else {
                                str += std::format(" {}:{}", note_str[n], totaltime);
                            }
                        }
                    }
                    else {
                        str += std::format(";{}:{}:{}\n", record->note, record->vol[0], totaltime);
                    }
                }
                if (calctime >= m_BaseFrame) {
                    str += "\n";
                    calctime = 0;
                }
            }
            str += std::format("\n;Channel {} End -----\n", iI);
            fwrite(str.c_str(), str.size(), 1, fp);
        }
        fclose(fp);
    }
}


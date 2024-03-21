#pragma once

//#include <stdlib.h>
//#include <stdio.h>
//#include <string.h>
//#include <ctype.h>

#include <cstdio>
#include <vector>
#include <string>
#include <format>
#include <memory>
#include <cmath>

typedef struct _NoteValue
{
    int                        note;
    int                        totaltime;
    int                        voiceno;
    std::vector<int>           time;
    std::vector<unsigned short> vol;
} NoteValue;
typedef std::shared_ptr<NoteValue> NoteValueP;

typedef struct _VoiceValue
{
    std::vector<int>           time;
    std::vector<unsigned short> vol;
} VoiceValue;
typedef std::shared_ptr<VoiceValue> VoiceValueP;


typedef struct recorddata {
    int    note;
    unsigned short freq;
    unsigned short vol;
    int    starttime;
    int    elaptime;
} RecordData;
typedef std::shared_ptr<RecordData> RecordDataP;
typedef std::vector<RecordDataP> RecordDataV;
typedef std::shared_ptr<RecordDataV> RecordTableP;

#define CH_MAX (4)

class MMLRecord {
private:
    std::vector<RecordTableP> m_RecordTables;
    int m_BaseFrame;
    std::string *m_DstName;
    std::vector<NoteValueP> NoteValues[CH_MAX];
    std::vector<VoiceValueP> VoiceValues;

    //double BaseSpeed;

    MMLRecord() {};

public:
    MMLRecord(int inframe, std::string dstname);
    ~MMLRecord();

    int CalcTime(int inframe);
    int CalcNote(int freq);


    void WriteRecord(int chan, unsigned short freq, unsigned short vol, int frameCount);
    int checkVoiceData(NoteValueP noteValue);
    void DumpRecord();

};
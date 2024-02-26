#ifndef __RECORD_H__
#define __RECORD_H__

#include <stdlib.h>
//#include <stdio.h>
//#include <string.h>
//#include <ctype.h>
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


typedef struct recorddata{
    int    note;
    unsigned short freq;
    unsigned short vol;
    int    starttime;
    int    elaptime;
} RecordData;
typedef std::shared_ptr<RecordData> RecordDataP;

void StartRecord(double basespeed, int inframe);
void ResetRecord();
void WriteRecord(int chan, unsigned short freq, unsigned short vol, int frameCount);
void DumpRecord(char* filename);


#endif	// __RECORD_H__

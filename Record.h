#ifndef __RECORD_H__
#define __RECORD_H__

typedef struct recorddata{
    int    note;
    unsigned short freq;
    unsigned short vol;
    int    starttime;
    int    elaptime;
} RecordData;

void StartRecord(double basespeed);
void ResetRecord();
void WriteRecord(int chan, unsigned short freq, unsigned short vol, int frameCount);
void DumpRecord(char* filename);


#endif	// __RECORD_H__

#ifndef __RECORD_H__
#define __RECORD_H__

typedef struct recorddata{
    int    note;
    int    vol;
    int    time;
} RecordData;

void ResetRecord();
void WriteRecord(int chan, unsigned short freq, unsigned short vol, int frameCount);

#endif	// __RECORD_H__

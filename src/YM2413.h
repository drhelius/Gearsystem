#ifndef YM2413_H
#define YM2413_H

#include "definitions.h"

class YM2413
{

public:
    YM2413();
    ~YM2413();

    void Init(int clockRate);
    void Reset(int clockRate);
    void Write(u8 port, u8 value);
    u8 Read(u8 port);
    void Tick(unsigned int clockCycles);
    int EndFrame(s16* pSampleBuffer);
    void SaveState(std::ostream& stream);
    void LoadState(std::istream& stream);

private:
    int m_iCycleCounter;
    int m_iSampleCounter;
    s16* m_pBuffer;
    int m_iBufferIndex;
    int m_ElapsedCycles;
    int m_iClockRate;
    u8 m_RegisterF2;
};

#endif	/* YM2413_H */

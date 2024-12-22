#ifndef KOREAN_MSX_8KB_0300_MEMORY_RULE_H
#define KOREAN_MSX_8KB_0300_MEMORY_RULE_H

#include "MemoryRule.h"

class KoreanMSX8KB0300MemoryRule : public MemoryRule
{
public:
    KoreanMSX8KB0300MemoryRule(Memory* pMemory, Cartridge* pCartridge, Input* pInput);
    virtual ~KoreanMSX8KB0300MemoryRule();
    virtual u8 PerformRead(u16 address);
    virtual void PerformWrite(u16 address, u8 value);
    virtual void Reset();
    virtual u8* GetPage(int index);
    virtual int GetBank(int index);
    virtual void SaveState(std::ostream& stream);
    virtual void LoadState(std::istream& stream);

private:
    int m_iPageAddress[6];
};

#endif // KOREAN_MSX_8KB_0300_MEMORY_RULE_H

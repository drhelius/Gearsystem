#ifndef JUMBODAHJEE_MEMORY_RULE_H
#define JUMBODAHJEE_MEMORY_RULE_H

#include "MemoryRule.h"

class JumboDahjeeMemoryRule : public MemoryRule
{
public:
    JumboDahjeeMemoryRule(Memory* pMemory, Cartridge* pCartridge, Input* pInput);
    virtual ~JumboDahjeeMemoryRule();
    virtual u8 PerformRead(u16 address);
    virtual void PerformWrite(u16 address, u8 value);
    virtual void Reset();
    virtual u8* GetPage(int index);
    virtual int GetBank(int index);

private:
    u8* m_pCartRAM;
};

#endif  /* JUMBODAHJEE_MEMORY_RULE_H */

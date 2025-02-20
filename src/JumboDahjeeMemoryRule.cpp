#include "JumboDahjeeMemoryRule.h"
#include "Memory.h"
#include "Cartridge.h"

JumboDahjeeMemoryRule::JumboDahjeeMemoryRule(Memory* pMemory, Cartridge* pCartridge, Input* pInput)
    : MemoryRule(pMemory, pCartridge, pInput)
{
    m_pCartRAM = new u8[0x2000];
    Reset();
}

JumboDahjeeMemoryRule::~JumboDahjeeMemoryRule()
{
    SafeDeleteArray(m_pCartRAM);
}

u8 JumboDahjeeMemoryRule::PerformRead(u16 address)
{
    if (address >= 0x2000 && address < 0x4000)
        return m_pCartRAM[address - 0x2000];
    else
    {
        if (address >= 0xC000)
            address = 0xC000 + (address & 0x3FF);

        return m_pMemory->Retrieve(address);
    }
}

void JumboDahjeeMemoryRule::PerformWrite(u16 address, u8 value)
{
    if (address < 0x2000)
    {
        Debug("--> ** Attempting to write on ROM address $%X %X", address, value);
    }
    else if (address < 0x4000)
    {
        m_pCartRAM[address - 0x2000] = value;
    }
    else if (address < 0xC000)
    {
        Debug("--> ** Attempting to write on ROM address $%X %X", address, value);
    }
    else
    {
        address = 0xC000 + (address & 0x3FF);
        m_pMemory->Load(address, value);
    }
}

void JumboDahjeeMemoryRule::Reset()
{
    for (int i = 0; i < 0x2000; i++)
        m_pCartRAM[i] = 0;
}

u8* JumboDahjeeMemoryRule::GetPage(int index)
{
    if ((index >= 0) && (index < 3))
        return m_pMemory->GetMemoryMap() + (0x4000 * index);
    else
        return NULL;
}

int JumboDahjeeMemoryRule::GetBank(int index)
{
    if ((index >= 0) && (index < 3))
        return index;
    else
        return 0;
}

#include "KoreanMSX8KB0300MemoryRule.h"
#include "Memory.h"
#include "Cartridge.h"

KoreanMSX8KB0300MemoryRule::KoreanMSX8KB0300MemoryRule(Memory* pMemory, Cartridge* pCartridge, Input* pInput)
    : MemoryRule(pMemory, pCartridge, pInput)
{
    Reset();
}

KoreanMSX8KB0300MemoryRule::~KoreanMSX8KB0300MemoryRule()
{
}

u8 KoreanMSX8KB0300MemoryRule::PerformRead(u16 address)
{
    if (address < 0xC000)
    {
        int slot = (address >> 13) & 0x07;
        if (slot > 5)
        {
            Debug("--> ** Invalid slot %d", slot);
            return 0xFF;
        }
        return m_pCartridge->GetROM()[m_iPageAddress[slot] + (address & 0x1FFF)];
    }
    else
    {
        // RAM + RAM mirror
        return m_pMemory->Retrieve(address);
    }
}

void KoreanMSX8KB0300MemoryRule::PerformWrite(u16 address, u8 value)
{
    u16 address_assumed = address & 0xFF00;
    int bank = value & (m_pCartridge->GetROMBankCount8k() - 1);

    switch (address_assumed)
    {
    case 0x0000:
        m_iPage[4] = bank;
        m_iPageAddress[4] = 0x2000 * bank;
        return;
    case 0x0100:
        m_iPage[2] = bank;
        m_iPageAddress[2] = 0x2000 * (value & (m_pCartridge->GetROMBankCount8k() - 1));
        return;
    case 0x0200:
        m_iPage[1] = bank;
        m_iPage[5] = m_iPage[1];
        m_iPageAddress[1] = 0x2000 * bank;
        m_iPageAddress[5] = m_iPageAddress[1];
        return;
    case 0x0300:
        m_iPage[3] = bank;
        m_iPageAddress[3] = 0x2000 * bank;;
        return;
    default:
        break;
    }

    if (address >= 0xC000)
    {
        if (address < 0xE000)
        {
            // RAM
            m_pMemory->Load(address, value);
            m_pMemory->Load(address + 0x2000, value);
        }
        else
        {
            // RAM (mirror)
            m_pMemory->Load(address, value);
            m_pMemory->Load(address - 0x2000, value);
        }
    }
}

void KoreanMSX8KB0300MemoryRule::Reset()
{
    for (int i = 0; i < 6; i++)
    {
        m_iPage[i] = i;
        m_iPageAddress[i] = 0x2000 * m_iPage[i];
    }
}

u8* KoreanMSX8KB0300MemoryRule::GetPage(int index)
{
    return m_pCartridge->GetROM() + m_iPageAddress[index];
}

int KoreanMSX8KB0300MemoryRule::GetBank(int index)
{
    return m_iPage[index];
}

bool KoreanMSX8KB0300MemoryRule::Has8kBanks()
{
    return true;
}

void KoreanMSX8KB0300MemoryRule::SaveState(std::ostream& stream)
{
    stream.write(reinterpret_cast<const char*> (m_iPageAddress), sizeof(m_iPageAddress));
    stream.write(reinterpret_cast<const char*> (m_iPage), sizeof(m_iPage));
}

void KoreanMSX8KB0300MemoryRule::LoadState(std::istream& stream)
{
    using namespace std;

    stream.read(reinterpret_cast<char*> (m_iPageAddress), sizeof(m_iPageAddress));
    stream.read(reinterpret_cast<char*> (m_iPage), sizeof(m_iPage));
}

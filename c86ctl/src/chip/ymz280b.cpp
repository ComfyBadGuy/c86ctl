#include "stdafx.h"
#include <stdlib.h>
#include <cstdint>
#include <algorithm>
#include "ymz280b.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

using namespace c86ctl;

// ファイル内定数化（マジックナンバーを名前で表現）
namespace {
    constexpr int CHANNELS = 8;
    constexpr int REG_STEP = 4;
    constexpr std::uint32_t MEMORY_BYTES = 16u * 1024u * 1024u;
    constexpr std::uint32_t MINIMAP_BLOCK = 32u * 1024u; // 32KB
    constexpr std::size_t MINIMAP_SIZE = MEMORY_BYTES / MINIMAP_BLOCK;

    // 24bit アドレスを reg[] から作る小ヘルパ
    inline std::uint32_t build24(const UCHAR *reg, int base, int ch) {
        int idx = base + (REG_STEP * ch);
        return (static_cast<std::uint32_t>(reg[idx]) << 16)
            | (static_cast<std::uint32_t>(reg[idx + REG_STEP]) << 8)
            | static_cast<std::uint32_t>(reg[idx + 2 * REG_STEP]);
    }
}

int c86ctl::CYMZ280BAdpcm::getLevel(UCHAR ch)
{
    return (ch < CHANNELS) ? static_cast<int>(reg[(ch * REG_STEP) + 2]) : 0;
}

UINT c86ctl::CYMZ280BAdpcm::getStartAddr(UCHAR ch)
{
    return (ch < CHANNELS) ? StartAddr[ch] : 0;
}

UINT c86ctl::CYMZ280BAdpcm::getEndAddr(UCHAR ch)
{
    return (ch < CHANNELS) ? EndAddr[ch] : 0;
}

bool CYMZ280BAdpcm::setReg(UCHAR adrs, UCHAR data)
{
    bool handled = true;

    UCHAR ch;
    reg[adrs] = data;

    switch (adrs) {
    case 0x01:
    case 0x05:
    case 0x09:
    case 0x0d:
    case 0x11:
    case 0x15:
    case 0x19:
    case 0x1d:
        ch = (adrs >> 2) & 0x07;
        if (data & 0x80) {    // Key On
            s_adpcmPlayed = true;
            StartAddr[ch] = build24(reg, 0x20, ch);
            EndAddr[ch]   = build24(reg, 0x23, ch);
            keyOnLevel[ch] = reg[(ch * REG_STEP) + 2] >> 3;
            sw[ch] = true;
            if (data & 0x10) {    // Loop enabled
                LoopStartAddr[ch] = build24(reg, 0x21, ch);
                LoopEndAddr[ch]   = build24(reg, 0x22, ch);
            }
        }
        else {
            keyOnLevel[ch] = 0;
            sw[ch] = false;
        }
        break;

    case 0x84:
    case 0x85:
    case 0x86:
        currentAddr = (reg[0x84] << 16) | (reg[0x85] << 8) | reg[0x86];
        break;

    case 0x87:
        if (s_adpcmPlayed) {
            //std::fill_n(minimap, static_cast<std::size_t>(minimapsize), 0);
            if (minimap)    memset(minimap, 0, minimapsize);    //性能を優先
            s_adpcmPlayed = false;
        }
        // 32KB ブロック単位で使用チェック
        if(minimap) minimap[currentAddr >> 15] |= 0x01;
        if (++currentAddr >= MEMORY_BYTES) currentAddr = 0;
        break;

    default:
        handled = false;
    }

    return handled;
}


////////////////////////////////////////////////////////////////////////////////

void CYMZ280B::byteOut(UINT addr, UCHAR data)
{
	int ch;
	if (0x100 <= addr) return;

	switch (addr) {
	case 0x02:
	case 0x06:
	case 0x0a:
	case 0x0e:
	case 0x12:
	case 0x16:
	case 0x1a:
	case 0x1e:
		ch = (addr >> 2) & 0x07;
		if (getMixedMask(ch))	data = 0;
		break;
	}

	if (setReg(addr, data))
		if (ds) ds->byteOut(addr, data);
}

void CYMZ280B::applyMask(int ch)
{
	UCHAR data = 0;
	UINT adrs = 0;

	if (!ds) return;
	if ((ch < 0) || (ch > 7))	return;

	bool mask = getMixedMask(ch);

	data = (mask) ? 0 : adpcm->getLevel(ch);
	adrs = (ch * 4) + 2;	// TL reg
	ds->byteOut(adrs, data);
}


void CYMZ280B::setPartMask(int ch, bool mask)
{
	if (ch < 0 || 8 <= ch) return;

	if (mask) {
		partMask |= 1 << ch;
	}
	else {
		partMask &= ~(1 << ch);
	}
	applyMask(ch);
}

void CYMZ280B::setPartSolo(int ch, bool mask)
{
	if (ch < 0 || 8 <= ch) return;

	if (mask)	partSolo |= 1 << ch;
	else		partSolo &= ~(1 << ch);

	for (int i = 0; i < 8; i++)
		applyMask(i);
}


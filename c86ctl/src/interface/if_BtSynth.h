
#pragma once

#include "if.h"

#include <WinIoCtl.h>
#include <mmsystem.h>
#include <vector>
#include "ringbuff.h"
#include "withlock.h"
#include "chip/chip.h"

namespace c86ctl{


DEFINE_GUID(GUID_DEVINTERFACE_BtSynth,
	0xc5c05439, 0x2b98, 0x46b3, 0x8e, 0x95, 0x7d, 0x23, 0xa2, 0xf5, 0x0e, 0x1c);
// {c5c05439-2b98-46b3-8e95-7d23a2f50e1c}


#define BTSYNTH_IOCTL(_index_) \
    CTL_CODE (FILE_DEVICE_BUS_EXTENDER, _index_, METHOD_BUFFERED, FILE_READ_DATA)

#define IOCTL_BTSYNTH_QUERY_SYSTEM_INFO			    BTSYNTH_IOCTL (0x0F)    //new
#define IOCTL_BTSYNTH_RESET							BTSYNTH_IOCTL (0x10)
#define IOCTL_BTSYNTH_QUERY_BOARD_NAME				BTSYNTH_IOCTL (0x11)
#define IOCTL_BTSYNTH_QUERY_CHIP_COUNT				BTSYNTH_IOCTL (0x12)
#define IOCTL_BTSYNTH_QUERY_CHIP_TYPE				BTSYNTH_IOCTL (0x13)
#define IOCTL_BTSYNTH_QUERY_FW_VERSION				BTSYNTH_IOCTL (0x14)
#define IOCTL_BTSYNTH_QUERY_BOARD_INFO				BTSYNTH_IOCTL (0x15)	//new
#define IOCTL_BTSYNTH_SET_INTERNAL_PARAM			BTSYNTH_IOCTL (0x16)	//new

#define IOCTL_BTSYNTH_WRITEREG_C86CTL				BTSYNTH_IOCTL (0x100)
#define IOCTL_BTSYNTH_WRITEREG_S98					BTSYNTH_IOCTL (0x101)

#define	IOCTL_BTSYNTH_GET_MCLK						BTSYNTH_IOCTL (0x200)
#define	IOCTL_BTSYNTH_SET_MCLK						BTSYNTH_IOCTL (0x201)
#define	IOCTL_BTSYNTH_GET_VOLUME					BTSYNTH_IOCTL (0x202)
#define	IOCTL_BTSYNTH_SET_VOLUME					BTSYNTH_IOCTL (0x203)


class BtSynthBase : public BaseSoundDevice, public IFirmwareVersionInfo
{
public:
	// ---------------------------------------------------------
	class BtSynthModule : public BaseSoundModule
	{
	public:
//		BtSynthModule(BtSynthBase *device, int chipidx, ChipType chipType);	//îpé~
		BtSynthModule(BtSynthBase* device, int slotidx, int chipidx, ChipType chipType);//óÒãìÇc86boxÇ…äÒÇπÇÈ
		virtual ~BtSynthModule();

	public:
		// IByteInput
		// -- before filter
		virtual void byteOut(UINT addr, UCHAR data);

	public:
		virtual void directOut(UINT addr, UCHAR data);

//	public:
		virtual int setPLLClock(UINT clock);

	public:
		// override to BaseSoundModule
		virtual enum ChipType getChipType() { return chiptype; };
		virtual int isValid(void){ return devif->isValid(); };
		//		virtual std::basic_string<TCHAR> getNodeId();

		virtual BaseSoundDevice* getParentDevice(){
			return static_cast<BaseSoundDevice*>(devif);
		};

	public:
		int getSlotIndex() { return slotidx; };
		int getChipIndex() { return chipidx; };

	private:
		BtSynthBase *devif;
		int devidx;
		int slotidx;
		int chipidx;

	private:
		//Chip *chip;
		ChipType chiptype;
	};


	// ---------------------------------------------------------
public:
	static const UINT NMAXCHIPS = 4;	//1moduleÇ†ÇΩÇËÇÃç≈ëÂchipêî
	static const UINT NMAXBOARDS = 8;	//BaseDeviceè„ÇÃç≈ëÂÉÇÉWÉÖÅ[Éãêî
public:
	static int UpdateInstances(withlock< std::vector< std::shared_ptr<BaseSoundDevice> > > &devices);

private:
	BtSynthBase();
	bool OpenDevice(std::basic_string<TCHAR> devpath);

public:
	~BtSynthBase(void);

public:
	// override to BaseSoundDevice
	virtual int reset(void);
	virtual void tick(void);
	virtual void update(void);
	virtual UINT getCPS(void){ return cps; };

	virtual int isValid(void);
	virtual void checkConnection(void);
	//	virtual std::basic_string<TCHAR> getNodeId();

	virtual BaseSoundModule* getModule(int id){
		if (id<0 || id>nmodules)
			return NULL;
		return modules[id];
	};
	virtual int getNumberOfModules(){
		return nmodules;
	}

public:
	virtual int getFWVer(UINT *major, UINT *minor, UINT *rev, UINT *build);
	int getBoardName(char *board_name);	//new
	bool setInternalParameter(ULONG idx, ULONG value);

// private -----------------------------------------------------
private:
	struct BOARD_INFO {
		UINT type;
		UINT nchips;
		UINT chiptype[NMAXCHIPS];
	};


private:
	int sendMsg(UINT *data, UINT size);
	void out(UCHAR idx, UINT addr, UCHAR data);

	int getBoardInfo(int boardIdx, BOARD_INFO* binfo);

	int setPLL(UINT idx, UINT clock);
private:
	HANDLE hDev;
	std::basic_string<TCHAR> devPath;

private:
	CRITICAL_SECTION csection;
	CRingBuff<UINT> rbuff;
	UINT cps, cal, calcount;

	LARGE_INTEGER freq;

private:
	static const int NMAXCHIP = (NMAXCHIPS * NMAXBOARDS);
	int nmodules;
	BtSynthModule *modules[NMAXCHIP];

protected:
	int refcount;


};

typedef std::shared_ptr<BtSynthBase> C86PciPtr;



};

#include <tamtypes.h>
#include <errno.h>
#include <sbv_patches.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <fileio.h>
#include <libmc.h>
#include <stdio.h>
#include <string.h>
#include "libpad.h"
#include <debug.h>
//#include "libmtap.h"
#include <libpwroff.h>
#include <iopcontrol.h>
#include <iopheap.h>


//PAD VARIABLES
//check for multiple definitions
#define DEBUG

#if !defined(ROM_PADMAN) && !defined(NEW_PADMAN)
#define ROM_PADMAN
#endif

#if defined(ROM_PADMAN) && defined(NEW_PADMAN)
#error Only one of ROM_PADMAN & NEW_PADMAN should be defined!
#endif

#if !defined(ROM_PADMAN) && !defined(NEW_PADMAN)
#error ROM_PADMAN or NEW_PADMAN must be defined!
#endif


extern unsigned char poweroff_irx;
extern unsigned int size_poweroff_irx;

//pad buffer
static char padBuf[256] __attribute__((aligned(64)));
//rumblers
static char actAlign[6];
static int actuators;
//button status
struct padButtonStatus buttons;
u32 paddata;
u32 old_pad;
u32 new_pad;
int port, slot;
extern void readPad(void);

void LoadModules(void);
void initialize(void);
int LoadIRX();

#define TYPE_XMC
static int mc_Type, mc_Free, mc_Format;

//Strings
	char *appName = "Mass Format Utility ";
	char *appVer = "Version 0.5 ";
	char *appAuthor = "Created By: 1UP & Based_Skid. Copyright \xa9 2018\n";
	char *help = "Special thanks to SP193 for all the help! \n";
	char *appNotice = "Notice: This May Not be Compatible With all PS2 Models!\n";
	char *txtselectBtn = "-Press SELECT to view Memory Card Information.\n";
	char *txtstartBtn = "-Press START to Format and Erase All Connected Memory Cards.\n";
	char *txttriangleBtn = "-Press TRIANGLE to Refresh Status and Clear Output.\n";
	char *txtsqrBtn = "-Press Square to Poweroff the console.\n";
	char *txtcrossBtn = "-Press X to Exit and Reboot.\n";
	char *osdmsg = "Exiting to OSDSYS\n";

int main(int argc, char *argv[]) {

	ResetIOP();
	int ret;
	
	// initialize
	initialize();
	// "Load IRX Modules"
	LoadIRX();
	menu_Text();

	if (mcInit(MC_TYPE_XMC) < 0) 
	{
		printf("Failed to Init libmc\n");
		gotoOSDSYS(1);
	}

	while (1)
	{
		//check to see if the pad is still connected
		checkPadConnected();
		//read pad 0
		buttonStatts(0, 0);

		if (new_pad & PAD_TRIANGLE)
		{
			menu_Text();
		}

		if (new_pad & PAD_SELECT)
		{
			//List all memory cards and show how many KB is free
			memoryCardCheckAndFormat(0);

		}

		if (new_pad & PAD_START)
		{
			//Format all memorycards
			memoryCardCheckAndFormat(1);
		}
		
		if(new_pad & PAD_CROSS)
		{
			scr_clear();
			scr_printf(appName);
			scr_printf(" \n");
			gotoOSDSYS();
		}
		
		if(new_pad & PAD_SQUARE) 
		{
			// Initialize Poweroff Library
			poweroffInit();
			// Power Off PS2
			poweroffShutdown();
		}
	}
	return 0;
}

void menu_Text(void)
{
	scr_clear();
	scr_printf(appName);
	scr_printf(appVer);
	scr_printf(appAuthor);
	scr_printf(help);
	scr_printf(appNotice);
	scr_printf(txtselectBtn);
	scr_printf(txtstartBtn);
	scr_printf(txttriangleBtn);
	scr_printf(txtcrossBtn);
	scr_printf(txtsqrBtn);
	scr_printf(" \n");
	mtapConnect();
}

void initialize(void)
{

	int ret;

	SifInitRpc(0);
	// init debug screen
	init_scr();
	// load all modules
	LoadModules();
	// Initialize The Multitap Library
	mtapInit();
	

	// init pad
	padInit(0);
	if ((ret = padPortOpen(0, 0, padBuf)) == 0)
	{
		#if defined DEBUG
			scr_printf("padOpenPort failed: %d\n", ret);
		#endif
		SleepThread();
	}

	if (!initializePad(0, 0))
	{
		#if defined DEBUG
			scr_printf("pad initalization failed!\n");
		#endif
		SleepThread();

	}
}

int LoadIRX()
{
	int a;
	sbv_patch_enable_lmb();
	sbv_patch_disable_prefix_check();
	printf(" Loading IRX!\n");

	a = SifExecModuleBuffer(&poweroff_irx, size_poweroff_irx, 0, NULL, &a);
	if (a < 0 )
	{
        scr_printf(" Could not load POWEROFF.IRX! %d\n", a);
	return -1;
	}

	printf(" Loaded POWEROFF.IRX!\n");
	return 0;

}

void LoadModules(void)
{
	int ret;
	
	ret = SifLoadModule("rom0:XSIO2MAN", 0, NULL);
	if (ret < 0) {
		printf("Failed to load module: XSIO2MAN");
		gotoOSDSYS(1);
	}
		
	ret = SifLoadModule("rom0:XMTAPMAN", 0, NULL);
	if (ret < 0) {
		printf("Failed to load module: XMTAPMAN");
		gotoOSDSYS(1);
	}
	
	ret = SifLoadModule("rom0:XPADMAN", 0, NULL);
	if (ret < 0) {
		printf("Failed to load module: XPADMAN");
		gotoOSDSYS(1);
	}
	ret = SifLoadModule("rom0:XMCMAN", 0, NULL);
	if (ret < 0) {
		printf("Failed to load module: XMCMAN");
		gotoOSDSYS(1);
	}

	ret = SifLoadModule("rom0:XMCSERV", 0, NULL);
	if (ret < 0) {
		printf("Failed to load module: XMCSERV");
		gotoOSDSYS(1);
	}
	
}

int memoryCardCheckAndFormat(int format)
{
	scr_clear();
	
	int rv,portNumber,slotNumber,ret;
	rv = mtapGetConnection(2);
	if(rv == 1) 
	{
		for (portNumber =0; portNumber <1; portNumber++)
		{
			for (slotNumber =0; slotNumber<4; slotNumber++)
			{
				mcGetInfo(portNumber, slotNumber, &mc_Type, &mc_Free, &mc_Format);
				mcSync(0, NULL, &ret);
				if (ret >= -10)
				{
					if (format == 0)
					{
						scr_printf("Memory Card Port %d Slot %d detected! %d kb Free\n\n", portNumber,slotNumber,mc_Free);
					}
					if (format == 1)
					{
						scr_printf("Formatting Memory Card Port %d Slot %d.\n", portNumber,slotNumber);
						mcFormat(portNumber, slotNumber);
						mcSync(0, NULL, &ret);
						if (ret == 0)
						{
							scr_printf("Memory Card Port %d Slot %d formatted!\n\n", portNumber,slotNumber);
						}
						else
						{
							scr_printf("Memory Card Port %d Slot %d failed to format!\n\n", portNumber,slotNumber);
		
						}
					}
				}
				else 
				{
					scr_printf("Memory Card Port %d Slot %d not detected!\n\n", portNumber,slotNumber);
				}			
			}
		}
	}
	else
	{
		scr_printf("Memory Card Port 0: Multi-tap is Not Connected. Only Using Slot 0. \n");
		mcGetInfo(0, 0, &mc_Type, &mc_Free, &mc_Format);
		mcSync(0, NULL, &ret);
		if (ret >= -10)
		{
			if (format == 0)
			{
				scr_printf("Memory Card In Port 0 detected! %d kb Free\n\n",mc_Free);
			}
			if (format == 1)
			{
				scr_printf("Formatting Memory Card In Port 0 \n");
				mcFormat(0, 0);
				mcSync(0, NULL, &ret);
				if (ret == 0)
				{
					scr_printf("Memory Card In Port 0 Formatted!\n\n");
				}
				else
				{
					scr_printf("Memory Card In Port 0 failed to format!\n\n");
				}
			}
		}
		else
		{
			scr_printf("Memory Card Port 0 not detected!\n\n");
		}
	}
	//Logical Port 2
	rv = mtapGetConnection(3);
	if(rv == 1) 
	{
		for (portNumber =1; portNumber <2; portNumber++)
		{
			for (slotNumber =0; slotNumber<4; slotNumber++)
			{
				mcGetInfo(portNumber, slotNumber, &mc_Type, &mc_Free, &mc_Format);
				mcSync(0, NULL, &ret);
				if (ret >= -10)
				{
					if (format == 0)
					{
						scr_printf("Memory Card Port %d Slot %d detected! %d kb Free\n\n", portNumber,slotNumber,mc_Free);
					}
					if (format == 1)
					{
						scr_printf("Formatting Memory Card Port %d Slot %d.\n", portNumber,slotNumber);
						mcFormat(portNumber, slotNumber);
						mcSync(0, NULL, &ret);
						if (ret == 0)
						{
							scr_printf("Memory Card Port %d Slot %d formatted!\n\n", portNumber,slotNumber);
						}
						else
						{
							scr_printf("Memory Card Port %d Slot %d failed to format!\n\n", portNumber,slotNumber);
		
						}
					}
				}
				else 
				{
					scr_printf("Memory Card Port %d Slot %d not detected!\n\n", portNumber,slotNumber);
				}			
			}
		}
	}
	else
	{
		scr_printf("Memory Card Port 1: Multi-tap is Not Connected. Only Using Slot 0.\n");
		mcGetInfo(1, 0, &mc_Type, &mc_Free, &mc_Format);
		mcSync(0, NULL, &ret);
		if (ret >= -10)
		{
			if (format == 0)
			{
				scr_printf("Memory Card In Port 1 detected! %d kb Free\n\n",mc_Free);
			}
			if (format == 1)
			{
				scr_printf("Formatting Memory Card In Port 1 \n");
				mcFormat(1, 0);
				mcSync(0, NULL, &ret);
				if (ret == 0)
				{
					scr_printf("Memory Card In Port 1 Formatted!\n\n");
				}
				else
				{
					scr_printf("Memory Card In Port 1 failed to format!\n\n");
				}
			}
		}
		else
		{
			scr_printf("Memory Card In Port 1 not detected!\n\n");
		}
	}
	scr_printf(txttriangleBtn);
	return 0;
}
/////////////////////////////////////////////////////////////////////
//waitPadReady
/////////////////////////////////////////////////////////////////////
static int waitPadReady(int port, int slot)
{
	int state;
	int lastState;
	char stateString[16];

	state = padGetState(port, slot);
	lastState = -1;
	while ((state != PAD_STATE_STABLE) && (state != PAD_STATE_FINDCTP1)) {
		if (state != lastState) {
			padStateInt2String(state, stateString);
		}
		lastState = state;
		state = padGetState(port, slot);
	}
	// Were the pad ever 'out of sync'?
	if (lastState != -1) {

	}
	return 0;
}



/////////////////////////////////////////////////////////////////////
//initalizePad
/////////////////////////////////////////////////////////////////////
static int initializePad(int port, int slot)
{

	int ret;
	int modes;
	int i;

	waitPadReady(port, slot);
	modes = padInfoMode(port, slot, PAD_MODETABLE, -1);
	if (modes > 0) {
		for (i = 0; i < modes; i++) {
		}

	}
	if (modes == 0) {
		return 1;
	}

	i = 0;
	do {
		if (padInfoMode(port, slot, PAD_MODETABLE, i) == PAD_TYPE_DUALSHOCK)
			break;
		i++;
	} while (i < modes);
	if (i >= modes) {
		return 1;
	}

	ret = padInfoMode(port, slot, PAD_MODECUREXID, 0);
	if (ret == 0) {
		return 1;
	}
	padSetMainMode(port, slot, PAD_MMODE_DUALSHOCK, PAD_MMODE_LOCK);

	waitPadReady(port, slot);
	padInfoPressMode(port, slot);

	waitPadReady(port, slot);
	padEnterPressMode(port, slot);

	waitPadReady(port, slot);
	actuators = padInfoAct(port, slot, -1, 0);

	if (actuators != 0) {
		actAlign[0] = 0;
		actAlign[1] = 1;
		actAlign[2] = 0xff;
		actAlign[3] = 0xff;
		actAlign[4] = 0xff;
		actAlign[5] = 0xff;

		waitPadReady(port, slot);

		padSetActAlign(port, slot, actAlign);
	}
	else {
	}
	return 1;
}

/////////////////////////////////////////////////////////////////////
//buttonStatts
/////////////////////////////////////////////////////////////////////
static void buttonStatts(int port, int slot)
{
	int ret;
	ret = padRead(port, slot, &buttons);

	if (ret != 0) {
		paddata = 0xffff ^ buttons.btns;

		new_pad = paddata & ~old_pad;
		old_pad = paddata;
	}
}

/////////////////////////////////////////////////////////////////////
//checkPadConnected
/////////////////////////////////////////////////////////////////////
void checkPadConnected(void)
{
	int ret, i;
	ret = padGetState(0, 0);
	while ((ret != PAD_STATE_STABLE) && (ret != PAD_STATE_FINDCTP1)) {
		if (ret == PAD_STATE_DISCONN) {
			#if defined DEBUG
				scr_printf("Controller(%d, %d) is disconnected\n", 0, 0);
			#endif
		}
		ret = padGetState(0, 0);
	}
	if (i == 1) {
	}
}

/////////////////////////////////////////////////////////////////////
//pad_wat_button
/////////////////////////////////////////////////////////////////////
void pad_wait_button(u32 button)
{
	while (1)
	{
		buttonStatts(0, 0);
		if (new_pad & button) return;
	}
}

void ResetIOP()
{
	// Thanks To SP193 For Clarifying This
	SifInitRpc(0);           //Initialize SIFRPC and SIFCMD. Although seemingly unimportant, this will update the addresses on the EE, which can prevent a crash from happening around the IOP reboot.
	SifIopReset("", 0);      //Reboot IOP with default modules (empty command line)
	while(!SifIopSync()){}   //Wait for IOP to finish rebooting.
	SifInitRpc(0);           //Initialize SIFRPC and SIFCMD.
	SifLoadFileInit();       //Initialize LOADFILE RPC.
	fioInit();               //Initialize FILEIO RPC.
}


void mtapConnect()
//////////////////////////////////////////////////////////////////////
//                        Multi-tap Code                           //
////////////////////////////////////////////////////////////////////
/*

MTAP Ports Shouldnt Be Confused With Controller or MC ports
Meaning that Mtap Port 2 is Still Memory Card Port 0 (Logical Port 1)

Logical Port Refers to the Actual Port on the PS2. (Ports 1 and 2)

The ASCII Reference Art Below Is taken From the PS2DEV Multitap Library Sample AND SOME INFO HAS BEEN ADDED/CORRECTED.
(The Sample had D and C backwards)
See https://github.com/ps2dev/ps2sdk/tree/master/ee/rpc/multitap For More Info
						
				     __________[ Port 1, Slot 3 ]
                                    /    _____[ Port 1, Slot 2 ]
 |------------|                     |   /
 |            |                   |-------|
 |            |                   | D   C |
 |     PS2    |          |--------|       | <- Multi-tap
 |            |          |        | A   B |
 |            |          |        |-------|
 |            |          |          |   \_____ [ Port 1, Slot 1 ]
 |LogicalPort 2 |]---------|          \__________[ Port 1, Slot 0 ]
 |            |
 |LogicalPort 1 |]---[ Port 0, Slot 0 ]
 |            |
 |------------|
Some More Notes

ACCORING TO THE MULTITAP SAMPLE YOU MUST USE XMODULES IN ORDER TO USE THE MULTITAP!

====Mtap Port Info====
You DONT have to Open Mulitap Ports 1 & 2 if you are just looking to access the Memory Cards. If you Wanted to Use Controller slots B,C,D on a Multi-tap Then you would need to open the Port
mtapPortOpen(0); >> Memory Card Port 1 (Logical Controller Port 1)
mtapPortOpen(1); >> Memory Card Port 2 (Logical Controller Port 2)
mtapPortOpen(2); >> Memory Card Port 1 (Logical MC Port 1)
mtapPortOpen(3); >> Memory Card Port 2 (Logical MC Port 2)
======================

====Port,Slot Info====
 1A: PORT = 0,SLOT = 0
 1B: PORT = 0,SLOT = 1
 1C: PORT = 0,SLOT = 2
 1D: PORT = 0,SLOT = 3
 
 2A: PORT = 1,SLOT = 0
 2B: PORT = 1,SLOT = 1
 2C: PORT = 1,SLOT = 2
 2D: PORT = 1,SLOT = 3
====================== 
 */
{
	closeMTAPports(); //In Case Multi-tap(s) are Connected or removed After the App has been started.
	int rv;
	mtapPortOpen(2);
	mtapPortOpen(3);

	scr_printf("Multi-tap Status: \n");
	//Checks For Mtap Connection on Physical Memory Card Slot 1
	rv = mtapGetConnection(2); // Checks MTAP port 2 (Memory Card Port 1) For MTAP Connection
	
	if(rv == 1)
	{
		scr_printf("Memory Card Port 0: Multi-tap Detected! \n");
	}
	else
    {
		scr_printf("Memory Card Port 0: Multi-tap is Not Connected. \n");
		mtapPortClose(2); //Closes The Multitap Port if The Multitap Is Not Present
    }
	
	
	//Checks For Mtap Connection on Physical Memory Card Slot 2
	rv = mtapGetConnection(3); // Checks MTAP port 3 (Memory Card Port 2) For MTAP Connection
    
	if(rv == 1)
	{
		scr_printf("Memory Card Port 1: Multi-tap Detected! \n");
	}
	else
    {
		scr_printf("Memory Card Port 1: Multi-tap is Not Connected. \n");
		mtapPortClose(3);
	}
}
// Closes MTAP Ports 2 and 3
void closeMTAPports()
{
	mtapPortClose(2);
	mtapPortClose(3);
}
void gotoOSDSYS(int failure)
{
	if (failure == 1)
	{
		scr_printf("An Application Failure Has Occurred.\n");
	}
	ResetIOP();
	scr_printf(osdmsg);
	sleep(3);
	LoadExecPS2("rom0:OSDSYS", 0, NULL);

}

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
#include <libpwroff.h>
#include <iopcontrol.h>
#include <iopheap.h>
#include "libmtap.h"

extern void freesio2;
extern void freepad;
extern void poweroff;
extern void mtapman;
extern void mcman;
extern void mcserv;

extern u32 size_poweroff;
extern u32 size_freesio2;
extern u32 size_freepad;
extern u32 size_mtapman;
extern u32 size_mcman;
extern u32 size_mcserv;
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
	char *appVer = "Version 1.0 ";
	char *appAuthor = "Created By: 1UP & Based_Skid. Copyright \xa9 2018\n";
	char *help = "Special thanks to SP193 for all the help! \n";
	char *appNotice = "Notice: This May Not be Compatible With all PS2 Models!\n";
	char *txtselectBtn = "-Press SELECT to view Memory Card Information.\n";
	char *txtstartBtn = "-Press START to Format and Erase All Connected Memory Cards.\n";
	char *txttriangleBtn = "-Press TRIANGLE to Refresh Status and Clear Output.\n";
	char *txtsqrBtn = "-Press Square to Poweroff the console.\n";
	char *txtcrossBtn = "-Press X to Exit and Reboot.\n";
	char *osdmsg = "Exiting to OSDSYS\n";
	char *appFail = "Application Failure!\n";
	char *modloadfail = "Failed to load module: ";
	
	

int main(int argc, char *argv[]) {

	ResetIOP();
	
	// initialize
	initialize();
	// "Load IRX Modules"
	LoadIRX();
	menu_Text();

	if (mcInit(MC_TYPE_XMC) < 0) 
	{
		gotoOSDSYS(6);
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
			gotoOSDSYS(0);
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
	mtapDetect();
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
	printf(" Loading IRX!\n");

	a = SifExecModuleBuffer(&poweroff, size_poweroff, 0, NULL, &a);
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
	
	ret = SifExecModuleBuffer(&freesio2, size_freesio2, 0, NULL, &ret); //XSIO2MAN SW Replacment module aka freesio2
	if (ret < 0) 
	{
		printf("Failed to Load freesio2 sw module");
		ret = SifLoadModule("rom0:XSIO2MAN", 0, NULL); // The Xmodule Code Shown in this function is for reference
		if (ret < 0) 
		{
			gotoOSDSYS(1);
		}
	}
			
	
	ret = SifExecModuleBuffer(&mtapman, size_mtapman, 0, NULL, &ret); // XMTAPMAN SW Replacement Module aka freeMTAP
	if (ret < 0) 
	{
		printf("Failed to Load freeMTAP sw module");
		ret = SifLoadModule("rom0:XMTAPMAN", 0, NULL); // Ref
		if (ret < 0) 
		{
			gotoOSDSYS(2);
		}
	}
	

	// You Can Use Freepad If you Need Controller Support in Your Application And Dont Require Using Controller Slots 1,2,3 (B,C,D) on The Multiap
	// Refer to The Multitap Sample Below (line 584) if You need to use the Controller Slots 1,2,3 (B,C,D)

	// Note: You Might be able to use Freepad with The multitap slots 1,2,3 but I have Never Tested This
	ret = SifExecModuleBuffer(&freepad, size_freepad, 0, NULL, &ret); // XPADMAN SW Replacement Module aka freepad 
	if (ret < 0) 
	{
		printf("Failed to Load freepad sw module");
		ret = SifLoadModule("rom0:XPADMAN", 0, NULL); //Ref
		if (ret < 0) 
		{
			gotoOSDSYS(3);
		}
	}

	//The Memory Card Modules Are Only Required If you Intend to Access Slots 1,2,3 (B,C,D) on the Multitap
	// Note Make your you Have libmc.h included
	ret = SifExecModuleBuffer(&mcman, size_mcman, 0, NULL, &ret); // XMCMAN SW Replacment Module
	if (ret < 0) 
	{
		printf("Failed to Load mcman sw module");
		ret = SifLoadModule("rom0:XMCMAN", 0, NULL);
		if (ret < 0) 
		{
			gotoOSDSYS(4);
		}
	}
	
	ret = SifExecModuleBuffer(&mcserv, size_mcserv, 0, NULL, &ret); // XMCSERV SW Replacment Module
	if (ret < 0) 
	{
		printf("Failed to Load mcserv sw module");
		ret = SifLoadModule("rom0:XMCSERV", 0, NULL);
		if (ret < 0) 
		{
			gotoOSDSYS(5);
		}
	}
 
	}
	


int memoryCardCheckAndFormat(int format)
{
	mtapDetect();
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
	
	// Port 2
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
	//SBV Patches
	sbv_patch_enable_lmb();
	sbv_patch_disable_prefix_check();
}


//////////////////////////////////////////////////////////////////////
//                        Multi-tap Code                           //
////////////////////////////////////////////////////////////////////
/*
This Portion Should Serve as Either A supplemental piece of information and/or correction to the original Sample Code
Located Here: https://github.com/ps2dev/ps2sdk/tree/master/ee/rpc/multitap
Multitap Sample Code Updated and Corrected by Based_Skid

//
A Few Things To Note: 
The Original Sample Has the C & D Slot Numbers Incorrectly Listed
The Original Sample Does Not Indicate that Mtap Ports 2 & 3 Exist
//

MTAP Ports Shouldnt Be Confused With Controller or MC ports
Meaning that Mtap Port 2 is Still Memory Card Port 0 (Logical Port 1)

Logical Port Refers to the Actual Port on the PS2. (Ports 1 and 2)

Im using the Term Logical port to describe because of the way Ports and Slots are Numbered Regarding Controllers and Memory Cards, and to avoid confusion

The ASCII Reference Art Below Is taken From the PS2DEV Multitap Library Sample AND SOME INFO HAS BEEN ADDED/CORRECTED.
(The Sample had D and C backwards)
						
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
Some More Info
---
ACCORING TO THE Original MULTITAP SAMPLE it Declares that YOU MUST USE XMODULES IN ORDER TO USE THE MULTITAP, 
Please Be Advised That You Can Use The Software IRX modules with the Multitap and the Board Secific Xmodules are not Required.
The Software Modules Are Meant to Be Drop in Replacments for The XModules!
See The LoadModule Function in This File and Check the Makefile to See How Software Modules Can be Used in Your Project.
---
===
A Quick Note about XMODULES: You Should NEVER Use XMODULES in your Own Applications
X Modules  are Board Specific and Compatibility Across all PS2 Models is Not Guarenteed. 
This Application uses the Software module (freemtap.irx) for the mutltitap module.
All of The Modules Loaded in this Application Are The Software Replacement Modules provided by the ps2sdk
===

====Mtap Port Info====
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//Controller Ports
mtapPortOpen(0); >> Memory Card Port 1 (Logical Controller Port 1)
mtapPortOpen(1); >> Memory Card Port 2 (Logical Controller Port 2)
//Memory Card Ports
mtapPortOpen(2); >> Memory Card Port 1 (Logical MC Port 1)
mtapPortOpen(3); >> Memory Card Port 2 (Logical MC Port 2)
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
You DONT have to Open Mulitap Ports 1 & 2 if you are just looking to access the Memory Cards. 
--
Libmc Works with Cards that are connected to the multitap. 
There are Some Code Examples Below to Demonstrate The Port,Slot number use
See LibMC sample for more information about working with memory cards: https://github.com/ps2dev/ps2sdk/blob/master/ee/rpc/memorycard/samples/mc_example.c#L78 
The Calls Below are modified slightly from the Libmc example and are to Demonstrate how the port and slot are targeted.
--
==
mcGetInfo(0, 0, &mc_Type, &mc_Free, &mc_Format); << This would Target the 1A Port on the Multitap (port 0, slot 0)

mcGetInfo(0, 1, &mc_Type, &mc_Free, &mc_Format); << 1B Port

mcGetInfo(0, 2, &mc_Type, &mc_Free, &mc_Format); << 1B Port

mcGetInfo(0, 3, &mc_Type, &mc_Free, &mc_Format); << 1B Port

mcGetInfo(1, 0, &mc_Type, &mc_Free, &mc_Format); << 2A Port

mcGetInfo(1, 1, &mc_Type, &mc_Free, &mc_Format); << 2B Port

mcGetInfo(1, 2, &mc_Type, &mc_Free, &mc_Format); << 2C Port

mcGetInfo(1, 3, &mc_Type, &mc_Free, &mc_Format); << 2D Port
==
--

--
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
If you Wanted to Use Controller slots B,C,D on a Multi-tap 
Then you would need to open the Mtap port that corresponds with the controller port

Example code (Modified From Original Sample Provided)
------------

 1. Loading the required IRX files
	The X Module Code Below is For Reference to the old Sample and should not be used.
    	See Line 213 to See The Required IRX Modules and some Code to load them. (Also see the MakeFile)

    // NOTE: X** modules must be used! <<< NOT TRUE!
    //SifLoadModule("rom0:XSIO2MAN", 0, NULL);
    //SifLoadModule("rom0:XMTAPMAN", 0, NULL);
    //SifLoadModule("rom0:XPADMAN", 0, NULL);
   
 2. Init, opening ports

    // Init libmtap
    mtapInit();

    // Init libpad
    padInit(0);

    // Open ports
    mtapPortOpen(0);
    mtapPortOpen(1);

 3. Check connections

    rv = mtapGetConnection(0);
    if(rv == 1)
        printf("- Multitap exists on slot 0\n");
    else
    {
        printf("- No multitap exists on slot 0\n");
        mtapPortClose(0);
    }

    rv = mtapGetConnection(1);
    if(rv == 1)
        printf("- Multitap exists on slot 1\n");
    else
    {
        printf("- No multitap exists on slot 1\n");
        mtapPortClose(1);
    }

 4. Open the controllers

    padPortOpen(0, 0, padBuf1A);
    padPortOpen(0, 1, padBuf1B);
    padPortOpen(0, 2, padBuf1C);
    padPortOpen(0, 3, padBuf1D);

 */
 
// Detects If Multi-tap is Connected. Closes Multi-tap Port(s) if Multi-tap(s) are not Connected
void mtapDetect()
{
    int mt;
    scr_printf("Multi-tap Status: \n");
    mtapPortOpen(2); // Checks MTAP port 2 (Memory Card Port 1) For MTAP Connection
    mt = mtapGetConnection(2);
    if(mt == 1)
    {
        scr_printf("Memory Card Port 0: Multi-tap Detected! \n");
    }
    else
    {
        scr_printf("Memory Card Port 0: Multi-tap is Not Connected. \n");
        mtapPortClose(2); //Closes The Multitap Port if The Multitap Is Not Connected
    }
    
    mtapPortOpen(3);
    mt = mtapGetConnection(3); // Checks MTAP port 3 (Memory Card Port 2) For MTAP Connection
    if(mt == 1)
    {
        scr_printf("Memory Card Port 1: Multi-tap Detected! \n");
    }
    else
    {
        scr_printf("Memory Card Port 1: Multi-tap is Not Connected. \n");
        mtapPortClose(3); //Closes The Multitap Port if The Multitap Is Not Connected
    }
}

void gotoOSDSYS(int sc)
{
	if (sc != 0)
	{
		scr_printf(appFail);
		if(sc ==1 || sc ==2 || sc ==3 || sc ==4 || sc ==5 || sc ==6)
		{
			scr_printf(modloadfail);
		}
		if (sc == 1)
		{
			scr_printf("XSIO2MAN\n");
		}
		if (sc == 2)
		{
			scr_printf("XMTAPMAN\n");
		}
		if (sc == 3)
		{
			scr_printf("XPADMAN\n");
		}
		if (sc == 4)
		{
			scr_printf("XMCMAN\n");
		}
		if (sc == 5)
		{
			scr_printf("XMCSERV\n");
		}
		if (sc == 6)
		{
			scr_printf("Failed to Init libmc\n");
		}
		sleep(5);
	}
	ResetIOP();
	scr_printf(osdmsg);
	LoadExecPS2("rom0:OSDSYS", 0, NULL);
}

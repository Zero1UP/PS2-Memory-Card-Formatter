#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <fileio.h>
#include <libmc.h>
#include <stdio.h>
#include <string.h>
#include "libpad.h"
#include <debug.h>


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
void initalise(void);

#define TYPE_MC
#define TYPE_XMC

static int mc_Type, mc_Free, mc_Format;
	char *origappName = "Memory Card Formatter v.01 by 1UP\n";
	char *appName = "Mass Format Utility v.01 by 1UP\n";
	char *txtselectBtn = "Press SELECT to view information on the cards inserted.\n";
	char *txtstartBtn = "Press START to Format all cards inserted.\n";
	char *txttriangleBtn = "Press TRIANGLE to come back to the main screen.\n";
	char *txtcrossBtn = "Press The Cross Button To Return to OSDSYS.\n";
int main(int argc, char *argv[]) {

	int ret;

	// Initialise
	initalise();
	main_Text();



	#ifdef TYPE_MC
		if (mcInit(MC_TYPE_MC) < 0) 
		{
			printf("Failed to initialise memcard server!\n");
			SleepThread();
		}
	#else
		if (mcInit(MC_TYPE_XMC) < 0) 
		{
			printf("Failed to initialise memcard server!\n");
			SleepThread();
		}
	#endif

	while (1)
	{
		//check to see if the pad is still connected
		checkPadConnected();
		//read pad 0
		buttonStatts(0, 0);

		if (new_pad & PAD_TRIANGLE)
		{
			scr_clear();
			main_Text();
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
			scr_printf(origappName);
			gotoOSDSYS();
		}

	}

	return 0;
}

void main_Text(void)
{
	scr_printf(origappName);
	scr_printf(txtselectBtn);
	scr_printf(txtstartBtn);
	scr_printf(txttriangleBtn);
	scr_printf(txtcrossBtn);
}

void initalise(void)
{

	int ret;

	SifInitRpc(0);
	// init debug screen
	init_scr();
	// load all modules
	LoadModules();

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



void LoadModules(void)
{
	int ret;

#ifdef TYPE_MC
	ret = SifLoadModule("rom0:SIO2MAN", 0, NULL);
	if (ret < 0) {
		printf("Failed to load module: SIO2MAN");
		SleepThread();
	}

	ret = SifLoadModule("rom0:MCMAN", 0, NULL);
	if (ret < 0) {
		printf("Failed to load module: MCMAN");
		SleepThread();
	}

	ret = SifLoadModule("rom0:MCSERV", 0, NULL);
	if (ret < 0) {
		printf("Failed to load module: MCSERV");
		SleepThread();
	}
#else
	ret = SifLoadModule("rom0:XSIO2MAN", 0, NULL);
	if (ret < 0) {
		printf("Failed to load module: SIO2MAN");
		SleepThread();
	}

	ret = SifLoadModule("rom0:XMCMAN", 0, NULL);
	if (ret < 0) {
		printf("Failed to load module: MCMAN");
		SleepThread();
	}

	ret = SifLoadModule("rom0:XMCSERV", 0, NULL);
	if (ret < 0) {
		printf("Failed to load module: MCSERV");
		SleepThread();
	}
#endif

#ifdef ROM_PADMAN
	ret = SifLoadModule("rom0:PADMAN", 0, NULL);
#else
	ret = SifLoadModule("rom0:XPADMAN", 0, NULL);
#endif
	if (ret < 0) {
		SleepThread();
	}
}

int memoryCardCheckAndFormat(int format)
{
	scr_clear();
	scr_printf("The following cards are detected:\n\n");

	int portNumber,ret;
	for (portNumber = 0; portNumber < 2; portNumber += 1)
	{
		mcGetInfo(portNumber, 0, &mc_Type, &mc_Free, &mc_Format);
		mcSync(0, NULL, &ret);

		if (ret >= -10)
		{
			scr_printf("Memory Card %d detected!\n", portNumber);
			scr_printf("Memory Card %d %d kb free!\n\n", portNumber, mc_Free);

			if (format == 1)
			{
				scr_printf("Formatting Memory Card %d.\n", portNumber);
				mcFormat(portNumber, 0);
				mcSync(0, NULL, &ret);

				if (ret == 0)
				{
					scr_printf("Memory Card %d formatted!\n\n", portNumber);
				}
				else
				{
					scr_printf("Memory Card %d failed to format!\n\n", portNumber);

				}
			}
		}
		else {
			scr_printf("Memory Card %d not detected!\n\n", portNumber);
		}
	}
	main_Text();
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
		//printf("Did not find any actuators.\n");
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
			scr_printf("	Pad(%d, %d) is disconnected\n", 0, 0);
#endif
		}
		ret = padGetState(0, 0);
	}
	if (i == 1) {
		//scr_printf("	Pad: OK!\n");
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

void gotoOSDSYS()
{
	ResetIOP();
	scr_printf("Exiting To OSDSYS..\n");
	sleep(3);
	SifLoadFileInit(); 
	SifLoadModule("rom0:SIO2MAN", 0, NULL);
	SifLoadModule("rom0:MCMAN", 0, NULL);
	SifLoadModule("rom0:MCSERV", 0, NULL);
	LoadExecPS2("rom0:OSDSYS", 0, NULL);
	
}

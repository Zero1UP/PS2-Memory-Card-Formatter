/*
			Mass Format Utility
	    Written By 1UP & Based Skid
		Licensed Under AFL 3.0
			Copyright 2018 
*/
#include "main.h" 
// App Strings
#include "strings.h"
//MtapHelper
#include "mtaphelper.h"

void menu_header(void)
{
	scr_printf(appName);
	scr_printf(appVer);
	scr_printf(appAuthor);
	//scr_printf(appNotice);
}

void menu_Text(void)
{
	scr_clear();
	menu_header();
	scr_printf(txtselectBtn);
	scr_printf(txtstartBtn);
	scr_printf(txttriangleBtn);
	scr_printf(txtcrossBtn);
	scr_printf(txtsqrBtn);
	scr_printf(" \n");
	scr_printf("Multi-tap Status: \n");
	mtGO();
}

void initialize(void)
{
	int ret;
	SifInitRpc(0);
	scr_clear();
	// init debug screen
	init_scr();
	scr_clear();
	menu_header();
	scr_printf("Loading... Please Wait. \n");
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
	int lm;

	
	lm = SifExecModuleBuffer(&freesio2, size_freesio2, 0, NULL, NULL);
	if (lm < 0) 
	{
		printf("Failed to Load freesio2 Module");
		gotoOSDSYS(1);
	}	
	
	lm = SifExecModuleBuffer(&mtapman, size_mtapman, 0, NULL, NULL);
	if (lm < 0) 
	{
		printf("Failed to Load MTAPMAN Module");
		gotoOSDSYS(18);
	}
	
	lm = SifExecModuleBuffer(&freepad, size_freepad, 0, NULL, NULL);
	if (lm < 0) 
	{
		printf("Failed to Load freepad Module");
		gotoOSDSYS(3);
	}
	
	lm = SifExecModuleBuffer(&mcman, size_mcman, 0, NULL, NULL);
	if (lm < 0) 
	{
		printf("Failed to Load mcman Module");
		gotoOSDSYS(4);
	}
	
	lm = SifExecModuleBuffer(&mcserv, size_mcserv, 0, NULL, NULL);
	if (lm < 0) 
	{
		printf("Failed to Load mcserv Module");
		gotoOSDSYS(5);
	}
		
	lm = SifExecModuleBuffer(&poweroff, size_poweroff, 0, NULL, NULL);
	if (lm < 0) 
	{
		printf("Failed to Load Poweroff module");
		gotoOSDSYS(17);
	}
	//init Multitap
	mtapInit();
	//init MC\n
	mcInit(MC_TYPE_XMC);
	}

int memoryCardCheckAndFormat(int format)
{
	mtGO(); // Check Multitap Status Before We do Anything Otherwise you Can End up Formatting the Same card 4 Times Over
	scr_clear(); // Clear The Screen to Hide output of mtGO()
	
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
	// SBV
	sbv_patch_enable_lmb();
	sbv_patch_disable_prefix_check();
}


void gotoOSDSYS(int sc)
{
	/*
	The Purpose of this Function is to Provide a Soft Reset and Handle Module Loading Errors.
	This Helps With Diagnosing Modules Failing to load.
	You Dont Actually Need it.
	*/
	if (sc != 0)
	{
		scr_printf(appFail);
		if(sc ==1 || sc ==2 || sc ==3 || sc ==4 || sc ==5)
		{
			scr_printf(modloadfail);
		}
		if (sc == 1)
		{
			scr_printf("SIO2MAN\n");
		}
		if (sc == 3)
		{
			scr_printf("freepad\n");
		}
		if (sc == 4)
		{
			scr_printf("MCMAN\n");
		}
		if (sc == 5)
		{
			scr_printf("MCSERV\n");
		}
		if (sc == 17)
		{
			scr_printf("Poweroff\n");
		}
		if (sc == 18)
		{
			scr_printf("MTAPMAN");
		}
		if (sc == 501)
		{
			scr_printf("Multitap Error \n");
		}
		if (sc == 999)
		{
			scr_printf("Unknown Error");
		}
		sleep(5);
	}
	ResetIOP();
	scr_printf(osdmsg);
	//Loads OSDSYS (We do this instead of Exiting to browser because we prefer it.)
	LoadExecPS2("rom0:OSDSYS", 0, NULL);
}

int main(int argc, char *argv[]) 
{

	ResetIOP();
	// initialize
	initialize();
	scr_clear();
	menu_header();	
	menu_Text();
	while (1)
	{
		//check to see if the pad is still connected
		checkPadConnected();
		//read pad 0 slot 0
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
			scr_printf("\n");
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

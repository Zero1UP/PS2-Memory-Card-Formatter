
// MTPT,mtP = Multitap Port
// mtRV = Multitap Retured Value

// Global Variable Used to Determine if the mtGO Function has already been used.
int MtapOPEN;

void mtOpenV()
{
	MtapOPEN = 1;				
}
// Closes Multitap Ports Automatically if mtGO() is called when multitap ports are already open in case you need to call mtGO() again.
void mtCloseP()
{
	int MTPT;
	for  (MTPT =0; MTPT<4; MTPT++)
	{
		mtapPortClose(MTPT);
	}
}

// A Simple For Loop To Handle Opening and Closing MultiTap ports. 
void mtGO()
{
	int MTPT;
	
	mtOpenV();
	
	// Close All MTAP Ports Automatically if This Function has Already Been Called
	if(MtapOPEN == 1)
	{
		mtCloseP();
	}
	
	for  (MTPT =0; MTPT<4; MTPT++)
	{
		mtDetect(MTPT);
	}
}

/*
	
	The mtDetect() Function Will Handle Detection of The Multitap and will also handle closing the port if it is not connected,
	
	Usage: 	mtDetect(<MTAP Port number>) [0,1,2,3]
	
	Example:mtDetect(1);
		
	WARNING: calling this function without specifying the MTAP Port Number as ang ARG will Crash the PS2!
			 Using a Negative Number (-1) or a Number Greater then 4 Will Also Result In a Crash!
	*/
void mtDetect(int mtP)
{
    /*
	Definitions
	MTPT,mtP = Multitap Port
    mtRV = Multitap Retured Value
	*/
    int mtRV;
	char *CP = "Controller Port";
	char *MCP = "Memory Card Port";
	char *mtD = "Multi-tap Detected! \n";
	char *mtND = "Multi-tap is Not Connected. \n";
	
    //printf("Multitap Status:\n");
	
	if (mtP ==0 || mtP ==1 || mtP ==2 || mtP ==3)
	{
		mtapPortOpen(mtP);
	   mtRV = mtapGetConnection(mtP);
		if (mtRV == 1)
		{
			printf("Multitap Detected Port %d Opened.\n", mtP);
			if (mtP == 0)
			{
				scr_printf("%s 0: %s",CP ,mtD);
			}
			if (mtP == 1)
			{
				scr_printf("%s 1: %s",CP, mtD);
			}
			if (mtP ==2)
			{
				scr_printf("%s 0: %s",MCP ,mtD);
			}
			if (mtP ==3)
			{
				scr_printf("%s 1: %s",MCP, mtD);
			}
			
		}
		else
		{
			printf("Mutitap Not Connected!\n");
			if (mtP == 0)
			{
				scr_printf("%s 0: %s",CP ,mtND);
			}
			if (mtP == 1)
			{
				scr_printf("%s 1: %s",CP, mtND);
			}
			if (mtP ==2)
			{
				scr_printf("%s 0: %s",MCP ,mtND);
			}
			if (mtP ==3)
			{
				scr_printf("%s 1: %s",MCP, mtND);
			}
			//Close MTAP Port if No Multitap is Connected
			mtapPortClose(mtP);
		}
	}
        
}

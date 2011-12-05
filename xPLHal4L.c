
#define _XPLHAL4L_C_

#include "xPLHal4L.h"


int stop = 0;


int HAL4L_levelTrace = 0;
static char logMessageBuffer[256];


int HAL4L_getLevelTrace() {
  return HAL4L_levelTrace;
}

void HAL4L_setLevelTrace(int level) {
  HAL4L_levelTrace = level;
}

/* Write a debug message out (if we are debugging) */
void HAL4L_Debug(int level, String theFormat, ...)
{
  va_list theParms;
  time_t rightNow;
  
  /* Skip if not a debug message */
  if (level>HAL4L_levelTrace) return;

  /* Get access to the variable parms */
  va_start(theParms, theFormat);

  /* Write a time stamp */
  time(&rightNow);
  strftime(logMessageBuffer, 40, "%y/%m/%d %H:%M:%S ", localtime(&rightNow));
  strcat(logMessageBuffer, "xPLHal4L : ");

  /* Convert formatted message */
  vsprintf(&logMessageBuffer[strlen(logMessageBuffer)], theFormat, theParms);

  /* Write to the console or system log file */
  strcat(logMessageBuffer, "\n");
  fprintf(stderr, logMessageBuffer);

  /* Release parms */
  va_end(theParms);
}



int getOptions ( int argc, char **argv)
{
    int c;
    int option_index = 0;
    struct option long_options[] =
    {
        {"config", 		1, NULL, 'c'},
        {"xhcpconfig", 	1, NULL, 'x'},
        {"debug", 		1, NULL, 'd'},
        {"help", 		0, NULL, 'h'},
        {0, 0, 0, 0}
    };
    
    while (1)
    {
        c = getopt_long (argc, argv, "x:c:hd:", long_options, &option_index);
        if (c == -1)
            break;
        
        switch (c)
        {
            case 'c':
                HAL4L_setConfigFile (optarg);
                break;
            case 'x':
                XHCP_setConfigFile (optarg);
                break;
            case 'd':
			printf("Argument '%s'\n",optarg);
                HAL4L_setLevelTrace(atoi(optarg));
                break;
            case 'h':
                printf ("option help\n");
                printf ("to do ...n");
                exit (0);
                break;
        }
        
    }
    
    return 1;
}



int loadHal4lConfig (char *fileName)
{
    HAL4L_Debug (HAL4L_INFO,"Loading xPLHal4L server configuration...");
	
	if ( rootConfig != NULL )
		 roxml_close (rootConfig);
    
    if ( fileName == NULL )
        Error_Quit ("Unable to load xPLHal4L config file");
    
    if ( (rootConfig = roxml_load_doc (fileName)) == NULL )
        Error_Quit ("Error loading xPLHal4L config file");
    
    return 0;
    
}

int saveHal4lConfig (char *fileName)
{
	int szBuffer = 0;
	char * buffer = NULL;
	FILE * file_out;

    HAL4L_Debug (HAL4L_INFO,"Write xPLHal4L server configuration...");

	szBuffer = roxml_commit_changes(rootConfig, NULL, &buffer, 1);

	file_out = fopen(fileName, "w");
	fwrite(buffer, 1, szBuffer, file_out);
	fclose(file_out);

	
    HAL4L_Debug (HAL4L_DEBUG,"%s", buffer);

	roxml_release(buffer);

	
//	szBuffer = roxml_commit_changes(rootConfig, fileName, NULL, 0);

	return 0;
}

int anim(int style)
{
	char *bt="\\|/-";
	char *ht=".oOo.oOo.      ";
	static int i = 0;
	static int sens = 0;
	
	switch (style)
	{
		case 1:
			i = (unsigned int) (++i % 4);
			write(STDOUT_FILENO, bt+i,1);
			write(STDOUT_FILENO,"\x0D",1);
			break;
		case 2:
			if ( i == 0 )
				sens = 1;
			else if ( i == 10 )
				sens = 0;
			if (sens )
				i++;
			else
				i--;
				
			write(STDOUT_FILENO, "                              ",i);
			write(STDOUT_FILENO, "- \x0D",3);
			break;
		case 3:
			if ( i == 0 )
				sens = 1;
			else if ( i == 15 )
				sens = 0;
			if (sens )
				i++;
			else
				i--;
				
			write(STDOUT_FILENO, "                              ",i);
			write(STDOUT_FILENO, bt+(i%4),1);
			write(STDOUT_FILENO, " \x0D",2);
			break;
		case 4:
			i = (unsigned int) (++i % 15);
			write(STDOUT_FILENO, ht+i,1);
			write(STDOUT_FILENO,"\x0D",1);
	}
		
}


void xpl4l_getSystemInfos ()
{
    
    char buffer[256];
    
    struct utsname sys_infos;
    
    
    if ( (uname (&sys_infos)) < 0 )
        Error_Quit ("Couldn't read system informations.");
    
    HAL4L_hostName = strdup (sys_infos.nodename);
    HAL4L_sysName = strdup (sys_infos.sysname);
    HAL4L_sysArchi = strdup (sys_infos.machine);
}

int main (int argc, String argv[])
{
    /* Parse command line parms */
    if (!xPL_parseCommonArgs (&argc, argv, FALSE))
    {
        fprintf (stderr, "Unable to parse xPL args");
        exit (1);
    }
    
    /* Start xPL up */
    if (!xPL_initialize (xPL_getParsedConnectionType ()))
    {
        fprintf (stderr, "Unable to start xPL");
        exit (1);
    }

	/* Initialize global variables */
	rootConfig = NULL;
	xpl4l_getSystemInfos ();
			
    if (!getOptions (argc, argv))
    {
        fprintf (stderr, "Error parsing common args");
        exit (1);
    }
    
    if ( HAL4L_getConfigFile ()==NULL )
    {
        HAL4L_Debug (HAL4L_INFO,"Loading default configuration file: config.xml");
        HAL4L_setConfigFile ("config.xml");
    }
    
    loadHal4lConfig (HAL4L_getConfigFile ());
    
    while ( !stop)
	{

		anim(1);

		enum XHCPstate_list toto;
		enum XHCPstate_list oldToto;

		xpl4l_timer(rootConfig);


		toto = XHCP_server (rootConfig);
		if ( toto != oldToto )
		{
			HAL4L_Debug(HAL4L_DEBUG,"Statut : %s", XHCPstate2String(toto));
			oldToto = toto;
		}


//		xpl4l_timer(rootConfig);
//		XHCP_server (rootConfig);
		usleep(100000);
	}


}

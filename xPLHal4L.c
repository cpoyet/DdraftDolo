
#define _XPLHAL4L_C_

#include "xPLHal4L.h"


int stop = 0;

int getOptions ( int argc, char **argv)
{
    int c;
    int option_index = 0;
    struct option long_options[] =
    {
        {"config", 		1, NULL, 'c'},
        {"xhcpconfig", 	1, NULL, 'x'},
        {"help", 		0, NULL, 'h'},
        {0, 0, 0, 0}
    };
    
    while (1)
    {
        c = getopt_long (argc, argv, "x:c:h", long_options, &option_index);
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
    printf ("Loading xPLHal4L server configuration...\n");
	
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

	szBuffer = roxml_commit_changes(rootConfig, NULL, &buffer, 1);

	file_out = fopen(fileName, "w");
	fwrite(buffer, 1, szBuffer, file_out);
	fclose(file_out);

	printf("Fichier config sauvegarde\n");
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
    
    if (!getOptions (argc, argv))
    {
        fprintf (stderr, "Error parsing common args");
        exit (1);
    }
    
    if ( HAL4L_getConfigFile ()==NULL )
    {
        printf ("Loading default configuration file: config.xml\n");
        HAL4L_setConfigFile ("config.xml");
    }
    
    loadHal4lConfig (HAL4L_getConfigFile ());
    
    while ( !stop)
	{
/*
	enum XHCPstate_list toto;
		enum XHCPstate_list oldToto;

		xpl4l_timer(rootConfig);


		toto = XHCP_server (rootConfig);
		if ( toto != oldToto )
		{
			printf("Statut : ");
			switch (toto)
			{
				case XHCPstate_init:
					printf("XHCPstate_init\n");
					break;
				case XHCPstate_waitConnect:
					printf("XHCPstate_waitConnect\n");
					break;
				case XHCPstate_waitCommand:
					printf("XHCPstate_waitCommand\n");
					break;
				case XHCPstate_waitData:
					printf("XHCPstate_waitData\n");
					break;
				case XHCPstate_endConnect:
					printf("XHCPstate_endConnect\n");
					break;
				case XHCPstate_death:
					printf("XHCPstate_death\n");
					break;
				default:
					printf("autre...\n");
					break;
			}
			oldToto = toto;
		}
*/

		anim(4);
		xpl4l_timer(rootConfig);
		XHCP_server (rootConfig);
		usleep(100000);
	}


}

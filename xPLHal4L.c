
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


int timer_loadConfig (node_t* argXmlConfig, int *delay)
{
    node_t **result;
    int nb_result;
    char buffer[80];
    int sz_buffer;
    char *xhcp_fileName;
    
    printf ("Loading timer configuration...\n");
    
 
    
    /* Time Out */
    result = roxml_xpath ( argXmlConfig, "//timer/tick[@interval]", &nb_result);
    if ( nb_result == 1 )
    {
        char *zaza = roxml_get_content ( roxml_get_attr (result[0], "interval", 0), buffer, 80, &sz_buffer );
        *delay = atoi (zaza);
        
        if ( *delay <= 0 )
            *delay=5;
    }
    else if ( nb_result == 0)
    {
        *delay = 5;
    }
    else
        Error_Quit ("Erroe parsing timer config file (interval)");
    
    roxml_release (RELEASE_LAST);
    printf ("*delay = %d\n", *delay);
    
    
    return 0;
}


int xpl4l_timer(node_t* argXmlConfig)
{
	static init = 1;
    static time_t t1;
	static delay;
	time_t t2;
    struct tm *ts;
    char       buf[80];

	if ( init )
	{
		t1 = 0;
		timer_loadConfig (argXmlConfig, &delay);
		init = 0;
	}
	
	t2 = time (NULL);
//printf("%d, %d\n",t2, t2%60);

	if ( t2%delay==0 && t1!=t2)
	{
		ts = localtime(&t2);
		strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);
		printf("%s\n", buf);

		//sendClockTick ();
		printf ("Message sending...\n");
		t1 = t2;
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

		write(STDOUT_FILENO,"|\x0D",2);
		xpl4l_timer(rootConfig);
		XHCP_server (rootConfig);
		usleep(100000);

		write(STDOUT_FILENO,"/\x0D",2);
		xpl4l_timer(rootConfig);
		XHCP_server (rootConfig);
		usleep(100000);

		write(STDOUT_FILENO,"-\x0D",2);
		xpl4l_timer(rootConfig);
		XHCP_server (rootConfig);
		usleep(100000);

		write(STDOUT_FILENO,"\\\x0D",2);
		xpl4l_timer(rootConfig);
		XHCP_server (rootConfig);
		usleep(100000);
		}
    
}

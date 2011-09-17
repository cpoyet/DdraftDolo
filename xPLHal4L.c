
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
    
    
    XHCP_server (rootConfig);
    
}

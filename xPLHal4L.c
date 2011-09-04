#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <errno.h>
#include <ctype.h>
#include <getopt.h>

#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <xPL.h>
#include <roxml.h>
//#include "./sqlite/sqlite3.h"

#include "XHCP_server.h"

#define XPLHAL4L_VERSION "1.0"


int getOptions( int argc, char **argv)
{
	int c;
	int option_index = 0;
	struct option long_options[] =
	{
		{"config", 	1, NULL, 'c'},
		{"help", 	0, NULL, 'h'},
		{0, 0, 0, 0}
	};

	while (1)
	{
		c = getopt_long (argc, argv, "c:h", long_options, &option_index);
		if (c == -1)
			break;
			
		switch (c)
		{
            case 'c':
				XHCP_setConfigFile(optarg);
				break;
			case 'h':
				printf ("option help\n");
				printf ("to do ...n");
				exit(0);
				break;
		}

	}
	
	return 1;
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
	
	if (!getOptions(argc, argv))
    {
        fprintf (stderr, "Error parsing common args");
        exit (1);
    }
	
	 XHCP_server(NULL);

}

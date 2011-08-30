#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <errno.h>

#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <xPL.h>
#include <roxml.h>
//#include "./sqlite/sqlite3.h"

#define XPLHAL4L_VERSION "1.0"


int main (int argc, String argv[])
{
    /* Parse command line parms */
    if (!xPL_parseCommonArgs (&argc, argv, FALSE)) exit (1);
    
    /* Start xPL up */
    if (!xPL_initialize (xPL_getParsedConnectionType ()))
    {
        fprintf (stderr, "Unable to start xPL");
        exit (1);
    }

}
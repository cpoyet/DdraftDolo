#ifndef _XPLHAL4L_H_
#define _XPLHAL4L_H_

#ifndef _XPLHAL4L_C_
#define EXT_XPLHAL4L extern
#else
#define EXT_XPLHAL4L 
#endif

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

#define XPLHAL4L_VERSION "0.0.1"
#define XPLHAL4L_VENDOR "xPLHal4L"

#ifndef IGNORE_EMPTY_TEXT_NODES
#define IGNORE_EMPTY_TEXT_NODES
#endif


/* Macros ********************************************************/

#define HAL4L_setConfigFile(V) HAL4L_configFile=V
#define HAL4L_getConfigFile() HAL4L_configFile

/* Prototypes ********************************************************************/
EXT_XPLHAL4L int saveHal4lConfig (char *);
EXT_XPLHAL4L int loadHal4lConfig (char *);

/* Variables globales ************************************************************/
EXT_XPLHAL4L char *HAL4L_configFile;
EXT_XPLHAL4L node_t *rootConfig;

EXT_XPLHAL4L int stop;


#endif

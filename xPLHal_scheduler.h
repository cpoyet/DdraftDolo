#ifndef _XPLHAL_SCHEDULER_H_
#define _XPLHAL_SCHEDULER_H_

#ifndef _XPLHAL_SCHEDULER_C_
#define EXT_XPLHAL_SCHEDULER extern
#else
#define EXT_XPLHAL_SCHEDULER 
#endif


#include <roxml.h>
#include <strings.h>


/* Macros ************************************************************************/

/* Definitions de types **********************************************************/
typedef struct TIMER_CONFIG
	{
		int clock_enabled;
		int clock_interval;
		int sched_interval;
	};

/* Prototypes ********************************************************************/
EXT_XPLHAL_SCHEDULER int timer_loadConfig (node_t* , int *, int*);
EXT_XPLHAL_SCHEDULER int xpl4l_timer(node_t* );

/* Variables globales ************************************************************/


#endif
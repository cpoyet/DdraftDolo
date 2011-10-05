#ifndef _XPLHAL_SCHEDULER_H_
#define _XPLHAL_SCHEDULER_H_

#ifndef _XPLHAL_SCHEDULER_C_
#define EXT_XPLHAL_SCHEDULER extern
#else
#define EXT_XPLHAL_SCHEDULER 
#endif


#include <roxml.h>
#include <strings.h>

/* Constantes ********************************************************************/
#define SCHEDULER_DEVICEID "scheduler"

/* Macros ************************************************************************/
#define TWtypeToStr(X) ( ( X == TWLIGHT_SUN ? "SUN" : \
						 ( X == TWLIGHT_CIVIL ? "CIVIL" : \
					     ( X == TWLIGHT_NAUTICAL ? "NAUTICAL" : \
					     ( X == TWLIGHT_ASTRONOMICAL ? "ASTRONOMICAL" : "UNKNOWN" )))) )

#define boolToStr(X) ( X == 0 ? "FALSE" : "TRUE" ) 
/* Definitions de types **********************************************************/
typedef enum _twilight_type { TWLIGHT_SUN, TWLIGHT_CIVIL, TWLIGHT_NAUTICAL, TWLIGHT_ASTRONOMICAL } twilight_t;

typedef struct _TIMER_CONFIG
	{
		int clock_enabled;
		int clock_interval;
		int sched_interval;
		int dawn_enabled;
		twilight_t dawn_type;
		int dusk_enabled;
		twilight_t dusk_type;
		char *userDateFormat;
	} TIMER_CONFIG;

/* Prototypes ********************************************************************/
EXT_XPLHAL_SCHEDULER int timer_loadConfig (node_t*);
EXT_XPLHAL_SCHEDULER int xpl4l_timer(node_t* );

/* Variables globales ************************************************************/

EXT_XPLHAL_SCHEDULER TIMER_CONFIG timerConfig;
#endif
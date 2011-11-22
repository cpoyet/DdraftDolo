
#define _XPLHAL_SCHEDULER_C_

#define _XOPEN_SOURCE 

#include "xPLHal_scheduler.h"
#include "xPLHal_rules.h"
#include "xPLHal_common.h"
#include "xPLHal4L.h"



xPL_ServicePtr clockService = NULL;
xPL_ServicePtr schedulerService = NULL;
xPL_MessagePtr clockTickMessage = NULL;
xPL_MessagePtr schedulerTickMessage = NULL;

int timeStr2int (char *str)
{
	int i = 0;
	int h = 0, m = 0, *t;
	unsigned int c = 0;
	
	t=&h;
	for (i=0; i<strlen(str); i++)
	{
		c = str[i]-0x30;
		if ( c > 9 )
			t=&m;
		else
			*t = *t * 10 + c;
	}
	return h*60+m;
//	return (str[0]-0x30)*600+(str[1]-0x30)*60+(str[3]-0x30)*10+(str[4]-0x30);
}

int dateStr2int( char *str)
{
	struct tm ts;
	char *r;
//printf("Date to convert : %s\n",str);
	r = strptime(str, "%d %b %Y", &ts);
//	printf("testing format \"%%d %%b %%Y\" r=[%s]\n",r?r:"NULL");
	if ( r == NULL || *r !='\0')
	{
		r = strptime(str, "%Y %m %d", &ts);
//		printf("testing format \"%%Y %%m %%d\" r=[%s]\n",r?r:"NULL");
	}
	if ( r == NULL || *r !='\0')
	{
		r = strptime(str, "%d-%m-%Y", &ts);
//		printf("testing format \"%%d-%%B-%%Y\" r=[%s]\n",r?r:"NULL");
	}
	if ( r == NULL || *r !='\0')
	{
		r = strptime(str, "%b %d %Y", &ts);
//		printf("testing format \"%%b %%d %%Y\" r=[%s]\n",r?r:"NULL");
	}
	if ( r == NULL || *r !='\0')
	{
		r = strptime(str, "%b %d, %Y", &ts);
//		printf("testing format \"%%b %%d, %%Y\" r=[%s]\n",r?r:"NULL");
	}
	if ( r == NULL || *r !='\0')
	{
		r = strptime(str, timerConfig.userDateFormat, &ts);
//		printf("testing user date format (\"%s\") r=[%s]\n", timerConfig.userDateFormat,r?r:"NULL");
	}
	 
//	printf("Y=%d, m=%d, d=%d\n", ts.tm_year,ts.tm_mon,ts.tm_mday);

	return (ts.tm_year+1900)*10000+(ts.tm_mon+1)*100+ts.tm_mday;
}

int monthStr2int ( char *str)
{
	int value=0;
	
	if ( strncasecmp(str,"ja",2) == 0 )
		value=1;
	else if ( strncasecmp(str,"fe",2) == 0 )
		value=2;
	else if ( strncasecmp(str,"mar",3) == 0 )
		value=3;
	else if ( strncasecmp(str,"ap",2) == 0  || strncasecmp(str,"av",2) == 0 )
		value=4;
	else if ( strncasecmp(str,"may",3) == 0 || strncasecmp(str,"mai",3) == 0 )
		value=5;
	else if ( strncasecmp(str,"jun",3) == 0 || strncasecmp(str,"juin",4) == 0 )
		value=6;
	else if ( strncasecmp(str,"jul",3) == 0 || strncasecmp(str,"juil",4) == 0 )
		value=7;
	else if ( strncasecmp(str,"au",2) == 0 || strncasecmp(str,"ao",2) == 0 )
		value=8;
	else if ( strncasecmp(str,"se",2) == 0 )
		value=9;
	else if ( strncasecmp(str,"oc",2) == 0 )
		value=10;
	else if ( strncasecmp(str,"no",2) == 0 )
		value=11;
	else if ( strncasecmp(str,"de",2) == 0 )
		value=12;

	if ( value == 0 )
		value = atoi(str);
		
	return value;
	
}


void timeEvent(time_t *time)
{

    node_t **determLst;
    int nbDetermLst;
	int anyRule;

	int i;

	
	/* On recherche tous les determinators contenant des conditions de temps */
	determLst = roxml_xpath ( rootConfig, "//timeCondition/ancestor-or-self::determinator", &nbDetermLst);

	for ( i=0; i<nbDetermLst; i++)
	{
		int ret;
		
		/* Type de gestion des rêgles */
		char *rule=xmlGetAttribut (determLst[i], "input", "match", FALSE);
		anyRule = ! strcasecmp(rule,"any");

		//ret = rules_verifTimeConditions( determLst[i], anyRule, tickDate, tickYear, tickMonth, tickDay, tickTime);
		ret = rules_verifTimeConditions( determLst[i], anyRule, time);

		if ( ret ) 
		{
			printf ("Le derterminator doit être executé\n");
rules_executeActions(determLst[i]);
			// if ( anyRule )
				// rules_executeActions(determLst[i]);
			// else
			// {
				// if (rules_verifAllConditions(determLst[i], /*TIME_EVENT*/ 0) )
					// rules_executeActions(determLst[i]);
			// }
		 }

	}
	
}

void internalMessageHandler(xPL_MessagePtr theMessage, xPL_ObjectPtr userValue)
{

	char *sourceVendor, *sourceDeviceID, *sourceInstanceID, *schemaClass, *schemaType;
	xPL_MessageType messgeType;	
	char *timeStr;
	int tickMonth, tickDate, tickDay, tickYear, tickTime;
	time_t epoch;




	sourceVendor = xPL_getSourceVendor (theMessage);
	sourceDeviceID = xPL_getSourceDeviceID (theMessage);
	sourceInstanceID = xPL_getSourceInstanceID (theMessage);
	messgeType = xPL_getMessageType (theMessage);
	schemaClass = xPL_getSchemaClass (theMessage);
	schemaType = xPL_getSchemaType (theMessage);

	printf ( "Received a Message from %s-%s.%s of type %d for %s.%s\n",
										sourceVendor, sourceDeviceID, sourceInstanceID, messgeType, schemaClass, schemaType);
	
	if ( strcmp(schemaClass, "internal") == 0)
	{
		if ( strcmp(schemaType, "tick") == 0 )
		{
			timeStr = xPL_getMessageNamedValue(theMessage, "time");
/*			int timeMinutes = timeStr2int( timeStr );*/
			tickDate  = atoi (xPL_getMessageNamedValue(theMessage, "date"));
			tickYear  = atoi (xPL_getMessageNamedValue(theMessage, "year"));
			tickMonth = atoi (xPL_getMessageNamedValue(theMessage, "month"));
			tickDay   = atoi (xPL_getMessageNamedValue(theMessage, "day"));
			tickTime  = timeStr2int( xPL_getMessageNamedValue(theMessage, "time") );
			
			
			epoch   = atol (xPL_getMessageNamedValue(theMessage, "epoch"));
			
			timeEvent( &epoch);
			
			printf("heure %s => %d\n", timeStr, tickTime);
			printf("tickMonth=%d tickDate=%d tickDay=%d tickYear=%d tickTime=%d epoch=%d\n", tickMonth, tickDate, tickDay, tickYear, tickTime, epoch );

		}
	}

}


void clockServiceHandler (xPL_ServicePtr theService, xPL_MessagePtr theMessage, xPL_ObjectPtr userValue)
{
    printf ( "Received a Clock Message from %s-%s.%s of type %d for %s.%s\n",
    xPL_getSourceVendor (theMessage), xPL_getSourceDeviceID (theMessage), xPL_getSourceInstanceID (theMessage),
    xPL_getMessageType (theMessage), xPL_getSchemaClass (theMessage), xPL_getSchemaType (theMessage));
    
}

twilight_t getTWtypeFromStr (char * str)
{
	if ( str == NULL || str[0]=='\0' )
		return TWLIGHT_SUN;
		
	if ( strcasecmp(str,"sun") == 0 )
		return TWLIGHT_SUN;
		
	if ( strcasecmp(str,"civil") == 0 )
		return TWLIGHT_CIVIL;
		
	if ( strcasecmp(str,"nautical") == 0 )
		return TWLIGHT_NAUTICAL;
		
	if ( strcasecmp(str,"astronotical") == 0 )
		return TWLIGHT_ASTRONOMICAL;
		
	return TWLIGHT_SUN;
}

//int timer_loadConfig (node_t* argXmlConfig, int *delay, int *clock_enabled)
int timer_loadConfig (node_t *argXmlConfig )
{
    char *tmp;
    
    printf ("Loading timer configuration...\n");
    
	
	/* Clock */
	timerConfig.clock_enabled = xmlGetBoolAttribut (argXmlConfig, "//clocking/xplclock", "enabled", 1);
	timerConfig.clock_interval = xmlGetIntAttribut (argXmlConfig, "//clocking/xplclock", "interval", 0);
	if ( timerConfig.clock_interval <= 0 ) timerConfig.clock_interval=60;
    
    /* internal tick */
	timerConfig.sched_interval = xmlGetIntAttribut (argXmlConfig, "//clocking/internal", "interval", 0);
	if ( timerConfig.sched_interval <= 0 ) timerConfig.sched_interval=5;
	
	/* Sunset - Sunrise */
	timerConfig.dawn_enabled = xmlGetBoolAttribut (argXmlConfig, "//clocking/twilight/dawn", "enabled", 1);
	timerConfig.dawn_type = getTWtypeFromStr ( xmlGetAttribut(argXmlConfig, "//clocking/twilight/dawn", "type", 1) );
	timerConfig.dusk_enabled = xmlGetBoolAttribut (argXmlConfig, "//clocking/twilight/dusk", "enabled", 1);
	timerConfig.dusk_type = getTWtypeFromStr ( xmlGetAttribut(argXmlConfig, "//clocking/twilight/dusk", "type", 1) );

	/* User date format */
	timerConfig.userDateFormat = strdup( xmlGetAttribut(argXmlConfig, "//user", "dateFormat", 1) );

	printf ("*clock_enabled = %d\n", timerConfig.clock_enabled);
    printf ("*delay = %d\n", timerConfig.sched_interval);
    
	printf (" timerConfig.dusk_type = %s\n", TWtypeToStr (timerConfig.dusk_type) );
    
    return 0;
}


xPL_MessagePtr createInternalTickMessage()
{
	xPL_MessagePtr tick_mess;

	/* Allocate the message */
	tick_mess = xPL_AllocMessage();

	/* Set the version (NOT DYNAMIC) */
	tick_mess->messageType = xPL_MESSAGE_TRIGGER;
	tick_mess->receivedMessage = TRUE;

	/* Allocate a name/value list, if needed */
	if (tick_mess->messageBody == NULL) tick_mess->messageBody = xPL_AllocNVList();
	
	/* Install source into message */
	xPL_setSource(tick_mess, XPLHAL4L_VENDOR, SCHEDULER_DEVICEID, HAL4L_hostName);

	xPL_setTarget(tick_mess, XPLHAL4L_VENDOR, SCHEDULER_DEVICEID, HAL4L_hostName);

	xPL_setSchema(tick_mess, "internal", "tick");

	return tick_mess;
}

int xpl4l_timer(node_t* argXmlConfig)
{
	static init = 1;
    static time_t oldClockTime, oldTickTime;
	time_t t;
    struct tm *ts;
    char       buf[80];

	Bool ret;
	
	if ( init )
	{
		t = 0;
		timer_loadConfig (argXmlConfig);
		init = 0;
		
		clockService = xPL_createService (XPLHAL4L_VENDOR, "clock", HAL4L_hostName);
		xPL_setServiceVersion (clockService, XPLHAL4L_VERSION);

		/* Add a responder for time setting */
		xPL_addServiceListener (clockService, clockServiceHandler, xPL_MESSAGE_ANY, "clock", NULL, NULL);
		
		/* Create a message to send */
		clockTickMessage = xPL_createBroadcastMessage (clockService, xPL_MESSAGE_STATUS);
		
		xPL_setSchema (clockTickMessage, "clock", "update");

		/* Internals messages */
		schedulerTickMessage = createInternalTickMessage();
		xPL_addMessageListener(internalMessageHandler, NULL);
		
		
	}
	
	t = time (NULL);
	ts = localtime(&t);

	if ( timerConfig.clock_enabled && t%timerConfig.clock_interval==0 && t!=oldClockTime)
	{

		strftime(buf, sizeof(buf), "%Y%m%d%H%M%S", ts);
		
		/* Install the value and send the message */
		xPL_setMessageNamedValue (clockTickMessage, "time", buf);
		
		/* Broadcast the message */
		xPL_sendMessage (clockTickMessage);

		oldClockTime = t;
	}

	if ( t%timerConfig.sched_interval==0 && t!=oldTickTime)
	{
		strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);
		
		/* Crée le messageBody si NULL et y ajoute une seule ligne si elle n'existe pas, sinon la met à jour */
		xPL_setMessageNamedValue(schedulerTickMessage, "timestamp", buf);
		strftime(buf, sizeof(buf), "%H:%M", ts);
		xPL_setMessageNamedValue(schedulerTickMessage, "time", buf);
		strftime(buf, sizeof(buf), "%Y", ts);
		xPL_setMessageNamedValue(schedulerTickMessage, "year", buf);
		strftime(buf, sizeof(buf), "%m", ts);
		xPL_setMessageNamedValue(schedulerTickMessage, "month", buf);
		strftime(buf, sizeof(buf), "%Y%m%d", ts);
		xPL_setMessageNamedValue(schedulerTickMessage, "date", buf);
		strftime(buf, sizeof(buf), "%d", ts);
		xPL_setMessageNamedValue(schedulerTickMessage, "day", buf);

		sprintf(buf, "%d", t);
		xPL_setMessageNamedValue(schedulerTickMessage, "epoch", buf);


		
		ret = xPL_dispatchMessageEvent(schedulerTickMessage);

		printf ("Message dispatched = %s\n", ret == TRUE ? "TRUE":"FALSE");
    
		oldTickTime = t;

	}


}

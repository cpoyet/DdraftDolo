
#define _XPLHAL_SCHEDULER_C_

#include "xPLHal_scheduler.h"
#include "xPLHal4L.h"


xPL_ServicePtr clockService = NULL;
xPL_ServicePtr schedulerService = NULL;
xPL_MessagePtr clockTickMessage = NULL;
xPL_MessagePtr schedulerTickMessage = NULL;


void internalMessageHandler(xPL_MessagePtr theMessage, xPL_ObjectPtr userValue)
{
    node_t **result;
    int nb_result;


	char *sourceVendor = xPL_getSourceVendor (theMessage);
	char *sourceDeviceID = xPL_getSourceDeviceID (theMessage);
	char *sourceInstanceID = xPL_getSourceInstanceID (theMessage);
	xPL_MessageType messgeType = xPL_getMessageType (theMessage);
	char *schemaClass = xPL_getSchemaClass (theMessage);
	char *schemaType = xPL_getSchemaType (theMessage);

	printf ( "Received a Message from %s-%s.%s of type %d for %s.%s\n",
										sourceVendor, sourceDeviceID, sourceInstanceID, messgeType, schemaClass, schemaType);
	if ( strcmp(schemaClass, "internal") == 0)
	{
		if ( strcmp(schemaType, "tick") == 0 )
		{
			/* On recherche tous les determinators contenant des conditions de temps */
			result = roxml_xpath ( rootConfig, "//determinator//timeCondition", &nb_result);
			printf("%d timeConditions trouv�es\n",nb_result);
			
		}
	}

}

void clockServiceHandler (xPL_ServicePtr theService, xPL_MessagePtr theMessage, xPL_ObjectPtr userValue)
{
    printf ( "Received a Clock Message from %s-%s.%s of type %d for %s.%s\n",
    xPL_getSourceVendor (theMessage), xPL_getSourceDeviceID (theMessage), xPL_getSourceInstanceID (theMessage),
    xPL_getMessageType (theMessage), xPL_getSchemaClass (theMessage), xPL_getSchemaType (theMessage));
    
}

char *xmlGetAttribut (node_t* argXmlConfig, char *nodeXpath, char *attrName, int opt)
{
    node_t **result;
    int nb_result;
    static char buffer[80];
    int sz_buffer;
	char *tmp;

	char *xpath = (char *)malloc(strlen(nodeXpath)+strlen(attrName)+4);
	
	sprintf(xpath,"%s[@%s]",nodeXpath,attrName);
    result = roxml_xpath ( argXmlConfig, xpath, &nb_result);
    if ( nb_result == 1 )
	{
        tmp = roxml_get_content ( roxml_get_attr (result[0], attrName, 0), buffer, 80, &sz_buffer );
	}
	else if ( nb_result == 0 && opt )
	{
		tmp="";
	}
	else	
	{
		fprintf(stderr,"ERROR Parsing %s (%d results)\n",xpath,nb_result);
        Error_Quit ("Error parsing timer config file");
	}
	free(xpath);
	roxml_release (RELEASE_LAST);

	return tmp;
}

int xmlGetBoolAttribut (node_t* argXmlConfig, char *nodeXpath, char *attrName, int opt)
{
	int i=0;
	char *trueLst[] = {"TRUE","1","ON","VRAI",NULL};
	char *falseLst[] = {"FALSE","0","OFF","FAUX",NULL};
	
	char *attr=xmlGetAttribut (argXmlConfig, nodeXpath, attrName, opt);
	
	do
	{
		if ( strcasecmp (trueLst[i], attr) == 0 )
			return 1;
	} while ( trueLst[++i]!=NULL );
	
	return 0;
}

int xmlGetIntAttribut (node_t* argXmlConfig, char *nodeXpath, char *attrName, int opt)
{
	char *attr=xmlGetAttribut (argXmlConfig, nodeXpath, attrName, opt);
	
	return atoi(attr);
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
		
		/* Cr�e le messageBody si NULL et y ajoute une seule ligne si elle n'existe pas, sinon la met � jour */
		xPL_setMessageNamedValue(schedulerTickMessage, "time", buf);

		ret = xPL_dispatchMessageEvent(schedulerTickMessage);

		printf ("Message dispatched = %s\n", ret == TRUE ? "TRUE":"FALSE");
    
		oldTickTime = t;

	}


}


#define _XPLHAL_SCHEDULER_C_

#include "xPLHal_scheduler.h"
#include "xPLHal4L.h"


xPL_ServicePtr clockService = NULL;
xPL_ServicePtr schedulerService = NULL;
xPL_MessagePtr clockTickMessage = NULL;
xPL_MessagePtr schedulerTickMessage = NULL;


void clockMessageHandler(xPL_MessagePtr theMessage, xPL_ObjectPtr userValue)
{
    printf ( "Received a Message from %s-%s.%s of type %d for %s.%s\n",
    xPL_getSourceVendor (theMessage), xPL_getSourceDeviceID (theMessage), xPL_getSourceInstanceID (theMessage),
    xPL_getMessageType (theMessage), xPL_getSchemaClass (theMessage), xPL_getSchemaType (theMessage));
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
    
    /* internal tip */
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


int xpl4l_timer(node_t* argXmlConfig)
{
	static init = 1;
    static time_t t1;
	//static int delay;
	static int clock_enabled;
	time_t t2;
    struct tm *ts;
    char       buf[80];

	if ( init )
	{
		t1 = 0;
		timer_loadConfig (argXmlConfig);
		init = 0;
		
    clockService = xPL_createService ("dolo", "clock", "default");
    xPL_setServiceVersion (clockService, XPLHAL4L_VERSION);

	/* Add a responder for time setting */
//		xPL_addServiceListener (clockService, clockServiceHandler, xPL_MESSAGE_ANY, "clock", NULL, NULL);
 xPL_addMessageListener(clockMessageHandler, NULL);
		
		/* Create a message to send */
		clockTickMessage = xPL_createBroadcastMessage (clockService, xPL_MESSAGE_STATUS);
		xPL_setSchema (clockTickMessage, "clock", "update");

	}
	
	t2 = time (NULL);
//printf("%d, %d\n",t2, t2%60);

	//if ( t2%delay==0 && t1!=t2)
	if ( t2%timerConfig.sched_interval==0 && t1!=t2)
	{
		ts = localtime(&t2);
		strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);
		printf("%s\n", buf);

    strftime (buf, 24, "%Y%m%d%H%M%S", ts);
    /* Install the value and send the message */
    xPL_setMessageNamedValue (clockTickMessage, "time", buf);
    
    /* Broadcast the message */
    xPL_sendMessage (clockTickMessage);


xPL_dispatchMessageEvent(clockTickMessage);
    
		t1 = t2;
	}


}


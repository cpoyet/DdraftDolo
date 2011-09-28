
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

char *xmlGetAttribut (node_t* argXmlConfig, char *nodeXpath, char *attrName)
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
	else
	{
		fprintf(stderr,"ERROR Parsing %s (%d results)\n",xpath,nb_result);
        Error_Quit ("Error parsing timer config file");
	}
	free(xpath);
	roxml_release (RELEASE_LAST);

	return tmp;
}

int xmlGetBoolAttribut (node_t* argXmlConfig, char *nodeXpath, char *attrName)
{
	int i=0;
	char *trueLst[] = {"TRUE","1","ON","VRAI",NULL};
	char *falseLst[] = {"FALSE","0","OFF","FAUX",NULL};
	
	char *attr=xmlGetAttribut (argXmlConfig, nodeXpath, attrName);
	
	do
	{
		if ( strcasecmp (trueLst[i], attr) == 0 )
			return 1;
	} while ( trueLst[++i]!=NULL );
	
	return 0;
}

int xmlGetIntAttribut (node_t* argXmlConfig, char *nodeXpath, char *attrName)
{
	char *attr=xmlGetAttribut (argXmlConfig, nodeXpath, attrName);
	
	return atoi(attr);
}

int timer_loadConfig (node_t* argXmlConfig, int *delay, int *clock_enabled)
{
    char *xhcp_fileName;
    
    printf ("Loading timer configuration...\n");
    
	/* Default values */
	*clock_enabled = 0;
	*delay = 5;
	
	/* Enabled */
	*clock_enabled = xmlGetBoolAttribut (argXmlConfig, "//clocking/xplclock", "enabled");
    
    /* Time Out */
    
	*delay = xmlGetIntAttribut (argXmlConfig, "//clocking/internal", "interval");
	if ( *delay <= 0 )
		*delay=5;
	
	
	
	
    
    printf ("*clock_enabled = %d\n", *clock_enabled);
    printf ("*delay = %d\n", *delay);
    
    
    return 0;
}


int xpl4l_timer(node_t* argXmlConfig)
{
	static init = 1;
    static time_t t1;
	static int delay;
	static int clock_enabled;
	time_t t2;
    struct tm *ts;
    char       buf[80];

	if ( init )
	{
		t1 = 0;
		timer_loadConfig (argXmlConfig, &delay, &clock_enabled);
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

	if ( t2%delay==0 && t1!=t2)
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


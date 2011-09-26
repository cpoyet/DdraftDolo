
#define _XPLHAL_SCHEDULER_C_

#include "xPLHal_scheduler.h"
#include "xPLHal4L.h"


xPL_ServicePtr clockService = NULL;
xPL_ServicePtr schedulerService = NULL;
xPL_MessagePtr clockTickMessage = NULL;
xPL_MessagePtr schedulerTickMessage = NULL;


void clockMessageHandler (xPL_ServicePtr theService, xPL_MessagePtr theMessage, xPL_ObjectPtr userValue)
{
    fprintf (stderr, "Received a Clock Message from %s-%s.%s of type %d for %s.%s\n",
    xPL_getSourceVendor (theMessage), xPL_getSourceDeviceID (theMessage), xPL_getSourceInstanceID (theMessage),
    xPL_getMessageType (theMessage), xPL_getSchemaClass (theMessage), xPL_getSchemaType (theMessage));
    
}

int timer_loadConfig (node_t* argXmlConfig, int *delay)
{
    node_t **result;
    int nb_result;
    char buffer[80];
    int sz_buffer;
    char *xhcp_fileName;
    
    printf ("Loading timer configuration...\n");
    
 
    
    /* Time Out */
    result = roxml_xpath ( argXmlConfig, "//timer/tick[@interval]", &nb_result);
    if ( nb_result == 1 )
    {
        char *zaza = roxml_get_content ( roxml_get_attr (result[0], "interval", 0), buffer, 80, &sz_buffer );
        *delay = atoi (zaza);
        
        if ( *delay <= 0 )
            *delay=5;
    }
    else if ( nb_result == 0)
    {
        *delay = 5;
    }
    else
        Error_Quit ("Erroe parsing timer config file (interval)");
    
    roxml_release (RELEASE_LAST);
    printf ("*delay = %d\n", *delay);
    
    
    return 0;
}


int xpl4l_timer(node_t* argXmlConfig)
{
	static init = 1;
    static time_t t1;
	static delay;
	time_t t2;
    struct tm *ts;
    char       buf[80];

	if ( init )
	{
		t1 = 0;
		timer_loadConfig (argXmlConfig, &delay);
		init = 0;
		
    clockService = xPL_createService ("dolo", "clock", "default");
    xPL_setServiceVersion (clockService, XPLHAL4L_VERSION);

	/* Add a responder for time setting */
		xPL_addServiceListener (clockService, clockMessageHandler, xPL_MESSAGE_ANY, "clock", NULL, NULL);
		
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
    
		printf ("Message sending...\n");
		t1 = t2;
	}


}


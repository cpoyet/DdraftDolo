/* xPL_Clock.c -- Simple xPL clock service that sends a time update out once a minute */
/* Copyright (c) 2004, Gerald R. Duprey Jr. */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#include <time.h>
#include <signal.h>
#include <pthread.h>
#include "../xPL.h"
//#include "./sqlite/sqlite3.h"

#define CLOCK_VERSION "1.0"

static time_t lastTimeSent = 0;
static xPL_ServicePtr clockService = NULL;
static xPL_ServicePtr schedulerService = NULL;
static xPL_MessagePtr clockTickMessage = NULL;
static xPL_MessagePtr schedulerTickMessage = NULL;

pthread_t th_clock, th_event_mill;
//sqlite3 *db;

typedef enum
{ TIME, MESSAGE, CONFIG } EVENT_TYPE;

typedef struct
{
    EVENT_TYPE type;
    time_t timeStamp;
    void * data;
} EVENT;

#define EVENTS_BUFFER_SIZE 10

struct
{
    EVENT * buffer[EVENTS_BUFFER_SIZE];
    int head;
    int tail;
} events_buffer;


int eventStack_available (void)
{
    return (unsigned int)(EVENTS_BUFFER_SIZE + events_buffer.head - events_buffer.tail) % EVENTS_BUFFER_SIZE;
}

EVENT * eventStack_get ()
{
    EVENT *evt = NULL;
    
    if (events_buffer.head != events_buffer.tail)
    {
        evt = events_buffer.buffer[events_buffer.tail];
        events_buffer.tail = (unsigned int)(events_buffer.tail + 1) % EVENTS_BUFFER_SIZE;
    }
    return evt;
}

int eventStack_add (EVENT *evt)
{
    int i = 0;
    
    i = (events_buffer.head + 1) % EVENTS_BUFFER_SIZE;
    
    if (i == events_buffer.tail)
    {
        fprintf (stderr, "Events buffer overflow !!!\n");
        return -1;
    }
    
    events_buffer.buffer[events_buffer.head] = evt;
    events_buffer.head = i;
    
    printf ("Adding event: tail=%d, head=%d, (%d/%d)\n", events_buffer.tail, events_buffer.head, eventStack_available (), EVENTS_BUFFER_SIZE);
    
    return 0;
}

void eventStack_flush ()
{
    events_buffer.tail = 0;
    events_buffer.head = events_buffer.tail;
}

EVENT * create_event (EVENT_TYPE type, void * data)
{
    EVENT * evt = NULL;
    
    time_t tStamp = time (NULL);
    
    
    if ( (evt = (EVENT *)malloc (sizeof(EVENT ))) == NULL )
    {
        fprintf (stderr, "Events alloc error !!!\n");
        return NULL;
    }
    
    evt->type = type;
    evt->timeStamp = tStamp;
    evt->data = data;
    return evt;
}

int delete_event (EVENT * evt)
{
    if ( evt->data != NULL )
        free (evt->data );
    
    free (evt);
    
    return 0;
}


void clockMessageHandler (xPL_ServicePtr theService, xPL_MessagePtr theMessage, xPL_ObjectPtr userValue)
{
    fprintf (stderr, "Received a Clock Message from %s-%s.%s of type %d for %s.%s\n",
    xPL_getSourceVendor (theMessage), xPL_getSourceDeviceID (theMessage), xPL_getSourceInstanceID (theMessage),
    xPL_getMessageType (theMessage), xPL_getSchemaClass (theMessage), xPL_getSchemaType (theMessage));
    
}

void schedulerMessageHandler(xPL_ServicePtr theService, xPL_MessagePtr theMessage, xPL_ObjectPtr userValue)
{
	return;
}


void shutdownHandler (int onSignal)
{
    xPL_setServiceEnabled (clockService, FALSE);
    xPL_releaseService (clockService);
    xPL_shutdown ();
    exit (0);
}

static void sendClockTick ()
{
    time_t rightNow = time (NULL);
    struct tm * decodedTime;
    char theDateTime[24];
    EVENT *evt;
    
    /* Skip unless a minute has passed (or this is our first time */
    /*if ((lastTimeSent != 0) && ((rightNow - lastTimeSent) < 60)) return; */
    // if ((lastTimeSent != 0) && ((rightNow - lastTimeSent) < 1)) return;
    
    /* Format the date/time */
    decodedTime = localtime (&rightNow);
    strftime (theDateTime, 24, "%Y%m%d%H%M%S", decodedTime);
    
    /* Install the value and send the message */
    xPL_setMessageNamedValue (clockTickMessage, "time", theDateTime);
    
    /* Broadcast the message */
    xPL_sendMessage (clockTickMessage);
    
    evt = create_event ( TIME , NULL);
    if ( eventStack_add (evt) != 0 )
    {
        fprintf (stderr, "Error adding event ... \n");
    }
    /* And reset when we last sent the clock update */
    //  lastTimeSent = rightNow;
}



void *thread_clock_process (void * arg)
{
    
    time_t t1 = time (NULL);
    time_t t2;
    long dt;
    int rescan = 60;
    int sleep_time = 5;

    struct tm *ts;
    char       buf[80];
  xPL_MessagePtr theMessage;
  
/*  theMessage = createReceivedMessage(xPL_MESSAGE_ANY);
  xPL_setMessageType(theMessage, xPL_MESSAGE_TRIGGER);*/
  
  String theMessageText="xpl-trig\n"
                        "{\n"
                        "hop=1\n"
                        "source=acme-pir.frontdoor\n"
                        "target=*\n"
                        "}\n"
                        "timer.tip\n"
                        "{\n"
                        "device=scheduler\n"
                        "time=%s\n"
                        "}";
  
  theMessage = parseMessage(theMessageText);
    
    while  ( 1 == 1 )
    {
//        sleep ((sleep_time + 1) - (time (NULL) % sleep_time));
    
usleep(500000);

    
        t2 = time (NULL);
printf("%d, %d\n",t2, t2%60);
if ( t2%60 == 0 )
{

    ts = localtime(&t2);
    strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);
    printf("%s\n", buf);
	sleep(1);
	}


        dt = (long)t2 - (long)t1;
        
       sendClockTick ();
        printf ("Message sending...\n");
        t1 = t2;
    }
	
    
    
    
    pthread_exit (0);
}
/*
xPL_MessagePtr createSchedulerMessage(void)
{
	xPL_MessagePtr theMessage;
	
	theMessage = createReceivedMessage(xPL_MESSAGE_ANY); // ou  xPL_MESSAGE_TRIGGER peut-être ?

	theMessage->hopCount = 1;

	xPL_setSource(theMessage, "dolo", "xPLHal4Linux", "scheduler");
	xPL_setTarget(theMessage, "dolo", "xPLHal4Linux", "scheduler");

	xPL_setSchema(theMessage, "timer", "tip");

return theMessage;

}
*/

#define WS_BUFFER_SIZE 512 
int get_url(char ** dest, char * url)
{
  int sockid;
  int bufsize;
  char buffer[WS_BUFFER_SIZE];
  struct sockaddr_in socketaddr;
  struct hostent *hostaddr;
  struct servent *servaddr;
  struct protoent *protocol;
  
  char *host;
  char *file;
  int nbcar=0;
 
  char *http_adr = strdup(url);
  
  *dest=NULL;
  
  if (strncmp(http_adr, "http://", 7) == 0)
	host = http_adr+7;
  else
	host = http_adr;

  file = strchr(host,'/');
  *file='\0';
  file++;
  
 
  sprintf(buffer, "GET /%s HTTP/1.0\r\n"
					"Host: %s\r\n"
					"User-Agent: Hal4Linux\r\n"
					"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
					"Accept-Language: fr,fr-fr;q=0.8,en-us;q=0.5,en;q=0.3\r\n"
					"Accept-Encoding: gzip, deflate\r\n"
					"Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"
					"Connection: keep-alive\r\n"
					"Pragma: no-cache\r\n"
					"Cache-Control: no-cache\r\n"
					"\r\n", file, host);
// printf("%s\n",buffer);
 
  /* Resolve the host name */
  if (!(hostaddr = gethostbyname(host))) {
    free(http_adr);
    fprintf(stderr, "Error resolving host.");
    return(-1);
  }
 
  /* clear and initialize socketaddr */
  memset(&socketaddr, 0, sizeof(socketaddr));
  socketaddr.sin_family = AF_INET;
 
  /* setup the servent struct using getservbyname */
  servaddr = getservbyname("http", "tcp");
  socketaddr.sin_port = servaddr->s_port;
 
  memcpy(&socketaddr.sin_addr, hostaddr->h_addr, hostaddr->h_length);
 
  /* protocol must be a number when used with socket()
     since we are using tcp protocol->p_proto will be 0 */
  protocol = getprotobyname("tcp");
 
  sockid = socket(AF_INET, SOCK_STREAM, protocol->p_proto);
  if (sockid < 0) {
    free(http_adr);
    fprintf(stderr, "Error creating socket.");
   return(-1);
  }
 
  /* everything is setup, now we connect */
  if(connect(sockid, (struct sockaddr*)&socketaddr, sizeof(socketaddr)) == -1) {
    free(http_adr);
    fprintf(stderr, "Error connecting.");
    return(-1);
  }
  /* send our get request for http */
  if (send(sockid, buffer, strlen(buffer), 0) == -1) {
    free(http_adr);
    fprintf(stderr, "Error sending data.");
    return(-1);
  }
 
   bzero(buffer,WS_BUFFER_SIZE);
 
  /* read the socket until its clear then exit */
  while ( (bufsize = read(sockid, buffer, WS_BUFFER_SIZE - 1)))
  {
	*dest = (char*) realloc (*dest, nbcar + bufsize + 1);
	strncpy((*dest)+nbcar, buffer, bufsize);
	nbcar += bufsize;
	(*dest)[nbcar]='\0';
  }
  
  free(http_adr);
  close(sockid);
  
  return nbcar;
}




int get_url_content(char ** dest, char * url)
{

	char *tmp_buff = NULL;
	char *tmp_content;
	int nbcar = 0;
	int i;
	
	nbcar = get_url(&tmp_buff,url);
//printf("%s\n---\n",tmp_buff);
	
	/*
	
	for (i=0; i<nbcar-3; i++)
	{
		if (tmp_buff[i]=='<' && tmp_buff[i+1]=='\r' )
		{
			(*dest) = (char *)malloc(sizeof(tmp_buff+i));
			strcpy ( *dest, tmp_buff+i+1 );
			break;
		}
	
	}
*/
	tmp_content = strstr(tmp_buff,"<?xml");
//	printf("strstr OK\n");
//	printf("%s\n---\n",tmp_content);
	(*dest) = (char *)malloc(strlen(tmp_content)+1);
	strcpy ( *dest, tmp_content );

	free (tmp_buff);
return strlen(tmp_content);
}


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
    
    /* Initialze clock service */
    
    /* Create  a service for us */
    clockService = xPL_createService ("dolo", "clock", "default");
    xPL_setServiceVersion (clockService, CLOCK_VERSION);
    
    schedulerService = xPL_createService ("dolo", "scheduler", "default");
    xPL_setServiceVersion (schedulerService, CLOCK_VERSION);

char *test =NULL;
int nbcar = get_url_content(&test,"http://www.earthtools.org/sun/48.376126/2.810753/25/8/1/1");
printf("----\n%s\n--- %d recus ---\n",test, nbcar);
free(test);
test=NULL;
nbcar = get_url_content(&test,"www.earthtools.org/height/48.376126/2.810753");
printf("----\n%s\n--- %d recus ---\n",test, nbcar);
free(test);


    /* Add a responder for time setting */
    xPL_addServiceListener (clockService, clockMessageHandler, xPL_MESSAGE_ANY, "clock", NULL, NULL);
    
    /* Create a message to send */
    clockTickMessage = xPL_createBroadcastMessage (clockService, xPL_MESSAGE_STATUS);
    xPL_setSchema (clockTickMessage, "clock", "update");
    
	//schedulerTickMessage = createSchedulerMessage();
    xPL_addServiceListener (schedulerService, schedulerMessageHandler, xPL_MESSAGE_ANY, "timer", NULL, NULL);
    schedulerTickMessage = xPL_createTargetedMessage(schedulerService, xPL_MESSAGE_ANY, 
																 xPL_getServiceVendor(schedulerService),
																 xPL_getServiceDeviceID(schedulerService),
																 xPL_getServiceInstanceID(schedulerService));
    
    eventStack_flush ();
    
    if (pthread_create (&th_clock, NULL, thread_clock_process, "1") < 0)
    {
        fprintf (stderr, "pthread_create error for thread 1\n");
        exit (1);
    }
    
/*     int rc;
    rc = sqlite3_open ("DraftDolo.db", &db);
    if( rc )
    {
        fprintf (stderr, "Can't open database: %s\n", sqlite3_errmsg (db));
        sqlite3_close (db);
        exit (1);
    }
 */    
    
    /* Install signal traps for proper shutdown */
    signal (SIGTERM, shutdownHandler);
    signal (SIGINT, shutdownHandler);
    
    /* Enable the service */
    xPL_setServiceEnabled (clockService, TRUE);
    
    /** Main Loop of Clock Action **/
    
    for (;;)
    {
        /* Let XPL run for a while, returning after it hasn't seen any */
        /* activity in 100ms or so                                     */
        xPL_processMessages (100);
        
        /* Process clock tick update checking */
        //sendClockTick();
    }
}


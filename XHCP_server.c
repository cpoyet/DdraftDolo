

#define _XHCP_SERVER_C_


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <sys/socket.h>       /*  socket definitions        */
#include <sys/types.h>        /*  socket types              */
#include <sys/wait.h>         /*  for waitpid()             */
#include <arpa/inet.h>        /*  inet (3) funtions         */
#include <unistd.h>           /*  misc. UNIX functions      */
#include <sys/time.h>         /*  For select()  */


#include "XHCP_server.h"

/*  Global macros/variables  */


#define XHCP_HAL_vendor "xPLHal4L"
#define XHCP_HAL_device "xPLHal4L"
#define XHCP_HAL_version "0.0.1"
#define XHCP_version "1.5"

#define LISTENQ          			(1024)

#define XHCP_SERVER_PORT            (3865)



void Error_Quit(char const * msg)
{
    fprintf(stderr, "WEBSERV: %s\n", msg);
    exit(EXIT_FAILURE);
}


/*  Read a line from a socket  */

ssize_t Readline(int sockd, void *vptr, size_t maxlen)
{
    ssize_t n, rc;
    char    c, *buffer;
    
    buffer = vptr;
    
    for ( n = 1; n < maxlen; n++ )
    {
        
        if ( (rc = read(sockd, &c, 1)) == 1 )
        {
            *buffer++ = c;
            if ( c == '\n' )
				break;
        }
        else if ( rc == 0 )
        {
            if ( n == 1 )
				return 0;
            else
				break;
        }
        else
        {
            if ( errno == EINTR )
				continue;
            Error_Quit("Error in Readline()");
        }
    }
    
    *buffer = 0;
    return n;
}


/*  Removes trailing whitespace from a string  */

int Trim(char * buffer)
{
    int n = strlen(buffer) - 1;
    
    while ( !isalnum(buffer[n]) && n >= 0 )
		buffer[n--] = '\0';
    
    return 0;
}

/*  Write a line to a socket  */

ssize_t Writeline(int sockd, const void *vptr, size_t n)
{
    size_t      nleft;
    ssize_t     nwritten;
    const char *buffer;
    
    buffer = vptr;
    nleft  = n;
    
    while ( nleft > 0 )
    {
        if ( (nwritten = write(sockd, buffer, nleft)) <= 0 )
        {
            if ( errno == EINTR )
				nwritten = 0;
            else
				Error_Quit("Error in Writeline()");
        }
        nleft  -= nwritten;
        buffer += nwritten;
    }
    
    return n;
}

void XHCP_printXHCPMessage(int sockd, XHCP_response_id messId )
{
	XHCP_response *resp;
	int i;
	char response_msg[256];	
	
	
	for ( resp = &XHCP_responseList[0]; resp->id != END_RES && resp->id != messId; resp++);
	
	sprintf(response_msg, "%d %s\r\n",resp->num, resp->str);
	
	Writeline(sockd, response_msg, strlen(response_msg));
}

void XHCP_printCustomMessage(int sockd, int messNum, char *Libelle, ... )
{
	va_list Marker;
	char response_msg[256];	
	char *tmp;
	
	sprintf(response_msg,"%d ", messNum);
	tmp = response_msg+strlen(response_msg);
	
    /* construction du libelle du message */
    va_start ( Marker, Libelle);
    vsprintf (tmp, Libelle, Marker);
    va_end ( Marker);

	strcat(response_msg,"\r\n");
	
	Writeline(sockd, response_msg, strlen(response_msg));
}


int Parse_Line(int conn, char *buffer)
{
	char *line=NULL;
	char *argv[MAX_CMD_ARGS+1];
	int  argc=0;
	
	char *token, *svgptr;
	int i;
	char *c;
	
	XHCP_command *cmd;
	
	int retValue=0;
	
	if ( (line = strdup (buffer)) == NULL )
		return -1;
	
	token = strtok_r(line, " ", &svgptr);
    if (token != NULL)
    {
        argv[argc++]=token;
		while ( argc<=MAX_CMD_ARGS && (token = strtok_r(NULL, " ", &svgptr)) != NULL  )
		{
			argv[argc++]=token;
		}
		
		for ( i=0; i<strlen(argv[0]); i++)
		{
			c=&argv[0][i];
			if ( *c>='a' && *c<='z' )
				*c= *c-32;
		}

		for ( cmd = XHCP_commandList; cmd->id != END_CMD; cmd++ )
			if ( strcmp (argv[0], cmd->str) == 0 ) break;
			
		if ( cmd->id == END_CMD )
		{
			XHCP_printXHCPMessage(conn, RES_COMNOTREC );  // 500 Command not recognised
			retValue = 1;
		}
		else
		{
			if ( cmd->fnct == NULL )
			{
			XHCP_printXHCPMessage(conn, RES_INTNERROR );  // 503 Internal error - command not performed ----- Pour l'instant !!!
			retValue = 1;
			}
			else
				retValue = cmd->fnct(argc, argv);
		}
		

 printf("%d arguments :\n",argc);
	for ( i=0; i<argc; i++ )
		printf( "%d - %s\n",i, argv[i]);

		
	}
	
	free(line);
	
	return retValue;
}

int getXHCPRequest(int conn)
{
    
    char   buffer[MAX_REQ_LINE] = {0};
    int    rval;
    fd_set fds;
    struct timeval tv;
	int status = 0;
    
    
    /*  Set timeout to 15 seconds  */
    tv.tv_sec  = 15;
    tv.tv_usec = 0;
    
    
    /*  Loop through request headers. If we have a simple request,
    then we will loop only once. Otherwise, we will loop until
    we receive a blank line which signifies the end of the headers,
    or until select() times out, whichever is sooner.                */
    do
    {
        /*  Reset file descriptor set  */
        FD_ZERO(&fds);
        FD_SET (conn, &fds);
        
        /*  Wait until the timeout to see if input is ready  */
        rval = select(conn + 1, &fds, NULL, NULL, &tv);
        
        
        /*  Take appropriate action based on return from select()  */
        if ( rval < 0 )
		{
            Error_Quit("Error calling select() in get_request()");
			status = -1;
		}
        else if ( rval == 0 )
        {
            /*  input not ready after timeout  */
			XHCP_printXHCPMessage(conn, RES_CONTIMOUT );  // 221 Connexion time-out
			status = 0;
        }
        else
        {
            /*  We have an input line waiting, so retrieve it  */
            Readline(conn, buffer, MAX_REQ_LINE - 1);
            Trim(buffer);
            
            if ( buffer[0] == '\0' )
			{
				status=1;
			}
			else
			{
				printf("Ligne lue : %s\n", buffer);
				status = Parse_Line(conn, buffer); // On traite la ligne...
			}

		}
    } while ( status > 0 );
    
    return status;
}


void customWelcomeMessage()
{

	XHCP_response *resp;
	char *hostName;
	char *message;
	char buffer[256];

	gethostname(buffer, 256);
	hostName = strdup(buffer);
	
	sprintf(buffer,"%s.%s Version %s XHCP %s ready",XHCP_HAL_vendor, hostName, XHCP_HAL_version, XHCP_version);
	char response_msg[256];	
	
	for ( resp = &XHCP_responseList[0]; resp->id != END_RES && resp->id != RES_HALWELCOM; resp++);
	
	resp->str=strdup(buffer);
	
	free (hostName);
	
}

int XHCP_server(int argc, char *argv[])
{
    int    listener, conn;
    pid_t  pid;
    struct sockaddr_in servaddr;
	int retStatus;
    

    
	
	
    /*  Create socket  */
    if ( (listener = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
		Error_Quit("Couldn't create listening socket.");
    
    
    /*  Populate socket address structure  */
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(XHCP_SERVER_PORT);
    
    
    /*  Assign socket address to socket  */
    if ( bind(listener, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0 )
		Error_Quit("Couldn't bind listening socket.");
    
    
    /*  Make socket a listening socket  */
    if ( listen(listener, LISTENQ) < 0 )
		Error_Quit("Call to listen failed.");
    
    
	customWelcomeMessage();
	
	
    /*  Loop infinitely to accept and service connections  */
    
    while ( 1 )
    {
        
        /*  Wait for connection  */
        
        if ( (conn = accept(listener, NULL, NULL)) < 0 )
			Error_Quit("Error calling accept()");
        
        XHCP_printXHCPMessage(conn, RES_HALWELCOM );  // 
		//XHCP_printCustomMessage(conn, 200, "%s.%s Version %s XHCP %s ready",XHCP_HAL_vendor, XHCP_HAL_device, XHCP_HAL_version, XHCP_version);
	
		while  ( (retStatus = getXHCPRequest(conn)) > 0 );
		
		if ( retStatus == 0 )
		{
			XHCP_printXHCPMessage(conn, RES_CLOCONBYE );  // Closing connection - good bye
		}
    
        /*  If we get here, we are still in the parent process,
        so close the connected socket, clean up child processes,
        and go back to accept a new connection.                   */
        
        if ( close(conn) < 0 )
			Error_Quit("Error closing connection socket in parent.");
        
    }
    
    return EXIT_FAILURE;    /*  We shouldn't get here  */
}

EXT_XHCP_SERVER XHCPcmd_QUIT ( int argc, char **argv)
{
	return 0;
}

EXT_XHCP_SERVER XHCPcmd_CAPABILITIES ( int argc, char **argv)
{
	
	return 1;
}
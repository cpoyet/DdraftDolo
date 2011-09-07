

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
//#include <sys/systeminfo.h>
//#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <uuid/uuid.h>

#include <roxml.h>
//#include <xPL.h>

#include "XHCP_server.h"
#include "xPLHal4L.h"

/*  Global macros/variables  */



#define XHCP_version "1.5"

#define LISTENQ              		(1024)
#define XHCP_SERVER_PORT            (3865)

#define XHCP_BUFFER_SZ            (256)

//Because UUID_PRINTABLE_STRING_LENGTH is not defined on Linux libuuid
#ifndef UUID_PRINTABLE_STRING_LENGTH
#define UUID_PRINTABLE_STRING_LENGTH 37
#endif


node_t *domConfig;

int ( *additionalDataHandler) ( int, char * ) = NULL;



void Error_Quit (char const * msg)
{
    fprintf (stderr, "WEBSERV: %s\n", msg);
    exit (EXIT_FAILURE);
}

  
String XHCP_getUuid()
{
  uuid_t uuid_id;
  char buffer[UUID_PRINTABLE_STRING_LENGTH+1];

  uuid_generate(uuid_id);
  uuid_unparse(uuid_id, buffer);
  
  /* Pass a copy off */
  return xPL_StrDup(buffer);
}




/*  Read a line from a socket  */

ssize_t Readline (int sockd, void *vptr, size_t maxlen)
{
    ssize_t n, rc;
    char    c, *buffer;
    
    buffer = vptr;
    
    for ( n = 1; n < maxlen; n++ )
    {
        
        if ( (rc = read (sockd, &c, 1)) == 1 )
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
            Error_Quit ("Error in Readline()");
        }
    }
    
    *buffer = 0;
    return n;
}


/*  Removes trailing whitespace from a string  */

int Trim (char * buffer, int mode)
{
    int n = strlen (buffer) - 1;
    
    while ( !isalnum (buffer[n]) && ( mode == 0 || (mode==1 && buffer[n] != '.' && buffer[n] != '>')) && n >= 0 )
        buffer[n--] = '\0';
    
    return 0;
}

/*  Write a line to a socket  */

ssize_t Writeline (int sockd, const void *vptr, size_t n)
{
    size_t      nleft;
    ssize_t     nwritten;
    const char *buffer;
    
    buffer = vptr;
    nleft  = n;
    
    while ( nleft > 0 )
    {
        if ( (nwritten = write (sockd, buffer, nleft)) <= 0 )
        {
            if ( errno == EINTR )
                nwritten = 0;
            else
                Error_Quit ("Error in Writeline()");
        }
        nleft  -= nwritten;
        buffer += nwritten;
    }
    
    return n;
}


void XHCP_print (int sockd, char *Libelle, ... )
{
    va_list Marker;
    char msg[256];
    
    /* construction du libelle du message */
    va_start ( Marker, Libelle);
    vsprintf (msg, Libelle, Marker);
    va_end ( Marker);
    
    strcat (msg, "\r\n");
    
    Writeline (sockd, msg, strlen (msg));
}


void XHCP_printMessage (int sockd, int messNum, char *Libelle, ... )
{
    va_list Marker;
    char response_msg[256];
    char *tmp;
    
    sprintf (response_msg, "%d ", messNum);
    tmp = response_msg+strlen (response_msg);
    
    /* construction du libelle du message */
    va_start ( Marker, Libelle);
    vsprintf (tmp, Libelle, Marker);
    va_end ( Marker);
    
    strcat (response_msg, "\r\n");
    
    Writeline (sockd, response_msg, strlen (response_msg));
}

void XHCP_printXHCPResponse (int sockd, XHCP_response_id messId )
{
    XHCP_response *resp;
    
    
    for ( resp = &XHCP_responseList[0]; resp->id != END_RES && resp->id != messId; resp++);
    
    //sprintf(response_msg, "%d %s\r\n",resp->num, resp->str);
    
    XHCP_printMessage (sockd, resp->num, resp->str );
    
}

char * toUpper (char *str)
{
    char *c;
    
    for ( c=str; *c != '\0'; c++)
    {
        if ( *c>='a' && *c<='z' )
            *c= *c-32;
    }
    
    return str;
}

int Parse_Line (int conn, char *buffer)
{
    char *line=NULL;
    char *argv[MAX_CMD_ARGS+1];
    int  argc=0;
    
    char *token, *svgptr;
    int i;
    
    XHCP_command *cmd;
    
    int retValue=0;
    
    if ( (line = strdup (buffer)) == NULL )
        return -1;
    
    token = strtok_r (line, " ", &svgptr);
    if (token != NULL)
    {
        argv[argc++]=token;
        while ( argc<=MAX_CMD_ARGS && (token = strtok_r (NULL, " ", &svgptr)) != NULL  )
        {
            argv[argc++]=token;
        }
        
        toUpper (argv[0]);
        
        for ( cmd = XHCP_commandList; cmd->id != END_CMD; cmd++ )
            if ( strcmp (argv[0], cmd->str) == 0 ) break;
        
        if ( cmd->id == END_CMD )
        {
            XHCP_printXHCPResponse (conn, RES_COMNOTREC );  // 500 Command not recognised
            retValue = 1;
        }
        else
        {
            if ( cmd->fnct == NULL )
            {
                //			XHCP_printXHCPResponse(conn, RES_INTNERROR );  // 503 Internal error - command not performed ----- Pour l'instant !!!
                XHCP_printMessage (conn, 500, "Command not implemented" );
                retValue = 1;
            }
            else
                retValue = cmd->fnct (conn, argc, argv);
        }
        
        
        printf ("%d arguments :\n", argc);
        for ( i=0; i<argc; i++ )
            printf ( "%d - %s\n", i, argv[i]);
        
        
    }
    
    free (line);
    
    return retValue;
}

char *addBuffer ( char *buffer, char*str)
{
    int szs;
    int szb;
    
    if ( str == NULL )
        return buffer;
    
    szs = strlen (str);
    
    if ( buffer == NULL )
    {
        buffer = (char *)malloc ((sizeof(char)*szs)+2);
        buffer[0]='\0';
    }
    else
    {
        szb= strlen (buffer);
        buffer = (char *)realloc (buffer, (sizeof(char)*(szs+szb))+2);
    }
    
    sprintf (buffer, "%s%s\n", buffer, str);
    
    return buffer;
}

int getXHCPRequest (int conn)
{
    
    char   buffer[MAX_REQ_LINE] =
    {0};
    int    rval;
    fd_set fds;
    struct timeval tv;
    int status = 0;
    int lastLineIsEmpty = 0;
    char *additionalDataBuffer=NULL;
    
    
    /*  Set timeout  */
    tv.tv_sec  = XHCP_connexionTimeOut;
    tv.tv_usec = 0;
    
    
    /*  Loop through request headers. If we have a simple request,
     then we will loop only once. Otherwise, we will loop until
     we receive a blank line which signifies the end of the headers,
     or until select() times out, whichever is sooner.                */
    do
    {
        /*  Reset file descriptor set  */
        FD_ZERO (&fds);
        FD_SET (conn, &fds);
        
        /*  Wait until the timeout to see if input is ready  */
        rval = select (conn + 1, &fds, NULL, NULL, &tv);
        
        
        /*  Take appropriate action based on return from select()  */
        if ( rval < 0 )
        {
            Error_Quit ("Error calling select() in get_request()");
            status = -1;
        }
        else if ( rval == 0 )
        {
            /*  input not ready after timeout  */
            XHCP_printXHCPResponse (conn, RES_CONTIMOUT );  // 221 Connexion time-out
            status = 0;
        }
        else
        {
            /*  We have an input line waiting, so retrieve it  */
            Readline (conn, buffer, MAX_REQ_LINE - 1);
            
            /* If we are not waiting for additional data the function handler should be not null*/
            if ( additionalDataHandler == NULL )
            {
                Trim (buffer, 0); // We suppress all extra characters
                
                if ( buffer[0] == '\0' )
                    status=1; // We continue....
                else
                {
                    printf ("Ligne lue : %s\n", buffer);
                    status = Parse_Line (conn, buffer); // We compute the line...
                }
            }
            else
            {
                /* We suppress all extra characters on the right except '.' and '>' */
                Trim (buffer, 1); 
                
                /* The handler is activate, so all lignes are added in buffer */
                if ( buffer[0] == '.' &&  buffer[1] == '\0')
                {
                    additionalDataHandler (conn, additionalDataBuffer );
                    lastLineIsEmpty=0;
                    additionalDataHandler = NULL;
                    free (additionalDataBuffer);
                    additionalDataBuffer = NULL;
                }
                else if ( buffer[0] == '\0' )
                {
                    /* Haha... a empty line ? We note it in case when the nest line contain a '.' */
                    lastLineIsEmpty=1;
                    additionalDataBuffer = addBuffer (additionalDataBuffer, buffer);
                }
                else
                {
                    /* Adding the line to the data buffer */
                    lastLineIsEmpty=0;
                    additionalDataBuffer = addBuffer (additionalDataBuffer, buffer);
                }
                
                
            }
            
            
        }
    } while ( status > 0 );
    
    return status;
}


void XHCP_customWelcomeMessage ()
{
    
    XHCP_response *resp;
    char buffer[256];
    
    sprintf (buffer, "%s.%s (on %s/%s) Version %s XHCP %s ready", XPLHAL4L_VENDOR, XHCP_hostName, XHCP_sysName, XHCP_sysArchi, XPLHAL4L_VERSION, XHCP_version);
    
    for ( resp = &XHCP_responseList[0]; resp->id != END_RES && resp->id != RES_HALWELCOM; resp++);
    
    resp->str=strdup (buffer);
    
}

void XHCP_getSystemInfos ()
{
    
    char buffer[256];
    
    struct utsname sys_infos;
    
    
    if ( (uname (&sys_infos)) < 0 )
        Error_Quit ("Couldn't read system informations.");
    
    XHCP_hostName = strdup (sys_infos.nodename);
    XHCP_sysName = strdup (sys_infos.sysname);
    XHCP_sysArchi = strdup (sys_infos.machine);
}

int loadConfig (node_t* argXmlConfig)
{
    node_t **result;
    int nb_result;
    char buffer[XHCP_BUFFER_SZ];
    int sz_buffer;
    char *xhcp_fileName;
    
    printf ("Loading XHCP server configuration...\n");
    
    if ( (xhcp_fileName = XHCP_getConfigFile ()) != NULL )
        domConfig = roxml_load_doc (xhcp_fileName);
    else if ( argXmlConfig != NULL )
        domConfig = argXmlConfig;
    else
        Error_Quit ("Unable to find XHCP configuration");
    
    
    /* Time Out */
    result = roxml_xpath ( domConfig, "//XHCPserver/ConnectionTimeOut[@delay]", &nb_result);
    if ( nb_result == 1 )
    {
        char *zaza = roxml_get_content ( roxml_get_attr (result[0], "delay", 0), buffer, XHCP_BUFFER_SZ, &sz_buffer );
        XHCP_connexionTimeOut = atoi (zaza);
        
        if ( XHCP_connexionTimeOut <= 0 )
            XHCP_connexionTimeOut=2;
    }
    else if ( nb_result == 0)
    {
        XHCP_connexionTimeOut = 5;
    }
    else
        Error_Quit ("Erroe parsing XHCP config file (ConnectionTimeOut)");
    
    roxml_release (RELEASE_LAST);
    printf ("XHCP_connexionTimeOut = %d\n", XHCP_connexionTimeOut);
    
    
    return 0;
}

int XHCP_server (node_t* argXmlConfig)
{
    int    listener, conn;
    pid_t  pid;
    struct sockaddr_in servaddr;
    int retStatus;
    
    
    XHCP_getSystemInfos ();
    
    loadConfig (argXmlConfig);
    
    
    
    /*  Create socket  */
    if ( (listener = socket (AF_INET, SOCK_STREAM, 0)) < 0 )
        Error_Quit ("Couldn't create listening socket.");
    
    
    /*  Populate socket address structure  */
    memset (&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl (INADDR_ANY);
    servaddr.sin_port        = htons (XHCP_SERVER_PORT);
    
    
    /*  Assign socket address to socket  */
    if ( bind (listener, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0 )
        Error_Quit ("Couldn't bind listening socket.");
    
    
    /*  Make socket a listening socket  */
    if ( listen (listener, LISTENQ) < 0 )
        Error_Quit ("Call to listen failed.");
    
    
    XHCP_customWelcomeMessage ();
    
    
    /*  Loop infinitely to accept and service connections  */
    
    while ( 1 )
    {
        
        /*  Wait for connection  */
        
        if ( (conn = accept (listener, NULL, NULL)) < 0 )
            Error_Quit ("Error calling accept()");
        
        XHCP_printXHCPResponse (conn, RES_HALWELCOM );  //
        
        while  ( (retStatus = getXHCPRequest (conn)) > 0 );
        
        if ( retStatus == 0 )
        {
            XHCP_printXHCPResponse (conn, RES_CLOCONBYE );  // Closing connection - good bye
        }
        
        /*  If we get here, we are still in the parent process,
         so close the connected socket, clean up child processes,
         and go back to accept a new connection.                   */
        
        if ( close (conn) < 0 )
            Error_Quit ("Error closing connection socket in parent.");
        
    }
    
    return EXIT_FAILURE;    /*  We shouldn't get here  */
}

int XHCPcmd_QUIT (int sockd, int argc, char **argv)
{
    return 0;
}

int XHCPcmd_CAPABILITIES (int sockd, int argc, char **argv)
{
    int s = 0;
    
    
    if ( argc>1)
    {
        toUpper (argv[1]);
        
        if ( strcmp (argv[1], "SCRIPTING") == 0 )
            s = 1;
        else
        {
            XHCP_printXHCPResponse (sockd, RES_SYNTAXERR );  // Syntax error
            return 1;
        }
    }
    
    XHCP_printMessage (sockd, s?241:236, "--000U0" );
    
    if (s)
        XHCP_print (sockd, "." );
    
    
    return 1;
}

int XHCPcmd_PUTCONFIGXML_handle (int sockd, char *cfgData)
{
    node_t *tmp;
    printf ("Entree XHCPcmd_PUTCONFIGXML_handle\n");
    if ( (tmp = roxml_load_buf (cfgData)) == NULL )
    {
        XHCP_printXHCPResponse (sockd, RES_SYNTAXERR); // Syntax Error
        return -1;
    }
    
    printf ("Chargement XML OK\n");
    roxml_close (rootConfig);
    printf ("Close OK\n");
    rootConfig = tmp;
    
    
    loadConfig (rootConfig);
    printf ("loadConfig OK\n");
    
    XHCP_printXHCPResponse (sockd, RES_CFGDOCUPL ); // Configuration document uploaded
    
    return 0;
}

int XHCPcmd_PUTCONFIGXML (int sockd, int argc, char **argv)
{
    XHCP_printXHCPResponse (sockd, RES_ENTCFGDOC ); // Enter configuration document, end with <CrLf>.<CrLf>
    
    XHCP_setAdditionalDataHandler (XHCPcmd_PUTCONFIGXML_handle);
    
    return 1;
}

int XHCPcmd_LISTRULES (int sockd, int argc, char **argv)
{
    node_t **rulesNodesLst;
    char buffer[XHCP_BUFFER_SZ];
    int sz_buffer;
    int i, nb;
    
    XHCP_printXHCPResponse (sockd, RES_LSTDTRFOL ); // List of Determinator Rules follows
    
    if ( (rulesNodesLst = roxml_xpath (rootConfig, "//determinator", &nb )) !=NULL )
    {
        for (i=0; i<nb; i++)
        {
            char *champ1 = roxml_get_content ( roxml_get_attr (rulesNodesLst[i], "guid", 0), buffer, XHCP_BUFFER_SZ, &sz_buffer );
            char *champ2 = roxml_get_content ( roxml_get_attr (rulesNodesLst[i], "name", 0), champ1 + sz_buffer + 1, XHCP_BUFFER_SZ, &sz_buffer );
            char *champ3 = roxml_get_content ( roxml_get_attr (rulesNodesLst[i], "enabled", 0), champ2 + sz_buffer + 1, XHCP_BUFFER_SZ, &sz_buffer );
 
			XHCP_print(sockd, "%s\t%s\t%s",champ1,champ2,champ3);
        }
    }
    
    XHCP_print(sockd, ".");
    return 1;
}


int xhcp_roxml_commit_changes(node_t *n, char ** buffer)
{
	int size = 0;
	int len = 0;
	FILE *fout = NULL;
int XHCP_ROXML_LONG_LEN = 512;
	
	
	if(n) {
		len = XHCP_ROXML_LONG_LEN;
			*buffer = (char*)malloc(XHCP_ROXML_LONG_LEN);
			memset(*buffer, 0, XHCP_ROXML_LONG_LEN);

		roxml_write_node(n, fout, buffer, 1, 0, &size, &len);

			char * ptr = NULL;
			len -= XHCP_ROXML_LONG_LEN;
			ptr = *buffer + len;
			len += strlen(ptr);


	}

	return len;
}



int XHCPcmd_GETRULE (int sockd, int argc, char **argv)
{
    node_t **rulesNodesLst;
    char buffer[XHCP_BUFFER_SZ];
    int sz_buffer;
    int nb;
	
	char *writeBuffer = NULL;
    
    
	if (argc != 2)
	{
		XHCP_printXHCPResponse (sockd, RES_SYNTAXERR ); // Syntax Error
		return 1;
	}
	
    sprintf(buffer,"//determinator[@guid='%s']",argv[1]);
	
	printf("%s\n",buffer);
	
    if ( (rulesNodesLst = roxml_xpath (rootConfig, buffer, &nb )) == NULL )
		XHCP_printXHCPResponse (sockd, RES_NOSUCHSCR ); // No such script/rule
	else
    {
		printf("%d rules trouvé\n",nb);
		XHCP_printXHCPResponse (sockd, RES_REQSCRFOL ); // Requested script/rule follows
	
		//sz_buffer = roxml_commit_changes(rulesNodesLst[0], NULL, &writeBuffer, 1);
		sz_buffer = xhcp_roxml_commit_changes(rulesNodesLst[0],&writeBuffer);
		printf("sz=%d len=%d\n",sz_buffer,strlen(writeBuffer));
		
		printf("%s\n",writeBuffer);
		XHCP_print(sockd, writeBuffer);
		
		XHCP_print(sockd, ".");
		
		free(writeBuffer);
    }

	
    
    return 1;
}





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
#include <sys/utsname.h>
#include <uuid/uuid.h>

#include <roxml.h>

#include "XHCP_server.h"
#include "xPLHal4L.h"

/*  Global macros/variables  */
#define XHCP_version "1.5"

#define LISTENQ                  	(1024)
#define XHCP_SERVER_PORT            (3865)

#define XHCP_BUFFER_SZ            (256)

//Because UUID_PRINTABLE_STRING_LENGTH is not defined in Linux uuid lib
#ifndef UUID_PRINTABLE_STRING_LENGTH
#define UUID_PRINTABLE_STRING_LENGTH 37
#endif





node_t *domConfig;



int ( *additionalDataHandler) ( int hdl, int argc, char **argv, char *data ) = NULL;



void Error_Quit (char const * msg)
{
    fprintf (stderr, "WEBSERV: %s\n", msg);
    exit (EXIT_FAILURE);
}


String XHCP_getUuid ()
{
    uuid_t uuid_id;
    char buffer[UUID_PRINTABLE_STRING_LENGTH+1];
    
    uuid_generate (uuid_id);
    uuid_unparse (uuid_id, buffer);
    
    /* Pass a copy off */
    //return xPL_StrDup (buffer);
    return strdup (buffer);
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
    //char msg[256];
    
    int sz_alloc=strlen (Libelle) + 256;
    int sz_msg;
    
    char *msg = (char *)malloc (sz_alloc);
    msg[0]='\0';
    
    /* construction du libelle du message */
    va_start ( Marker, Libelle);
    
    sz_msg=strlen (msg);
    if ( sz_alloc-sz_msg<128 )
    {
        sz_alloc += 128;
        msg = (char *)realloc (msg, sz_alloc);
    }
    vsprintf (msg, Libelle, Marker);
    
    va_end ( Marker);
    
    Writeline (sockd, msg, strlen (msg));
    
    sz_msg=strlen (msg);
    if ( msg[sz_msg-1] != '\n' )
        Writeline (sockd, "\r\n", 2);
    
    free (msg);
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
/*
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
 XHCP_printXHCPResponse (conn, RES_COMNOTREC );
 retValue = 1;
 }
 else
 {
 if ( cmd->fnct == NULL )
 {
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
 */

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

int cut_Line (char *buffer, int *argc, char **argv)
{
    int count = 0;
    char *token, *svgptr;
	static char* lBuff=NULL;
    
	if ( lBuff != NULL )
		free (lBuff);
	lBuff= strdup(buffer);
	
    token = strtok_r (lBuff, " ", &svgptr);
    if (token != NULL)
    {
        argv[count++]=token;
        while ( count<=MAX_CMD_ARGS && (token = strtok_r (NULL, " ", &svgptr)) != NULL  )
        {
            argv[count++]=token;
        }
        
        toUpper (argv[0]);
    }
    *argc=count;
    return count;
}

int exec_Line (int conn, int argc, char **argv)
{
    
    XHCP_command *cmd;
    
    int retValue=0;
    
    
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
            /*XHCP_printXHCPResponse(conn, RES_INTNERROR );  // 503 Internal error - command not performed ----- Pour l'instant !!!*/
            XHCP_printMessage (conn, 500, "Command not implemented" );
            retValue = 1;
        }
        else
            retValue = cmd->fnct (conn, argc, argv);
    }
    
    
    return retValue;
}

int _getXHCPRequest (int conn)
{
    
    char   buffer[MAX_REQ_LINE] = {0};
    int    rval;
    fd_set fds;
    struct timeval tv;
    int status = 0;
    char *additionalDataBuffer=NULL;
    int argc = 0;
    char *argv[MAX_CMD_ARGS+1];
    
    
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
                    
                    cut_Line (buffer, &argc, argv);
                    
                    printf ("%d arguments :\n", argc);
                    int i;
                    for ( i=0; i<argc; i++ )
                        printf ( "%d - %s\n", i, argv[i]);
                    
                    status = exec_Line (conn, argc, argv ); // We compute the line...
                }
            }
            else
            {
                /* We suppress all extra characters on the right except '.' and '>' */
                Trim (buffer, 1);
                
                /* The handler is activate, so all lignes are added in buffer */
                if ( buffer[0] == '.' &&  buffer[1] == '\0')
                {
                    additionalDataHandler (conn, argc, argv, additionalDataBuffer );
                    additionalDataHandler = NULL;
                    free (additionalDataBuffer);
                    additionalDataBuffer = NULL;
                }
                else
                {
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
    
    sprintf (buffer, "%s.%s (%s/%s) Version %s XHCP %s ready", XPLHAL4L_VENDOR, XHCP_hostName, XHCP_sysName, XHCP_sysArchi, XPLHAL4L_VERSION, XHCP_version);
    
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

int XHCP_loadConfig (node_t* argXmlConfig)
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
    static int listener;
	static int conn;
    static pid_t  pid;
    static struct sockaddr_in servaddr;

    static char *additionalDataBuffer=NULL;
    static int argc = 0;
    static char *argv[MAX_CMD_ARGS+1];
   
    static char	buffer[MAX_REQ_LINE] = {0};
	int		status, nbCar;

	
	switch (XHCP_running_status)
	{
		/* ------------------------------------------------------------------------ */
		case (XHCPstate_init):

			XHCP_getSystemInfos ();
			
			XHCP_loadConfig(argXmlConfig);
    
    
			/*  Create socket  */
			if ( (listener = socket (AF_INET, SOCK_STREAM, 0)) < 0 )
				Error_Quit ("Couldn't create listening socket.");
			
			
			/*  Populate socket address structure  */
			memset (&servaddr, 0, sizeof(servaddr));
			servaddr.sin_family      = AF_INET;
			servaddr.sin_addr.s_addr = htonl (INADDR_ANY);
			servaddr.sin_port        = htons (XHCP_SERVER_PORT);

			
			/* "Address already in use" error message killer !!!! */
			int tr=1;
			if (setsockopt(listener,SOL_SOCKET,SO_REUSEADDR,&tr,sizeof(int)) == -1)
			{
				perror("setsockopt");
				exit(1);
			}
			
			/*  Assign socket address to socket  */
			if ( bind (listener, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0 )
			{
				perror("Bind");
				Error_Quit ("Couldn't bind listening socket.");
			}
			
			/*  Make socket a listening socket  */
			if ( listen (listener, LISTENQ) < 0 )
				Error_Quit ("Call to listen failed.");
			
			
			XHCP_customWelcomeMessage ();
		
			/* L'initialisation est terminée, on passe à la suite */
			XHCP_running_status = XHCPstate_waitConnect;
			
			/* No break, we continue !!!*/
			
		/* ------------------------------------------------------------------------ */
		case (XHCPstate_waitConnect):
		
			/*  Wait for connection  */
			if ( (conn = accept (listener, NULL, NULL)) < 0 )
			{
				if ( (errno == EWOULDBLOCK) || (errno == EAGAIN) )
				{
					return XHCP_running_status;
				}
				else
					Error_Quit ("Error calling accept()");
			
			}
			//TODO Gestion du timeout
			
			XHCP_printXHCPResponse (conn, RES_HALWELCOM );  // Petit message de bienvenue
			
			XHCP_running_status = XHCPstate_waitCommand;
		
			/* No break, we continue !!!*/
			
		/* ------------------------------------------------------------------------ */
		case (XHCPstate_waitCommand):
		
			if ( (nbCar = recv(conn, buffer, MAX_REQ_LINE - 1, MSG_DONTWAIT)) <0 )
			{
				if ( (errno == EWOULDBLOCK) || (errno == EAGAIN) )
				{
					//TODO Gestion du timeout
					return XHCP_running_status;
				}
				else
					Error_Quit ("Error calling accept()");
			
			}
			
			buffer[nbCar]='\0';

            Trim (buffer, 0); // We suppress all extra characters
                
			if ( buffer[0] == '\0' )
				return XHCP_running_status; // We continue....

			printf ("Ligne lue : %s\n", buffer);

			cut_Line (buffer, &argc, argv);

			printf ("%d arguments :\n", argc);
			int i;
			for ( i=0; i<argc; i++ )
				printf ( "%d - %s\n", i, argv[i]);

			status = exec_Line (conn, argc, argv ); // We compute the line...
			printf("Ligne executee, statut = %d\n",status);
			switch (status)
			{
				case -1:  // deconnexion
					XHCP_running_status = XHCPstate_endConnect;
					return XHCP_running_status;
					break;
				case 0:   // Fin de la commande
					return XHCP_running_status;
					break;
				// default : // On continue
			}

			XHCP_running_status = XHCPstate_waitData;
			
			/* No break, we continue !!!*/

		/* ------------------------------------------------------------------------ */
		case (XHCPstate_waitData):
			
			if ( (nbCar = recv(conn, buffer, MAX_REQ_LINE - 1, MSG_DONTWAIT)) <0 )
			{
				if ( (errno == EWOULDBLOCK) || (errno == EAGAIN) )
				{
					//TODO Gestion du timeout
					return XHCP_running_status;
				}
				else
					Error_Quit ("Error calling accept()");
			
			}

			buffer[nbCar]='\0';
			/* We suppress all extra characters on the right except '.' and '>' */
			Trim (buffer, 1);

			/* The handler is activate, so all lignes are added in buffer */
			if ( buffer[0] == '.' &&  buffer[1] == '\0')
			{
				additionalDataHandler (conn, argc, argv, additionalDataBuffer );
				additionalDataHandler = NULL;
				free (additionalDataBuffer);
				additionalDataBuffer = NULL;

				XHCP_running_status = XHCPstate_waitCommand;
			}
			else
			{
				additionalDataBuffer = addBuffer (additionalDataBuffer, buffer);
			}

			
			break;
			
		/* ------------------------------------------------------------------------ */
		case (XHCPstate_endConnect):
		
			XHCP_printXHCPResponse (conn, RES_CLOCONBYE ); 
			 
		    if ( close (conn) < 0 )
				Error_Quit ("Error closing connection socket in parent.");

			XHCP_running_status = XHCPstate_waitConnect;

			
		//default :  /* (XHCPstate_death) */
			/* Do nothing ... */
			
	}
	
	return XHCP_running_status;
}


int _XHCP_server (node_t* argXmlConfig)
{
    int    listener, conn;
    pid_t  pid;
    struct sockaddr_in servaddr;
    int retStatus;
    
    
    XHCP_getSystemInfos ();
    
    XHCP_loadConfig (argXmlConfig);
    
    
    
    /*  Create socket  */
    if ( (listener = socket (AF_INET, SOCK_STREAM, 0)) < 0 )
        Error_Quit ("Couldn't create listening socket.");
    
    
    /*  Populate socket address structure  */
    memset (&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl (INADDR_ANY);
    servaddr.sin_port        = htons (XHCP_SERVER_PORT);
    


    /* "Address already in use" error message killer !!!! */
    int tr=1;
    if (setsockopt(listener,SOL_SOCKET,SO_REUSEADDR,&tr,sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }    
    /*  Assign socket address to socket  */
    if ( bind (listener, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0 )
    {
        perror("Bind");
        Error_Quit ("Couldn't bind listening socket.");
    }
    
    /*  Make socket a listening socket  */
    if ( listen (listener, LISTENQ) < 0 )
        Error_Quit ("Call to listen failed.");
    
    
    XHCP_customWelcomeMessage ();
    
    
    /*  Loop infinitely to accept and service connections  */
    
    while ( ! stop )
    {
        
        /*  Wait for connection  */
        
        if ( (conn = accept (listener, NULL, NULL)) < 0 )
            Error_Quit ("Error calling accept()");
        
        XHCP_printXHCPResponse (conn, RES_HALWELCOM );  //
        
        while  ( (retStatus = _getXHCPRequest (conn)) > 0 );
        
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
    
    return 0;
}

int XHCPcmd_QUIT (int sockd, int argc, char **argv)
{
    return -1;
}

int XHCPcmd_SHUTDOWN (int sockd, int argc, char **argv)
{
    stop = 1;
    
    XHCP_printMessage (sockd, 221, "Shuting down in progress ... Good night... !!" );
    
    return -1;
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
    
    
    return 0;
}

int XHCPcmd_PUTCONFIGXML_handle (int sockd, int argc, char **argv, char *data)
{
    node_t *tmp;
    printf ("Entree XHCPcmd_PUTCONFIGXML_handle\n");
    if ( (tmp = roxml_load_buf (data)) == NULL )
    {
        XHCP_printXHCPResponse (sockd, RES_SYNTAXERR); // Syntax Error
        return -1;
    }
    
    printf ("Chargement XML OK\n");
    roxml_close (rootConfig);
    printf ("Close OK\n");
    rootConfig = tmp;
    
    
    XHCP_loadConfig (rootConfig);
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
            
            XHCP_print (sockd, "%s\t%s\t%s", champ1, champ2, champ3);
        }
    }
    
    XHCP_print (sockd, ".");
    return 0;
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
    
    sprintf (buffer, "//determinator[@guid='%s']", argv[1]);
    
    printf ("%s\n", buffer);
    
    if ( (rulesNodesLst = roxml_xpath (rootConfig, buffer, &nb )) == NULL )
        XHCP_printXHCPResponse (sockd, RES_NOSUCHSCR ); // No such script/rule
    else
    {
        XHCP_printXHCPResponse (sockd, RES_REQSCRFOL ); // Requested script/rule follows
        
        sz_buffer = roxml_commit_changes (rulesNodesLst[0], NULL, &writeBuffer, 1);
        
        XHCP_print (sockd, writeBuffer);
        XHCP_print (sockd, ".");
        
        free (writeBuffer);
    }
    
    
    
    return 0;
}

int XHCPcmd_SETRULE_handle (int sockd, int argc, char **argv, char *data)
{
    node_t *nTmp;
    node_t **lstNodes;
    char *newId = NULL;
    int nb;

	// A supprimer !!!
    char *zaza = NULL;

    printf ("Entree XHCPcmd_SETRULE_handle avec %d arguments\n", argc);
                   int i;
                    for ( i=0; i<argc; i++ )
                        printf ( "%d - %s\n", i, argv[i]);
 


    if ( (nTmp = roxml_load_buf (data)) == NULL )
    {
        XHCP_printXHCPResponse (sockd, RES_SYNTAXERR); // Syntax Error
        return 0;
    }
    
    printf ("Chargement XML OK\n");
    
	if ( argc == 2 ) // pas de node mais id donné en argument
		newId = strdup(argv[1]);

		// On recherche si un id est présent
    if ( (lstNodes = roxml_xpath (nTmp, "//determinator[@guid]", &nb )) != NULL )
	{
		node_t *attr_tmp = roxml_get_attr (lstNodes[0], "guid", 0);

		if ( newId == NULL && argc == 1 )
		{
			newId = roxml_get_content(attr_tmp, NULL, 0, NULL);
		}

		roxml_del_node(attr_tmp);
	}
	if ( newId == NULL )
		newId = XHCP_getUuid();

printf("a la fin id = %s\n",newId==NULL ? "NULL":newId);
		
	lstNodes = roxml_xpath (nTmp, "//determinator", &nb );
	if ( nb == 1 )
		nTmp = lstNodes[0];
	else
    {
printf("Pas trouve //derterminator, nb=%d\n",nb);
        XHCP_printXHCPResponse (sockd, RES_SYNTAXERR); // Syntax Error
        return 0;
    }
	
	// Ajout d'un nouveau noeud
	roxml_add_node(nTmp, 0, ROXML_ATTR_NODE, "guid", newId);		
printf("attribut ajouté\n");

printf("Nouvel arbre determinator\n");
roxml_commit_changes (nTmp, NULL, &zaza, 1);
printf("%s\n",  zaza);
free(zaza);




	lstNodes = roxml_xpath (rootConfig, "//determinators", &nb );
	if ( nb != 1 )
    {
printf("Pas trouve //determinators, nb=%d\n",nb);
        XHCP_printXHCPResponse (sockd, RES_INTNERROR); // Internal error
        return 0;
    }

	/* On ratache le nouveau determinator à la liste */
	roxml_parent_node(lstNodes[0], nTmp);
		
	
	saveHal4lConfig (HAL4L_getConfigFile ());
	//roxml_close (nTmp);
	
	
    loadHal4lConfig (HAL4L_getConfigFile ());


    free(newId);


    XHCP_printXHCPResponse (sockd, RES_CFGDOCUPL ); // Configuration document uploaded
    
    return 0;
}

int XHCPcmd_SETRULE (int sockd, int argc, char **argv)
{
    XHCP_printXHCPResponse (sockd, RES_SENDDRULE ); // Send rule, end with <CrLf>.<CrLf>
    
    XHCP_setAdditionalDataHandler (XHCPcmd_SETRULE_handle);
    
    return 1;
}

int XHCPcmd_GETCONFIGXML (int sockd, int argc, char **argv)
{
    char *writeBuffer = NULL;
    
	XHCP_printXHCPResponse (sockd, RES_CFGDOCFOL ); // Configuration document follows
	
	roxml_commit_changes (rootConfig, NULL, &writeBuffer, 1);
	
	XHCP_print (sockd, writeBuffer);
	XHCP_print (sockd, ".");
	
	free (writeBuffer);

    return 0;
}


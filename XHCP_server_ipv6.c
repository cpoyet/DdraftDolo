
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
#include <uuid/uuid.h>
#include <fcntl.h>

#include <roxml.h>

#include "XHCP_server_ipv6.h"
#include "xPLHal_common.h"
#include "xPLHal4L.h"

/*  Global macros/variables  */
#define XHCP_version "1.5"

#define LISTENQ                  	(1024)
#define XHCP_SERVER_PORT            "3865"

#define XHCP_BUFFER_SZ            (256)

//Because UUID_PRINTABLE_STRING_LENGTH is not defined in Linux uuid lib
#ifndef UUID_PRINTABLE_STRING_LENGTH
#define UUID_PRINTABLE_STRING_LENGTH 37
#endif


#define XPLHAL_ROOT_PAGE "<html>\
<head>\
<meta content=\"text/html; charset=ISO-8859-1\"\
http-equiv=\"content-type\">\
<title></title>\
</head>\
<body>\
<table style=\"text-align: left; width: 774px; height: 449px;\" border=\"1\"\
cellpadding=\"2\" cellspacing=\"2\">\
<tbody>\
<tr>\
<td style=\"vertical-align: top; height: 57px;\"><span\
style=\"font-family: monospace;\">&nbsp;xPLHal4L.foxg20\
(Linux/armv5tejl) Version 0.0.1 XHCP 1.5 ready<br>\
</span><br>\
</td>\
</tr>\
</tbody>\
</table>\
<br>\
</body>\
</html>"




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
    size_t      nleft, nsent;
    ssize_t     nwritten;
    const char *buffer;
    
    buffer = vptr;
    nleft  = n;
	nsent = 0;
    
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
		nsent += nwritten;
    }
    
    return nsent;
}


void XHCP_print (int sockd, char *Libelle, ... )
{
    va_list Marker;
    //char msg[256];
	ssize_t wcar;
    
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
    
    wcar = Writeline (sockd, msg, strlen (msg));
	//printf("XHCP_print print Ligne [%i] char writen\n",wcar);
    
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
        //retValue = 1;
    }
    else
    {
        if ( cmd->fnct == NULL )
        {
            /*XHCP_printXHCPResponse(conn, RES_INTNERROR );  // 503 Internal error - command not performed ----- Pour l'instant !!!*/
            XHCP_printMessage (conn, 500, "Command not implemented" );
            //retValue = 1;
        }
        else
            retValue = cmd->fnct (conn, argc, argv);
    }
    
    
    return retValue;
}

void XHCP_customWelcomeMessage ()
{
    
    XHCP_response *resp;
    char buffer[256];
    
    sprintf (buffer, "%s.%s (%s/%s) Version %s XHCP %s ready", XPLHAL4L_VENDOR, HAL4L_hostName, HAL4L_sysName, HAL4L_sysArchi, XPLHAL4L_VERSION, XHCP_version);
    
    for ( resp = &XHCP_responseList[0]; resp->id != END_RES && resp->id != RES_HALWELCOM; resp++);
    
    resp->str=strdup (buffer);
    
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

int processHttpRequest(int sockd, char *buffer)
{
    char *sep = "\n";
    char *line, *brkt;
    char *responseBuffer=NULL;


	for (line = strtok_r(buffer, sep, &brkt); line; line = strtok_r(NULL, sep, &brkt))
	{
		responseBuffer = addBuffer (responseBuffer, line);
		printf("Ligne HTTP : [%s]\n",line);
	}
	printf("Buffer : %i car. [%s]\n",strlen(responseBuffer), responseBuffer);
	//responseBuffer = "coucou xPLHal est là !!";
	//XHCP_print (sockd, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %i\r\n\r\n%s",strlen(responseBuffer),responseBuffer);
	
	responseBuffer = XPLHAL_ROOT_PAGE;
	
	Writeline (sockd, responseBuffer, strlen(responseBuffer));

	//free(responseBuffer);
	return 0;
}

int XHCP_server (node_t* argXmlConfig)
{
	static const int zero=0;
	
    static int listener;
	static int conn;
    static pid_t  pid;
    //static struct sockaddr_in servaddr;
    static struct sockaddr_storage servaddr;
	
	static struct timeval time_conn, time_resp;
	
    static char *additionalDataBuffer=NULL;
    static int argc = 0;
    static char *argv[MAX_CMD_ARGS+1];
   
    static char	buffer[MAX_REQ_LINE] = {0};
	
	int		status, nbCar;
	// Variables non persistantes
	int sockV4, sockV6;
	struct addrinfo hints,  *list_addr, *p_adresse, *aiv4, *aiv6, *choix_ai;
	
	switch (XHCP_running_status)
	{
		/* ------------------------------------------------------------------------ */
		case (XHCPstate_init):

			XHCP_loadConfig(argXmlConfig);
    
    
			/*  Create socket  */
		//	if ( (listener = socket (AF_INET, SOCK_STREAM, 0)) < 0 )
		//		Error_Quit ("Couldn't create listening socket.");
			
			
			/*  Populate socket address structure  */
			memset (&hints, 0, sizeof(hints));
			hints.ai_family      = AF_UNSPEC;
			hints.ai_socktype     = SOCK_STREAM;
			hints.ai_flags        = AI_PASSIVE;
			if ( (getaddrinfo(NULL, XHCP_SERVER_PORT, &hints, &list_addr)) < 0 )
			{
				//printf("getaddrinfo: %s\n",gai_strerroe(flags));
				perror("getaddrinfo");
				exit(1);
			}
			
			sockV4=-1; sockV6=-1;
			p_adresse = list_addr;
			
			// Vérification des protocoles en testant l'ouverture d'une socket
			while (p_adresse)
			{
				if ( p_adresse->ai_family == AF_INET6)
				{
					if ( sockV6 < 0 )
					{
						printf("Scanning IPV6...");
						if ( (sockV6=socket(p_adresse->ai_family, p_adresse->ai_socktype, p_adresse->ai_protocol)) >= 0 )
						{
							aiv6 = p_adresse;
							printf(" found");
						}
						printf("\n");
					}
				}
				else if ( p_adresse->ai_family == AF_INET)
				{
					if ( sockV4 < 0 )
					{
						printf("Scanning IPV4...");
						if ( (sockV4=socket(p_adresse->ai_family, p_adresse->ai_socktype, p_adresse->ai_protocol)) >= 0 )
						{
							aiv4 = p_adresse;
							printf(" found");
						}
						printf("\n");
					}
				}
			p_adresse = p_adresse->ai_next;
			}

			// Selection de la socket
			if ( sockV6 >= 0 )
			{
				choix_ai = aiv6;
				listener = sockV6;
				// Tentative d'activation de la traduction IPV6->IPV4
				if ( setsockopt(sockV6, IPPROTO_IPV6, IPV6_V6ONLY, &zero, sizeof(zero)) < 0 )
				{
					perror("notice : setsockopt(IPV6_ONLY)");
					printf("IPV6 ONLY ! Tentative de passer en IPV4...\n");
					
					if (sockV4 >=0)
					{
						close(sockV6);
						sockV6 = -1;
					}
				}
				else
				{	//Traduction possible, on ferme l'ipv4
					if (sockV4 >= 0)
					{
						printf("IPV4 over IPV6 => close IPV4\n");
						close(sockV4);
						sockV4=-1;
					}
				}
			}
			
			if ( sockV4 >= 0 )
			{
				choix_ai = aiv4;
				listener = sockV4;
			}
			else if ( sockV6 < 0 )
			{
				Error_Quit ("Aucun protocole IPV4 ou IPV6 possible.");
			}
			
			
				
			
			/* "Address already in use" error message killer !!!! */
			int tr=1;
			if (setsockopt(listener,SOL_SOCKET,SO_REUSEADDR,&tr,sizeof(int)) == -1)
			{
				perror("setsockopt");
				exit(1);
			}
			
			/*  Assign socket address to socket  */
			if ( bind (listener, choix_ai->ai_addr, choix_ai->ai_addrlen) < 0 )
			{
				perror("Bind");
				Error_Quit ("Couldn't bind listening socket.");
			}
			
			/*  Make socket a listening socket  */
			if ( listen (listener, LISTENQ) < 0 )
				Error_Quit ("Call to listen failed.");
			
			int flags = fcntl(listener, F_GETFL );
			fcntl(listener, F_SETFL, flags | O_NONBLOCK );
			
			freeaddrinfo(list_addr);
			
			/* Diverses initialisations */
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
			//Gestion du timeout pour la bascule en mode commande
			gettimeofday(&time_conn, NULL);
			
			//XHCP_running_status = XHCPstate_waitCommand;
			XHCP_running_status = XHCPstate_waitHTTPRequest;
		
			/* No break, we continue !!!*/
			
		/* ------------------------------------------------------------------------ */
		case (XHCPstate_waitHTTPRequest):
		
			if ( (nbCar = recv(conn, buffer, MAX_REQ_LINE - 1, MSG_DONTWAIT)) <0 )
			{
				if ( (errno == EWOULDBLOCK) || (errno == EAGAIN) )
				{
					//TODO Gestion du timeout
					gettimeofday(&time_resp, NULL);
					if ( timerdiff(&time_conn, &time_resp) > 100000 )
					{
						// On passe en mode "Commande"
						XHCP_running_status = XHCPstate_waitCommand;
						XHCP_printXHCPResponse (conn, RES_HALWELCOM );  // Petit message de bienvenue
					}
					return XHCP_running_status;
				}
				else
					Error_Quit ("Error calling accept()");
			
			}
		
			buffer[nbCar]='\0';

			/* We suppress all extra characters */
			Trim (buffer, 0);

			
			processHttpRequest(conn, buffer);


			
			if ( close (conn) < 0 )
				Error_Quit ("Error closing connection socket in parent.");

			
			XHCP_running_status = XHCPstate_waitConnect;
			return  XHCP_running_status;

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

int XHCPcmd_QUIT (int sockd, int argc, char **argv)
{
    return XHCP_EXE_DISCON;
}

int XHCPcmd_SHUTDOWN (int sockd, int argc, char **argv)
{
    stop = 1;
    
    XHCP_printMessage (sockd, 221, "Shuting down in progress ... Good night... !!" );
    
    return XHCP_EXE_DISCON;
}

int XHCPcmd_LISTGLOBALS (int sockd, int argc, char **argv)
{
    node_t **globalsLst;
    char buffer[XHCP_BUFFER_SZ];
    int sz_buffer;
    int i, nb;
    
    XHCP_printXHCPResponse (sockd, RES_LSTGLVFOL ); // List of global variables follows
    
    if ( (globalsLst = roxml_xpath (rootConfig, "//global", &nb )) !=NULL )
    {
        for (i=0; i<nb; i++)
        {
            char *champ1 = roxml_get_content ( roxml_get_attr (globalsLst[i], "name", 0), buffer, XHCP_BUFFER_SZ, &sz_buffer );
            char *champ2 = roxml_get_content ( roxml_get_attr (globalsLst[i], "value", 0), champ1 + sz_buffer + 1, XHCP_BUFFER_SZ, &sz_buffer );
            
            XHCP_print (sockd, "%s=%s", champ1, champ2);
        }
    }
    
    XHCP_print (sockd, ".");
    return XHCP_EXE_SUCCESS;
}

int XHCPcmd_SETGLOBAL (int sockd, int argc, char **argv)
{
	if ( argc < 3 )
	{
		XHCP_printXHCPResponse (sockd, RES_SYNTAXERR );  // Syntax error
		return XHCP_EXE_ERROR;
	}

	if (  setGlobal(argv[1], argv[2]) == -1 )
            XHCP_printXHCPResponse (sockd, RES_INTNERROR); // Internal error
        else
            XHCP_printXHCPResponse (sockd, RES_GLOVALUPD); // "Global value updated"
        
	return XHCP_EXE_SUCCESS;
}

int XHCPcmd_GETGLOBAL (int sockd, int argc, char **argv)
{
    if ( argc < 2 )
    {
        XHCP_printXHCPResponse (sockd, RES_SYNTAXERR );  // Syntax error
        return XHCP_EXE_ERROR;
    }
    
    char *value = getGlobal (argv[1]);
    
    if ( value != NULL )
    {
        XHCP_printXHCPResponse (sockd, RES_GLOVALFOL ); // Global value follows
        
        XHCP_print (sockd, value);
        XHCP_print (sockd, ".");
    }
    else
    {
        XHCP_printXHCPResponse (sockd, RES_NOSUCHGLO ); // No such global
    }
    
    return XHCP_EXE_SUCCESS;
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
            return XHCP_EXE_ERROR;
        }
    }
    
    XHCP_printMessage (sockd, s?241:236, "--000U0" );
    
    if (s)
        XHCP_print (sockd, "." );
    
    
    return XHCP_EXE_SUCCESS;
}

int XHCPcmd_PUTCONFIGXML_handle (int sockd, int argc, char **argv, char *data)
{
    node_t *tmp;
    printf ("Entree XHCPcmd_PUTCONFIGXML_handle\n");
    if ( (tmp = roxml_load_buf (data)) == NULL )
    {
        XHCP_printXHCPResponse (sockd, RES_SYNTAXERR); // Syntax Error
        return XHCP_EXE_ERROR;
    }
    
    printf ("Chargement XML OK\n");
    roxml_close (rootConfig);
    printf ("Close OK\n");
    rootConfig = tmp;
    
    
    XHCP_loadConfig (rootConfig);
    printf ("loadConfig OK\n");
    
    XHCP_printXHCPResponse (sockd, RES_CFGDOCUPL ); // Configuration document uploaded
    
    return XHCP_EXE_SUCCESS;
}

int XHCPcmd_PUTCONFIGXML (int sockd, int argc, char **argv)
{
    XHCP_printXHCPResponse (sockd, RES_ENTCFGDOC ); // Enter configuration document, end with <CrLf>.<CrLf>
    
    XHCP_setAdditionalDataHandler (XHCPcmd_PUTCONFIGXML_handle);
    
    return XHCP_EXE_ARGS;
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
    return XHCP_EXE_SUCCESS;
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
        return XHCP_EXE_ERROR;
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
    
    
    
    return XHCP_EXE_SUCCESS;
}

int XHCPcmd_DELRULE (int sockd, int argc, char **argv)
{
    node_t **rulesNodesLst;
    char buffer[XHCP_BUFFER_SZ];
    int sz_buffer;
    int nb;
    
    
    
    if (argc != 2)
    {
        XHCP_printXHCPResponse (sockd, RES_SYNTAXERR ); // Syntax Error
        return XHCP_EXE_ERROR;
    }
    
    sprintf (buffer, "//determinator[@guid='%s']", argv[1]);
    
    printf ("%s\n", buffer);
    
    if ( (rulesNodesLst = roxml_xpath (rootConfig, buffer, &nb )) == NULL )
        XHCP_printXHCPResponse (sockd, RES_NOSUCHSCR ); // No such script/rule
    else
    {
        
        roxml_del_node (rulesNodesLst[0]);

		saveHal4lConfig (HAL4L_getConfigFile ());
		loadHal4lConfig (HAL4L_getConfigFile ());
	
        XHCP_printXHCPResponse (sockd, RES_SCRSUCDEL ); // Script/rule successfully deleted
    }
    
    
    
    return XHCP_EXE_SUCCESS;
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
        return XHCP_EXE_ERROR;
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
        return XHCP_EXE_ERROR;
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
        return XHCP_EXE_ERROR;
    }

	/* On ratache le nouveau determinator à la liste */
	roxml_parent_node(lstNodes[0], nTmp);
		
	
	saveHal4lConfig (HAL4L_getConfigFile ());
	//roxml_close (nTmp);
	
	
    loadHal4lConfig (HAL4L_getConfigFile ());


    free(newId);


    XHCP_printXHCPResponse (sockd, RES_CFGDOCUPL ); // Configuration document uploaded
    
    return XHCP_EXE_SUCCESS;
}

int XHCPcmd_SETRULE (int sockd, int argc, char **argv)
{
    XHCP_printXHCPResponse (sockd, RES_SENDDRULE ); // Send rule, end with <CrLf>.<CrLf>
    
    XHCP_setAdditionalDataHandler (XHCPcmd_SETRULE_handle);
    
    return XHCP_EXE_ARGS;
}

int XHCPcmd_GETCONFIGXML (int sockd, int argc, char **argv)
{
    char *writeBuffer = NULL;
    
	XHCP_printXHCPResponse (sockd, RES_CFGDOCFOL ); // Configuration document follows
	
	roxml_commit_changes (rootConfig, NULL, &writeBuffer, 1);
	
	XHCP_print (sockd, writeBuffer);
	XHCP_print (sockd, ".");
	
	//free (writeBuffer);
	roxml_release(RELEASE_LAST);

    return XHCP_EXE_SUCCESS;
}


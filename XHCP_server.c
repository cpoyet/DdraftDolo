

/*  Global macros/variables  */

#define MAX_REQ_LINE         (1024)

#define XHCP_HAL_vendor "XPL-XPLHAL"
#define XHCP_HAL_device "SERVER1"
#define XHCP_HAL_version "1.0.3.1"
#define XHCP_version "1.5"


/*  Prints an error message and quits  */

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



int getXHCPRequest(int conn, struct ReqInfo * reqinfo)
{
    
    char   buffer[MAX_REQ_LINE] = {0};
    int    rval;
    fd_set fds;
    struct timeval tv;
    
    
    /*  Set timeout to 5 seconds  */
    tv.tv_sec  = 5;
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
		}
        else if ( rval == 0 )
        {
            /*  input not ready after timeout  */
			sprintf(response_msg,"200 %s.%s Version %s XHCP %s ready",XHCP_HAL_vendor, XHCP_HAL_device, XHCP_HAL_version, XHCP_version);
			Writeline(conn, response_msg, strlen(response_msg));
            return 0;
        }
        else
        {
            /*  We have an input line waiting, so retrieve it  */
            Readline(conn, buffer, MAX_REQ_LINE - 1);
            Trim(buffer);
            
            if ( buffer[0] == '\0' )
				break;
            
            if ( Parse_Line(buffer, XHCP_reqinfo) )
				break;
        }
    } while ( XHCP_reqinfo->id != CMD_QUIT );
    
    return 0;
}


int XHCP_server(int argc, char *argv[])
{
    int    listener, conn;
    pid_t  pid;
    struct sockaddr_in servaddr;
	int retStatus;
    
	char response_msg[256];
    
    /*  Create socket  */
    if ( (listener = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
		Error_Quit("Couldn't create listening socket.");
    
    
    /*  Populate socket address structure  */
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(SERVER_PORT);
    
    
    /*  Assign socket address to socket  */
    if ( bind(listener, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0 )
		Error_Quit("Couldn't bind listening socket.");
    
    
    /*  Make socket a listening socket  */
    if ( listen(listener, LISTENQ) < 0 )
		Error_Quit("Call to listen failed.");
    
    
    /*  Loop infinitely to accept and service connections  */
    
    while ( 1 )
    {
        
        /*  Wait for connection  */
        
        if ( (conn = accept(listener, NULL, NULL)) < 0 )
			Error_Quit("Error calling accept()");
        
        sprintf(response_msg,"200 %s.%s Version %s XHCP %s ready",XHCP_HAL_vendor, XHCP_HAL_device, XHCP_HAL_version, XHCP_version);
		Writeline(conn, response_msg, strlen(response_msg));
	
		while  ( (retStatus = XHCP_Request(conn)) > 0 );
		
		if ( retStatus < 0 )
			Error_Quit("Connexion time out");
    
        /*  If we get here, we are still in the parent process,
        so close the connected socket, clean up child processes,
        and go back to accept a new connection.                   */
        
        if ( close(conn) < 0 )
			Error_Quit("Error closing connection socket in parent.");
        
        waitpid(-1, NULL, WNOHANG);
    }
    
    return EXIT_FAILURE;    /*  We shouldn't get here  */
}


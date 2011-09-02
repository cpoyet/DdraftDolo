




#ifndef _XHCP_SERVER_H_
#define _XHCP_SERVER_H_

#ifndef _XHCP_SERVER_C_
#define EXT_XHCP_SERVER extern
#else
#define EXT_XHCP_SERVER 
#endif


#include <stdio.h>


#define MAX_CMD_ARGS		(16)
#define MAX_REQ_LINE		(1024)



EXT_XHCP_SERVER enum _XHCP_command_id {	CMD_ADDEVENT        , 
				CMD_ADDSINGLEEVENT  , 
				CMD_CAPABILITIES    , 
				CMD_CLEARERRLOG     , 
				CMD_DELDEVCONFIG    , 
				CMD_DELEVENT        , 
				CMD_DELGLOBAL       , 
				CMD_DELRULE         , 
				CMD_DELSCRIPT       , 
				CMD_GETCONFIGXML    , 
				CMD_GETDEVCONFIG    , 
				CMD_GETDEVCONFIGVALUE,
				CMD_GETERRLOG       , 
				CMD_GETEVENT        , 
				CMD_GETGLOBAL       , 
				CMD_GETRULE         , 
				CMD_GETSCRIPT       , 
				CMD_GETSETTING      , 
				CMD_LISTALLDEVS     , 
				CMD_LISTDEVICES     , 
				CMD_LISTEVENTS      , 
				CMD_LISTGLOBALS     , 
				CMD_LISTOPTIONS     , 
				CMD_LISTRULEGROUPS  , 
				CMD_LISTRULES       , 
				CMD_LISTSCRIPTS     , 
				CMD_LISTSETTINGS    , 
				CMD_LISTSINGLEEVENTS, 
				CMD_LISTSUBS        , 
				CMD_MODE            , 
				CMD_PUTCONFIGXML    , 
				CMD_PUTDEVCONFIG    , 
				CMD_PUTSCRIPT       , 
				CMD_RELOAD          , 
				CMD_REPLINFO        , 
				CMD_RUNRULE         , 
				CMD_RUNSUB          , 
				CMD_SENDXAPMSG      , 
				CMD_SENDXPLMSG      , 
				CMD_SETGLOBAL       , 
				CMD_SETRULE         , 
				CMD_SETSETTING      , 
				CMD_QUIT            ,
				END_CMD					 };
typedef enum _XHCP_command_id XHCP_command_id;
EXT_XHCP_SERVER enum _XHCP_response_id {	RES_HALWELCOM,
				RES_RELOADSUC,
				RES_SCRIPTEXE,
				RES_LSTSETFOL,
				RES_LSTOPTFOL,
				RES_SETUPDATE,
				RES_ERRLOGFOL,
				RES_REQSETFOL,
				RES_CFGDOCFOL,
				RES_REQSCRFOL,
				RES_SCRSAVSUC,
				RES_LSTSCRFOL,
				RES_XPLMSGTRA,
				RES_SCRSUCDEL,
				RES_CFGDOCUPL,
				RES_LSTXPLDFO,
				RES_LSTGFGITF,
				RES_LSTEVTFOL,
				RES_EVTADDSUC,
				RES_CFGITESUC,
				RES_CONTIMOUT,
				RES_CLOCONBYE,
				RES_EVTINFFOL,
				RES_EVTDEVSUC,
				RES_LSTSUBFOL,
				RES_ERRLOGCLR,
				RES_SUBROUFOL,
				RES_REPMODACT,
				RES_LSTGLVFOL,
				RES_GLOVALUPD,
				RES_GLOVALDEL,
				RES_CFGITEFOL,
				RES_DEVCFGDEL,
				RES_CAPABISTR,
				RES_LSTDTRFOL,
				RES_RULADDSUC,
				RES_STAINFFOL,
				RES_LSTDTGFOL,
				RES_CAPSUBFOL,
				RES_SCRSVGFOL,
				RES_GLOVALFOL,
				RES_ENTERSCRT,
				RES_SNDMESSTR,
				RES_ENTCFGDOC,
				RES_ENTEVTDAT,
				RES_SNDCFGITM,
				RES_SENDDRULE,
				RES_RELODFAIL,
				RES_SCRNOTEXE,
				RES_NOSUCHSET,
				RES_NOSUCHSCR,
				RES_NOCONFAVI,
				RES_NOSUCHDEV,
				RES_NOVENINFO,
				RES_NOSUCHEVT,
				RES_NOSUCHSUB,
				RES_NOSUCHGLO,
				RES_COMNOTREC,
				RES_SYNTAXERR,
				RES_PERDENIED,
				RES_INTNERROR,
				RES_REPALRACT,
				RES_REPDATFOL,
				END_RES			};
typedef enum _XHCP_response_id XHCP_response_id;

typedef struct
{
	XHCP_command_id id;
	char *str;
	int ( *fnct ) ( int, char * [] );
} XHCP_command;

typedef struct
{
	XHCP_response_id id;
	int num;
	char *str;
} XHCP_response;

EXT_XHCP_SERVER XHCPcmd_QUIT ( int, char **);
EXT_XHCP_SERVER XHCPcmd_CAPABILITIES ( int, char **);

#ifndef _XHCP_SERVER_C_

EXT_XHCP_SERVER XHCP_command *XHCP_commandList;
EXT_XHCP_SERVER XHCP_response *XHCP_responseList;

#else

EXT_XHCP_SERVER XHCP_command XHCP_commandList[] = {
				{ CMD_ADDEVENT        , "ADDEVENT"         , NULL }, 
				{ CMD_ADDSINGLEEVENT  , "ADDSINGLEEVENT"   , NULL },  
				{ CMD_CAPABILITIES    , "CAPABILITIES"     , XHCPcmd_CAPABILITIES },
				{ CMD_CLEARERRLOG     , "CLEARERRLOG"      , NULL },    
				{ CMD_DELDEVCONFIG    , "DELDEVCONFIG"     , NULL },
				{ CMD_DELEVENT        , "DELEVENT"         , NULL }, 
				{ CMD_DELGLOBAL       , "DELGLOBAL"        , NULL },  
				{ CMD_DELRULE         , "DELRULE"          , NULL },
				{ CMD_DELSCRIPT       , "DELSCRIPT"        , NULL },  
				{ CMD_GETCONFIGXML    , "GETCONFIGXML"     , NULL },
				{ CMD_GETDEVCONFIG    , "GETDEVCONFIG"     , NULL },
				{ CMD_GETDEVCONFIGVALUE,"GETDEVCONFIGVALUE", NULL },
				{ CMD_GETERRLOG       , "GETERRLOG"        , NULL },  
				{ CMD_GETEVENT        , "GETEVENT"         , NULL }, 
				{ CMD_GETGLOBAL       , "GETGLOBAL"        , NULL },  
				{ CMD_GETRULE         , "GETRULE"          , NULL },
				{ CMD_GETSCRIPT       , "GETSCRIPT"        , NULL },  
				{ CMD_GETSETTING      , "GETSETTING"       , NULL },   
				{ CMD_LISTALLDEVS     , "LISTALLDEVS"      , NULL },    
				{ CMD_LISTDEVICES     , "LISTDEVICES"      , NULL },    
				{ CMD_LISTEVENTS      , "LISTEVENTS"       , NULL },   
				{ CMD_LISTGLOBALS     , "LISTGLOBALS"      , NULL },    
				{ CMD_LISTOPTIONS     , "LISTOPTIONS"      , NULL },    
				{ CMD_LISTRULEGROUPS  , "LISTRULEGROUPS"   , NULL },  
				{ CMD_LISTRULES       , "LISTRULES"        , NULL },  
				{ CMD_LISTSCRIPTS     , "LISTSCRIPTS"      , NULL },    
				{ CMD_LISTSETTINGS    , "LISTSETTINGS"     , NULL },
				{ CMD_LISTSINGLEEVENTS, "LISTSINGLEEVENTS" , NULL },    
				{ CMD_LISTSUBS        , "LISTSUBS"         , NULL }, 
				{ CMD_MODE            , "MODE"             , NULL },  
				{ CMD_PUTCONFIGXML    , "PUTCONFIGXML"     , NULL },
				{ CMD_PUTDEVCONFIG    , "PUTDEVCONFIG"     , NULL },
				{ CMD_PUTSCRIPT       , "PUTSCRIPT"        , NULL },  
				{ CMD_RELOAD          , "RELOAD"           , NULL },    
				{ CMD_REPLINFO        , "REPLINFO"         , NULL }, 
				{ CMD_RUNRULE         , "RUNRULE"          , NULL },
				{ CMD_RUNSUB          , "RUNSUB"           , NULL },    
				{ CMD_SENDXAPMSG      , "SENDXAPMSG"       , NULL },   
				{ CMD_SENDXPLMSG      , "SENDXPLMSG"       , NULL },   
				{ CMD_SETGLOBAL       , "SETGLOBAL"        , NULL },  
				{ CMD_SETRULE         , "SETRULE"          , NULL },
				{ CMD_SETSETTING      , "SETSETTING"       , NULL },   
				{ CMD_QUIT            , "QUIT"             , XHCPcmd_QUIT },   
				{ END_CMD             , NULL               , NULL }  };
				
EXT_XHCP_SERVER XHCP_response XHCP_responseList[] = {
//                                     ----+----1----+----2----+----3----+----4----+----5
				{ RES_HALWELCOM, 200, "XPLHal Welcome Banner" },
				{ RES_RELOADSUC, 201, "Reload successful" },
				{ RES_SCRIPTEXE, 203, "Script executed" },
				{ RES_LSTSETFOL, 204, "List of settings follows" },
				{ RES_LSTOPTFOL, 205, "List of options follows" },
				{ RES_SETUPDATE, 206, "Setting updated" },
				{ RES_ERRLOGFOL, 207, "Error log follows" },
				{ RES_REQSETFOL, 208, "Requested setting follows" },
				{ RES_CFGDOCFOL, 209, "Configuration document follows" },
				{ RES_REQSCRFOL, 210, "Requested script follows" },
				{ RES_SCRSAVSUC, 211, "Script saved successfully" },
				{ RES_LSTSCRFOL, 212, "List of scripts follows" },
				{ RES_XPLMSGTRA, 213, "XPL message transmitted" },
				{ RES_SCRSUCDEL, 214, "Script successfully deleted" },
				{ RES_CFGDOCUPL, 215, "Configuration document uploaded" },
				{ RES_LSTXPLDFO, 216, "List of XPL devices follows" },
				{ RES_LSTGFGITF, 217, "List of config items follows" },
				{ RES_LSTEVTFOL, 218, "List of events follows" },
				{ RES_EVTADDSUC, 219, "Event added successfully" },
				{ RES_CFGITESUC, 220, "Configuration items received successfully" },
				{ RES_CONTIMOUT, 221, "Connexion time-out" },
				{ RES_CLOCONBYE, 221, "Closing connection - good bye" },
				{ RES_EVTINFFOL, 222, "Event information follows" },
				{ RES_EVTDEVSUC, 223, "Event deleted successfully" },
				{ RES_LSTSUBFOL, 224, "List of subs follows" },
				{ RES_ERRLOGCLR, 225, "Error log cleared" },
				{ RES_SUBROUFOL, 229, "Sub-routine follows" },
				{ RES_REPMODACT, 230, "Replication mode active" },
				{ RES_LSTGLVFOL, 231, "List of global variables follows" },
				{ RES_GLOVALUPD, 232, "Global value updated" },
				{ RES_GLOVALDEL, 233, "Global variable deleted" },
				{ RES_CFGITEFOL, 234, "Configuration item value(s) follow" },
				{ RES_DEVCFGDEL, 235, "Device configuration deleted" },
				{ RES_CAPABISTR, 236, "Capabilities string" },
				{ RES_LSTDTRFOL, 237, "List of Determinator Rules follows" },
				{ RES_RULADDSUC, 238, "Rule added successfully" },
				{ RES_STAINFFOL, 239, "Status information follows" },
				{ RES_LSTDTGFOL, 240, "List of determinator groups follows" },
				{ RES_CAPSUBFOL, 241, "Capabilities/subsystem information follows" },
				{ RES_SCRSVGFOL, 242, "Script saved - additional information follows" },
				{ RES_GLOVALFOL, 291, "Global value follows" },
				{ RES_ENTERSCRT, 311, "Enter script, end with <CrLf>.<CrLf>" },
				{ RES_SNDMESSTR, 313, "Send message to be transmitted, end with <CrLf>.<CrLf>" },
				{ RES_ENTCFGDOC, 315, "Enter configuration document, end with <CrLf>.<CrLf>" },
				{ RES_ENTEVTDAT, 319, "Enter event data, end with <CrLf>.<CrLf>" },
				{ RES_SNDCFGITM, 320, "Send configuration items, end with <CrLf>.<CrLf>" },
				{ RES_SENDDRULE, 328, "Send rule, end with <CrLf>.<CrLf>" },
				{ RES_RELODFAIL, 401, "Reload failed" },
				{ RES_SCRNOTEXE, 403, "Script not executed" },
				{ RES_NOSUCHSET, 405, "No such setting" },
				{ RES_NOSUCHSCR, 410, "No such script" },
				{ RES_NOCONFAVI, 416, "No config available for specified device" },
				{ RES_NOSUCHDEV, 417, "No such device" },
				{ RES_NOVENINFO, 418, "No vendor information available for specified device." },
				{ RES_NOSUCHEVT, 422, "No such event" },
				{ RES_NOSUCHSUB, 429, "No such sub-routine" },
				{ RES_NOSUCHGLO, 491, "No such global" },
				{ RES_COMNOTREC, 500, "Command not recognised" },
				{ RES_SYNTAXERR, 501, "Syntax error" },
				{ RES_PERDENIED, 502, "Permission denied" },
				{ RES_INTNERROR, 503, "Internal error - command not performed" },
				{ RES_REPALRACT, 530, "A replication client is already active" },
				{ RES_REPDATFOL, 600, "Replication data follows" },
				{ END_RES,		   0, NULL } };
#endif
#endif
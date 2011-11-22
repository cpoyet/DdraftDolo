#ifndef _XPLHAL_COMMON_H_
#define _XPLHAL_COMMON_H_

#ifndef _XPLHAL_COMMON_C_
#define EXT_XPLHAL_COMMON extern
#else
#define EXT_XPLHAL_COMMON 
#endif


#include <roxml.h>

/* Constantes ********************************************************************/

/* Macros ************************************************************************/

/* Definitions de types **********************************************************/

/* Prototypes ********************************************************************/
EXT_XPLHAL_COMMON char *xmlGetAttribut (node_t* argXmlConfig, char *nodeXpath, char *attrName, int opt);
EXT_XPLHAL_COMMON int xmlGetBoolAttribut (node_t* argXmlConfig, char *nodeXpath, char *attrName, int opt);
EXT_XPLHAL_COMMON int xmlGetIntAttribut (node_t* argXmlConfig, char *nodeXpath, char *attrName, int opt);

EXT_XPLHAL_COMMON char *getGlobal(char *name);
EXT_XPLHAL_COMMON int setGlobal(char *name, char *value)

/* Variables globales ************************************************************/

#endif
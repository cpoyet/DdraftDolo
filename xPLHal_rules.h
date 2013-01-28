#ifndef _XPLHAL_RULES_H_
#define _XPLHAL_RULES_H_

#ifndef _XPLHAL_RULES_C_
#define EXT_XPLHAL_RULES extern
#else
#define EXT_XPLHAL_RULES 
int sortOrderAction(void const *a, void const *b);
int compareClockStrings(int v1, char *op, int v2);
int compareGlobalStrings(char *va, char *op, char *globalValue);

#endif


#include <roxml.h>

/* Constantes ********************************************************************/

/* Macros ************************************************************************/

/* Definitions de types **********************************************************/

/* Prototypes ********************************************************************/


//EXT_XPLHAL_RULES int rules_testTimeConditions (node_t *detNode, int anyRule, int tickDate, int tickYear, int tickMonth, int tickDay, int tickTime);
//EXT_XPLHAL_RULES int rules_executeDetActions (node_t *detNode);
//EXT_XPLHAL_RULES int rules_testAllConditions (node_t *detNode, int type_event);


EXT_XPLHAL_RULES int rules_verifAllConditions(node_t *detNode, int type_event);
EXT_XPLHAL_RULES int rules_verifTimeConditions ( node_t *detNode, int anyRule, time_t *time);
EXT_XPLHAL_RULES int rules_executeActions(node_t *detNode);
EXT_XPLHAL_RULES int rules_verifDayConditions ( node_t *detNode, int anyRule, time_t *time);
EXT_XPLHAL_RULES int rules_verifGlobalConditions ( node_t *detNode, int anyRule, char *globalName);


/* Variables globales ************************************************************/


#endif

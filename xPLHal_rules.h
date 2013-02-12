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
typedef enum _event_type { EV_XPLMESSAGE, EV_GLOBALCHANGED, EV_TIME } event_t;
typedef enum _rule_type { ALL_RULE, ANY_RULE } rule_t;

/* Prototypes ********************************************************************/


//EXT_XPLHAL_RULES int rules_testTimeConditions (node_t *detNode, int anyRule, int tickDate, int tickYear, int tickMonth, int tickDay, int tickTime);
//EXT_XPLHAL_RULES int rules_executeDetActions (node_t *detNode);
//EXT_XPLHAL_RULES int rules_testAllConditions (node_t *detNode, int type_event);


EXT_XPLHAL_RULES int rules_verifAllConditions(node_t *detNode);
EXT_XPLHAL_RULES int rules_verifTimeConditions ( node_t *detNode, rule_t anyRule, time_t *time);
EXT_XPLHAL_RULES int rules_executeActions(node_t *detNode);
EXT_XPLHAL_RULES int rules_verifDayConditions ( node_t *detNode, rule_t anyRule, int weekDay);
EXT_XPLHAL_RULES int rules_verifGlobalConditions ( node_t *detNode, rule_t anyRule);
EXT_XPLHAL_RULES int rules_verifXplConditions ( node_t *detNode, rule_t anyRule, xPL_MessagePtr theMessage);

/* Variables globales ************************************************************/


#endif

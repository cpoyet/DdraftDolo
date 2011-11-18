
#define _XPLHAL_RULES_C_

#define _XOPEN_SOURCE 

#include "xPLHal_rules.h"
#include "xPLHal_scheduler.h"
#include "xPLHal4L.h"


int compareClockStrings(int v1, char *op, int v2)
{
	int result;

	result = v2 - v1;

	if  ( op[0] == '!' && op[1] == '=' && result == 0 )
		return 0;
	if  ( ( op[0] == '=' || op[1] == '=' ) && result == 0 )
		return 1;
	if ( ( (op[0]=='&' && op[1]=='g') || (op[1]=='&' && op[2]=='g') || op[0] == '>' ) && result < 0 )
		return 1;
	if ( ( (op[0]=='&' && op[1]=='l') || (op[1]=='&' && op[2]=='l') || op[0] == '<' ) && result > 0 )
		return 1;

	return 0;

}

int rules_executeActions(node_t *detNode)
{

    node_t **tActLst;
    int nbActLst;
	
	int i;

	/* Liste des actions */
	tActLst = roxml_xpath ( detNode, "output/*", &nbActLst);
	HAL4L_Debug("rules_executeActions : %d actions trouvées",nbActLst);
	
	
	qsort (tActLst, nbActLst, sizeof(node_t *), sortOrderAction);

	
	for (i=0; i<nbActLst; i++)
	{
		//printf("- %s\n",);
		
		char *action = roxml_get_name(tActLst[i], NULL, 0);
		
		if ( strcasecmp(action,"logAction") == 0 )
		{
		}
		else if ( strcasecmp(action,"xplAction") == 0 )
		{
		}
		else if ( strcasecmp(action,"globalAction") == 0 )
		{
		}
		else if ( strcasecmp(action,"delayAction") == 0 )
		{
		}
		else if ( strcasecmp(action,"stopAction") == 0 )
		{
		}
		else if ( strcasecmp(action,"suspendAction") == 0 )
		{
		}
		else if ( strcasecmp(action,"executeAction") == 0 )
		{
		}
		else if ( strcasecmp(action,"execRuleAction") == 0 )
		{
		}
		else if ( strcasecmp(action,"runScriptAction") == 0 )
		{
		}
		else
		{
		}
		
	}


	return 0;
}

int sortOrderAction(void const *a, void const *b)
{
	char bufA[10],bufB[10];
	int sz_bufA, sz_bufB;

   node_t *nodeA = *(node_t **)a;
   node_t *nodeB = *(node_t **)b;
   
   node_t *attrA = roxml_get_attr(nodeA, "executeOrder", 0);
   node_t *attrB = roxml_get_attr(nodeB, "executeOrder", 0);
   
   roxml_get_content ( attrA, bufA, 9, &sz_bufA );
   roxml_get_content ( attrB, bufB, 9, &sz_bufB );
   
   int orderA = atoi(bufA);
   int orderB = atoi(bufB);
   
   return orderA - orderB;
}

//int verifTimeConditions ( node_t *detNode, int anyRule, int tickDate, int tickYear, int tickMonth, int tickDay, int tickTime)
int rules_verifTimeConditions ( node_t *detNode, int anyRule, time_t *time)
{
    node_t **tCondLst;
    int nbCondLst;
	char ct[10], op[32], va[80];
	int sz_ct, sz_op, sz_va;
	int i;
	int ret=0;

	struct tm *tb;

    tb = localtime(time);
	
	/* Liste des conditions */
	tCondLst = roxml_xpath ( detNode, "descendant-or-self::timeCondition", &nbCondLst);
	HAL4L_Debug("rules_verifTimeConditions : %d timeConditions trouvées",nbCondLst);

	for ( i=0; i<nbCondLst; i++)
	{				
		/* On recupere les elements de la regle */
		roxml_get_content ( roxml_get_attr (tCondLst[i], "category", 0), ct, 10, &sz_ct );
		//roxml_release (RELEASE_LAST);
		roxml_get_content ( roxml_get_attr (tCondLst[i], "operator", 0), op, 32, &sz_op );
		//roxml_release (RELEASE_LAST);
		roxml_get_content ( roxml_get_attr (tCondLst[i], "value", 0), va, 80, &sz_va );
		//roxml_release (RELEASE_LAST);
		/* Comparaison des elements en fonction du type */
		/*if (strcasecmp(ct,"time") == 0 )
			ret = compareClockStrings(tickTime, op, timeStr2int(va));
		else if (strcasecmp(ct,"date") == 0 )
			ret = compareClockStrings(tickDate, op, dateStr2int(va));
		else if (strcasecmp(ct,"day") == 0 )
			ret = compareClockStrings(tickDay, op, atoi(va));
		else if (strcasecmp(ct,"month") == 0 )
			ret = compareClockStrings(tickMonth, op, monthStr2int(va));
		else if (strcasecmp(ct,"year") == 0 )
			ret = compareClockStrings(tickYear, op, atoi(va));*/
		if (strcasecmp(ct,"time") == 0 )
			ret = compareClockStrings(tb->tm_hour*60+tb->tm_min, op, timeStr2int(va));
		else if (strcasecmp(ct,"date") == 0 )
			ret = compareClockStrings((tb->tm_year+1900)*10000+(tb->tm_mon+1)*100+tb->tm_mday, op, dateStr2int(va));
		else if (strcasecmp(ct,"day") == 0 )
			ret = compareClockStrings(tb->tm_mday, op, atoi(va));
		else if (strcasecmp(ct,"month") == 0 )
			ret = compareClockStrings(tb->tm_mon+1, op, monthStr2int(va));
		else if (strcasecmp(ct,"year") == 0 )
			ret = compareClockStrings(tb->tm_year+1900, op, atoi(va));

			
		HAL4L_Debug("rules_verifTimeConditions : Via time_t : Date=%d Heure=%d",(tb->tm_year+1900)*10000+(tb->tm_mon+1)*100+tb->tm_mday, tb->tm_hour*60+tb->tm_min);

		char dn[80]; int sz_dn;
		roxml_get_content ( roxml_get_attr (tCondLst[i], "display_name", 0), dn, 80, &sz_dn );
		roxml_release (RELEASE_LAST);
		HAL4L_Debug("rules_verifTimeConditions : %s -> %s", dn, ret?"OK":"NOK");
		
		/* Sorite de la boucle dès qu'une condition est fausse */
		if ( ret && anyRule )
			return ret;
		if ( !ret && !anyRule )
			return ret;
	}

	return ret;
}


int rules_verifAllConditions(node_t *detNode, int type_event)
{
	return 0;
}



#define _XPLHAL_RULES_C_

#define _XOPEN_SOURCE 

#include "xPLHal4L.h"
#include "xPLHal_rules.h"
#include "xPLHal_scheduler.h"
#include "xPLHal_common.h"


int isNum(char *c)
{
	int i;
	int sz = strlen(c);
	
	for (i=0; i<sz; i++ )
		if ( (c[i]<0 || c[i]>9) && c[i]!='.' && c[i] != '-')
			return 0;
	
	return 1;
}


int compareGlobalStrings(char *va, char *op, char *globalValue)
{
	char *endVa, *endGlobalValue;
	long valVa, valGlobalValue;
	

	if ( isNum(va) && isNum(globalValue) )
	{
		valVa = strtol(va, &endVa, 10);
		valGlobalValue = strtol(globalValue, &endGlobalValue, 10);
		return compareClockStrings(valVa, op, valGlobalValue);
	}
	else
		if ( op[0] == '=' && strcasecmp(va,globalValue) == 0 )
			return 1;
			
	return 0;
}

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
	
	char *p;
	char c1[256]; int sz_c1;
	
	int i, j;

	/* Liste des actions */
	tActLst = roxml_xpath ( detNode, "output/*", &nbActLst);
	HAL4L_Debug(HAL4L_DEBUG,"rules_executeActions : %d actions trouv�es",nbActLst);
	
	
	qsort (tActLst, nbActLst, sizeof(node_t *), sortOrderAction);

	
	for (i=0; i<nbActLst; i++)
	{
		//printf("- %s\n",);
		
		char *action = roxml_get_name(tActLst[i], NULL, 0);
		
		if ( strcasecmp(action,"logAction") == 0 )
		{
			roxml_get_content ( roxml_get_attr (tActLst[i], "logText", 0), c1, 255, &sz_c1 );
			HAL4L_Debug(0, c1);
		}
		else if ( strcasecmp(action,"xplAction") == 0 )
		{
 			xPL_MessagePtr action_mess;

			/* Allocate the message */
			action_mess = xPL_AllocMessage();

			/* Set the version (NOT DYNAMIC) */
			roxml_get_content ( roxml_get_attr (tActLst[i], "msg_type", 0), c1, 255, &sz_c1 );
			if ( strcasecmp("stat",c1) == 0 )
				action_mess->messageType = xPL_MESSAGE_STATUS;
			else if ( strcasecmp("trig",c1) == 0 )
				action_mess->messageType = xPL_MESSAGE_TRIGGER;
			else if ( strcasecmp("cmnd",c1) == 0 )
				action_mess->messageType = xPL_MESSAGE_COMMAND;
			else
			{
				HAL4L_Debug(HAL4L_ERROR, "ERROR - xplAction : msg_type \"%s\" unknown",c1);
				continue;
			}
			
			action_mess->receivedMessage = TRUE;

			/* Allocate a name/value list, if needed */
			if (action_mess->messageBody == NULL) action_mess->messageBody = xPL_AllocNVList();
			
			/* Install source into message */
			xPL_setSource(action_mess, XPLHAL4L_VENDOR, SCHEDULER_DEVICEID, HAL4L_hostName);

			roxml_get_content ( roxml_get_attr (tActLst[i], "msg_target", 0), c1, 255, &sz_c1 );
			xPL_setTarget(action_mess, XPLHAL4L_VENDOR, SCHEDULER_DEVICEID, c1);

			roxml_get_content ( roxml_get_attr (tActLst[i], "msg_schema", 0), c1, 255, &sz_c1 );
			p = strchr(c1, '.');
			*p='\0';
			p++;
			xPL_setSchema(action_mess, c1, p);
		
			node_t **tParamLst;
			int nbParamLst;
			tParamLst = roxml_xpath ( tActLst[i], "/xplActionParam[@expression]", &nbParamLst);
	
			for (j=0; j<nbParamLst; j++)
			{
				roxml_get_content ( roxml_get_attr (tActLst[i], "expression", 0), c1, 255, &sz_c1 );
				p = strchr(c1, '=');
				*p='\0';
				p++;

				xPL_setMessageNamedValue(action_mess, c1, p);
			}
		
			xPL_sendMessage(action_mess);
			xPL_releaseMessage(action_mess);
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

int rules_verifXplConditions ( node_t *detNode, rule_t anyRule, xPL_MessagePtr theMessage)
{
    node_t **tCondLst;
    int nbCondLst;

	char msg_type[9], source_vendor[9], source_device[9], source_instance[17], target_vendor[9], target_device[9], target_instance[17], schema_class[9], schema_type[9];
	int sz_msg_type, sz_source_vendor, sz_source_device, sz_source_instance, sz_target_vendor, sz_target_device, sz_target_instance, sz_schema_class, sz_schema_type;

	int i;
	int ret=0;

    char *globalValue;

	/* Liste des conditions */
	tCondLst = roxml_xpath ( detNode, "descendant-or-self::xplCondition", &nbCondLst);
	HAL4L_Debug(HAL4L_DEBUG,"rules_verifXplConditions : %d xplCondition trouvées",nbCondLst);

	for ( i=0; i<nbCondLst; i++)
	{
		/* On recupere les elements de la regle */
        roxml_get_content ( roxml_get_attr (tCondLst[i], "msg_type", 0),        msg_type,        8, &sz_msg_type );
        roxml_get_content ( roxml_get_attr (tCondLst[i], "source_vendor", 0),   source_vendor,   8, &sz_source_vendor );
        roxml_get_content ( roxml_get_attr (tCondLst[i], "source_device", 0),   source_device,   8, &sz_source_device );
        roxml_get_content ( roxml_get_attr (tCondLst[i], "source_instance", 0), source_instance,16, &sz_source_instance );
        roxml_get_content ( roxml_get_attr (tCondLst[i], "target_vendor", 0),   target_vendor,   8, &sz_target_vendor );
        roxml_get_content ( roxml_get_attr (tCondLst[i], "target_device", 0),   target_device,   8, &sz_target_device );
        roxml_get_content ( roxml_get_attr (tCondLst[i], "target_instance", 0), target_instance,16, &sz_target_instance );
        roxml_get_content ( roxml_get_attr (tCondLst[i], "schema_class", 0),    schema_class,    8, &sz_schema_class );
        roxml_get_content ( roxml_get_attr (tCondLst[i], "schema_type", 0),     schema_type,     8, &sz_schema_type );

        
        
        
		/* Sorite de la boucle dès qu'une condition est fausse */
		if ( ret && anyRule==ANY_RULE )
			return ret;
		if ( !ret && anyRule==ALL_RULE )
			return ret;
	}

	return ret;
}

int rules_verifTimeConditions ( node_t *detNode, rule_t anyRule, time_t *time)
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
	HAL4L_Debug(HAL4L_DEBUG,"rules_verifTimeConditions : %d timeConditions trouv�es",nbCondLst);

	for ( i=0; i<nbCondLst; i++)
	{				
		/* On recupere les elements de la regle */
		roxml_get_content ( roxml_get_attr (tCondLst[i], "category", 0), ct, 10, &sz_ct );
		roxml_get_content ( roxml_get_attr (tCondLst[i], "operator", 0), op, 32, &sz_op );
		roxml_get_content ( roxml_get_attr (tCondLst[i], "value", 0), va, 80, &sz_va );
		/* Comparaison des elements en fonction du type */
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

			
	/*	HAL4L_Debug(HAL4L_DEBUG,"rules_verifTimeConditions : Via time_t : Date=%d Heure=%d",(tb->tm_year+1900)*10000+(tb->tm_mon+1)*100+tb->tm_mday, tb->tm_hour*60+tb->tm_min);

		char dn[80]; int sz_dn;
		roxml_get_content ( roxml_get_attr (tCondLst[i], "display_name", 0), dn, 80, &sz_dn );
		roxml_release (RELEASE_LAST);
		HAL4L_Debug(HAL4L_DEBUG,"rules_verifTimeConditions : %s -> %s", dn, ret?"OK":"NOK");*/
		
		/* Sorite de la boucle d�s qu'une condition est fausse */
		if ( ret && anyRule==ANY_RULE )
			return ret;
		if ( !ret && anyRule==ALL_RULE )
			return ret;
	}

	return ret;
}

int rules_verifDayConditions ( node_t *detNode, rule_t anyRule, int weekDay)
{
    node_t **tCondLst;
    int nbCondLst;
	char dow[8];
	int sz_dow;
	int i;
	int ret=0;

	/* Liste des conditions */
	tCondLst = roxml_xpath ( detNode, "descendant-or-self::dayCondition", &nbCondLst);
	HAL4L_Debug(HAL4L_DEBUG,"rules_verifdAYConditions : %d dayCondition trouvées",nbCondLst);

	for ( i=0; i<nbCondLst; i++)
	{				
		/* On recupere les elements de la regle */
		roxml_get_content ( roxml_get_attr (tCondLst[i], "dow", 0), dow, 8, &sz_dow );
        
        if ( sz_dow == 7 )
            ret = dow[weekDay]=='1'?1:0;
        else
            ret = 0 ;
    
    
		/* Sorite de la boucle dès qu'une condition est fausse */
		if ( ret && anyRule==ANY_RULE )
			return ret;
		if ( !ret && anyRule==ALL_RULE )
			return ret;
	}

	return ret;
}

int rules_verifGlobalConditions ( node_t *detNode, rule_t anyRule)
{

    node_t **tCondLst;
    int nbCondLst;

	char nm[10], op[32], va[80];
	int sz_nm, sz_op, sz_va;

	int i;
	int ret=0;

    char *globalValue;

	/* Liste des conditions */
	tCondLst = roxml_xpath ( detNode, "descendant-or-self::globalCondition", &nbCondLst);
	HAL4L_Debug(HAL4L_DEBUG,"rules_verifGlobalConditions : %d globalCondition trouvées",nbCondLst);

	for ( i=0; i<nbCondLst; i++)
	{
		/* On recupere les elements de la regle */
		roxml_get_content ( roxml_get_attr (tCondLst[i], "name", 0), nm, 10, &sz_nm );
		roxml_get_content ( roxml_get_attr (tCondLst[i], "operator", 0), op, 32, &sz_op );
		roxml_get_content ( roxml_get_attr (tCondLst[i], "value", 0), va, 80, &sz_va );

        globalValue = getGlobal (nm);
		ret = compareGlobalStrings(va, op, globalValue);

		/* Sorite de la boucle dès qu'une condition est fausse */
		if ( ret && anyRule==ANY_RULE )
			return ret;
		if ( !ret && anyRule==ALL_RULE )
			return ret;
	}

	return ret;
}
int compute_ev_xPLMessage (xPL_MessagePtr theMessage)
{
    node_t **tDetLst;
    int nbDetLst;
	int i, ret;

	/* Recherche de determinators possédant
			- input match="any"
			- une condition "xplCondition"
			- ne possédant pas de condition "globalChanged"
			=> Vérification que la condition est vérifiée avec le message en cours
			=> Execution des actions */
	tDetLst = roxml_xpath ( rootConfig, "//determinator/input[@match='any']/ancestor::determinator[not(./input/globalChanged) && (./input/timeCondition)]", &nbDetLst);
	for ( i=0; i<nbDetLst; i++)
	{
		ret = rules_verifXplConditions ( tDetLst[i], ANY_RULE, theMessage);
		if ( ret )
			rules_executeActions(tDetLst[i]);
	}

	/* Recherche de determinators possédant
			- input match="all"
			- une condition "xplCondition"
			- ne possédant pas de condition "globalChanged"
			=> Test de toutes les conditions
			=> Si OK, alors execution des actions */
	tDetLst = roxml_xpath ( rootConfig, "//determinator/input[@match='all']/ancestor::determinator[not(./input/globalChanged) && (./input/timeCondition)]", &nbDetLst);
	for ( i=0; i<nbDetLst; i++)
	{
		ret = rules_verifAllConditions ( tDetLst[i] );
		if ( ret )
			rules_executeActions(tDetLst[i]);
	}

	return 0;
}

int compute_ev_globalChanged (char *variableName)
{
    node_t **tDetLst;
    int nbDetLst;
	int i, ret;

	char xpathFormat[] = "//determinator/input[@match='%s']/ancestor::determinator[(./input/globalChanged[@name='%s'])]";
	char *xpathString;
	
	xpathString = (char *) malloc (sizeof(char)*(100 + strlen(variableName)));
	
	/* Recherche de determinators possédant
			- input match="any"
			- une condition "globalChanged" avec attribut @name="le nom de la variable modifiée"
			=> Execution des actions*/
	sprintf(xpathString, xpathFormat, "any", variableName);
	tDetLst = roxml_xpath ( rootConfig, xpathString, &nbDetLst);
	for ( i=0; i<nbDetLst; i++)
	{
		rules_executeActions(tDetLst[i]);
	}

	/* Recherche de determinators possédant
			- input match="all"
			- une condition "globalChanged" avec attribut @name="le nom de la variable modifiée"
			=> Test de toutes les conditions
			=> Si OK, alors execution des actions */
	sprintf(xpathString, xpathFormat, "all", variableName);
	tDetLst = roxml_xpath ( rootConfig, xpathString, &nbDetLst);
	for ( i=0; i<nbDetLst; i++)
	{
		ret = rules_verifAllConditions ( tDetLst[i] );
		if ( ret )
			rules_executeActions(tDetLst[i]);
	}

	free(xpathString);
	return 0;
}

int compute_ev_time (time_t *time)
{
    node_t **tDetLst;
    int nbDetLst;
	int i, ret;

	char xpathFormat[] = "//determinator/input[@match='%s']/ancestor::determinator[not(./input/globalChanged) && (./input/%s)]";
	char *xpathString;

    struct tm *ts;
	
	xpathString = (char *) malloc (sizeof(char)*120);

	/* Recherche de determinators possédant
			- input match="any"
			- une condition "timeCondition" 
			- ne possédant pas de condition "globalChanged"
			=> Vérification que la condition est vérifiée avec la date/heure en cours
			=> Execution des actions*/
	sprintf(xpathString, xpathFormat, "any", "timeCondition");
	tDetLst = roxml_xpath ( rootConfig, xpathString, &nbDetLst);
	for ( i=0; i<nbDetLst; i++)
	{
		ret = rules_verifTimeConditions ( tDetLst[i], ANY_RULE, time);
		if ( ret )
			rules_executeActions(tDetLst[i]);
	}

	/* Recherche de determinators possédant
			- input match="any"
			- une condition "dayCondition"
			- ne possédant pas de condition "globalChanged"
			=> Vérification que l'attribut @dow possedant le flag du jour en cours à 1
			=> Execution des actions*/
	sprintf(xpathString, xpathFormat, "any", "dayCondition");
	tDetLst = roxml_xpath ( rootConfig, xpathString, &nbDetLst);
	ts = localtime(time);
	
	for ( i=0; i<nbDetLst; i++)
	{
		ret = rules_verifDayConditions ( tDetLst[i], ANY_RULE, ts->tm_wday);
		if ( ret )
			rules_executeActions(tDetLst[i]);
	}

	/* - Recherche de determinators possédant
			- input match="all"
			- une condition "timeCondition"
			- ne possédant pas de condition "globalChanged"
			=> Test de toutes les conditions
			=> Si OK, alors execution des actions*/
	sprintf(xpathString, xpathFormat, "all", "timeCondition");
	tDetLst = roxml_xpath ( rootConfig, xpathString, &nbDetLst);
	for ( i=0; i<nbDetLst; i++)
	{
		ret = rules_verifAllConditions ( tDetLst[i] );
		if ( ret )
			rules_executeActions(tDetLst[i]);
	}

			
	free(xpathString);
	return 0;
}

int rules_verifAllConditions(node_t *detNode)
{
	return 0;
}




#define _XPLHAL_COMMON_C_

#include "xPLHal_common.h"
#include "xPLHal4L.h"


char *getGlobal(char *name)
{
    node_t **result;
    int nb_result;
    static char buffer[80];
    int sz_buffer;
	char *ret = NULL;

	char *xpath = (char *)malloc(strlen(name)+27);

	sprintf(xpath,"//globals/global[@name='%s']",name);
    result = roxml_xpath ( rootConfig, xpath, &nb_result);
    if ( nb_result == 1 )
	{
        ret = roxml_get_content ( roxml_get_attr (result[0], "value", 0), buffer, 80, &sz_buffer );
	}
	roxml_release(RELEASE_LAST);
	
	free(xpath);

	return ret;
}

int setGlobal(char *name, char *value)
{
    node_t **result;
    int nb_result;

	char *xpath = (char *)malloc(strlen(name)+27);

        sprintf (xpath, "//globals/global[@name='%s']", name);
        result = roxml_xpath ( rootConfig, xpath, &nb_result);
        if ( nb_result == 1 )
        {
            node_t *attr = roxml_get_attr (result[0], "value", 0);
            if (attr != NULL )
                roxml_del_node (attr);
            roxml_add_node (result[0], 0, ROXML_ATTR_NODE, "value", value);
			roxml_release(RELEASE_LAST);
        }
        else if ( nb_result == 0 )
        {
            result = roxml_xpath ( rootConfig, "//globals", &nb_result);
            node_t *tmp = roxml_add_node (result[0], 0, ROXML_ELM_NODE, "global", NULL);
            roxml_add_node (tmp, 0, ROXML_ATTR_NODE, "name", name);
            roxml_add_node (tmp, 0, ROXML_ATTR_NODE, "value", value);
        }
        else
            return -1;
        
        saveHal4lConfig (HAL4L_getConfigFile ());
        
        return 0;
}


char *getAttrWithDeRef (node_t* argXmlConfig, char *nodeXpath, char *attrName, int opt)
{

	int sStart, sEnd;
	
	char *attr=xmlGetAttribut (argXmlConfig, nodeXpath, attrName, opt);
	int sz_attr = strlen(attr);
	
	for (sStart=0; sStart<sz_attr && attr[sStart]!='{'; sStart++);
		
	if ( sStart<sz_attr )
	{
		for (sEnd=sStart+1; sEnd<sz_attr && attr[sStart]!='}'; sEnd++);
		if ( sEnd<sz_attr )
		{
			// OK, on a trouvé une valeur de substitution !! 
			
			
			
			HAL4L_Debug (HAL4L_INFO,"Loading default configuration file: config.xml");
			
		}
	}
	

	return attr;
}


char *xmlGetAttribut (node_t* argXmlConfig, char *nodeXpath, char *attrName, int opt)
{
    node_t **result;
    int nb_result;
    static char buffer[80];
    int sz_buffer;
	char *tmp;

	char *xpath = (char *)malloc(strlen(nodeXpath)+strlen(attrName)+4);
	
	sprintf(xpath,"%s[@%s]",nodeXpath,attrName);
    result = roxml_xpath ( argXmlConfig, xpath, &nb_result);
    if ( nb_result == 1 )
	{
        tmp = roxml_get_content ( roxml_get_attr (result[0], attrName, 0), buffer, 80, &sz_buffer );
	}
	else if ( nb_result == 0 && opt )
	{
		tmp="";
	}
	else	
	{
		fprintf(stderr,"ERROR Parsing %s (%d results)\n",xpath,nb_result);
        Error_Quit ("Error parsing timer config file");
	}
	free(xpath);
	//roxml_release (RELEASE_LAST);

	return tmp;
}

int xmlGetBoolAttribut (node_t* argXmlConfig, char *nodeXpath, char *attrName, int opt)
{
	int i=0;
	char *trueLst[] = {"TRUE","1","ON","VRAI",NULL};
	char *falseLst[] = {"FALSE","0","OFF","FAUX",NULL};
	
	char *attr=xmlGetAttribut (argXmlConfig, nodeXpath, attrName, opt);
	
	do
	{
		if ( strcasecmp (trueLst[i], attr) == 0 )
			return 1;
	} while ( trueLst[++i]!=NULL );
	
	return 0;
}

int xmlGetIntAttribut (node_t* argXmlConfig, char *nodeXpath, char *attrName, int opt)
{
	char *attr=xmlGetAttribut (argXmlConfig, nodeXpath, attrName, opt);
	
	return atoi(attr);
}

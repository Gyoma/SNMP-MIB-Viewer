#include "Parser.h"
#include <iostream>


Parser::Parser(Tree::Ptr tree) :
    _tree(tree)
{}

Parser::~Parser()
{}


bool Parser::isLabelChar(char ch)
{
    if ((isalnum(ch)) || (ch == '-'))
        return true;

    if (ch == '_' /*&& netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID,
                   NETSNMP_DS_LIB_MIB_PARSE_LABEL)*/)
    {
        return true;
    }

    return false;
}

void Parser::parseImports(std::ifstream& file, const std::string& moduleName)
{
    //	register int    type;
    //	char            token[MAXTOKEN];
    //	char            modbuf[256];
    //#define MAX_IMPORTS	256
    //	struct module_import import_list[MAX_IMPORTS];
    //	int             this_module;
    //	struct module* mp;
    //
    //	int             import_count = 0;   /* Total number of imported descriptors */
    //	int             i = 0, old_i;       /* index of first import from each module */

    Token token;
    ModuleImport import;
    std::vector<ModuleImport> imports;

    parseToken(file, token);
    //type = get_token(fp, token, MAXTOKEN);

    /*
     * Parse the IMPORTS clause
     */
    while (token.type != LT::eSEMI && token.type != LT::eENDOFFILE)
    {
        if (token.type == LT::eLABEL)
        {
            //if (import_count == MAX_IMPORTS)
            //{
            //	print_error("Too many imported symbols", token, type);
            //	do	
            //	{
            //		type = get_token(fp, token, MAXTOKEN);
            //	} while (type != SEMI && type != ENDOFFILE);
            //	return;
            //}
            import.labels.push_back(token.lexem);
            //import_list[import_count++].label = strdup(token);
        }
        else if (token.type == LT::eFROM)
        {
            parseToken(file, token);

            if (import.labels.empty())
            {
                parseToken(file, token);
                continue;
            }

            //type = get_token(fp, token, MAXTOKEN);
            //if (import_count == i)
            //{    /* All imports are handled internally */
            //	type = get_token(fp, token, MAXTOKEN);
            //	continue;
            //}
            //this_module = which_module(token);
            import.module = token.lexem;


            //for (old_i = i; i < import_count; ++i)
            //	import_list[i].modid = this_module;

            /*
             * Recursively read any pre-requisite modules
             */

            imports.push_back(std::move(import));

            auto status = readModuleInternal(token.lexem);

            if (status == LoadStatus::MODULE_NOT_FOUND)
            {
                status = readImportReplacements(token.lexem, imports);

                if (status == LoadStatus::MODULE_NOT_FOUND)
                {
                    _errinf.isError = true;
                    _errinf.description = "Module " + token.lexem + " not found.";
                    return;
                }
            }

            if (_errinf.isError)
                return;


            //if (read_module_internal(token) == MODULE_NOT_FOUND)
            //{
            //	int found = 0;
            //	for (; old_i < import_count; ++old_i)
            //	{
            //		found += read_import_replacements(token, &import_list[old_i]);
            //	}
            //	if (!found)
            //		print_module_not_found(token);
            //}
        }
        //type = get_token(fp, token, MAXTOKEN);
        parseToken(file, token);
    }

    /* Initialize modid in case the module name was missing. */
    //for (; i < import_count; ++i) //hz
    //	import_list[i].modid = -1;

    /*
     * Save the import information
     *   in the global module table
     */

    auto& module = _tree->findModule(moduleName);

    if (module)
    {
        if (imports.empty())
            return;

        //if (mp->imports && (mp->imports != root_imports)) //todo
        //{
        //	/*
        //	 * this can happen if all modules are in one source file.
        //	 */
        //	for (i = 0; i < mp->no_imports; ++i)
        //	{
        //		DEBUGMSGTL(("parse-mibs",
        //			"#### freeing Module %d '%s' %d\n",
        //			mp->modid, mp->imports[i].label,
        //			mp->imports[i].modid));
        //		free((char*)mp->imports[i].label);
        //	}
        //	free((char*)mp->imports);
        //}

        module->imports = std::move(imports);

        return;

    }

    //for (mp = module_head; mp; mp = mp->next)
    //	if (mp->modid == current_module)
    //	{
    //		if (import_count == 0)
    //			return;
    //		if (mp->imports && (mp->imports != root_imports))
    //		{
    //			/*
    //			 * this can happen if all modules are in one source file.
    //			 */
    //			for (i = 0; i < mp->no_imports; ++i)
    //			{
    //				DEBUGMSGTL(("parse-mibs",
    //					"#### freeing Module %d '%s' %d\n",
    //					mp->modid, mp->imports[i].label,
    //					mp->imports[i].modid));
    //				free((char*)mp->imports[i].label);
    //			}
    //			free((char*)mp->imports);
    //		}
    //		mp->imports = (struct module_import*)
    //			calloc(import_count, sizeof(struct module_import));
    //		if (mp->imports == NULL)
    //			return;
    //		for (i = 0; i < import_count; ++i)
    //		{
    //			mp->imports[i].label = import_list[i].label;
    //			mp->imports[i].modid = import_list[i].modid;
    //			DEBUGMSGTL(("parse-mibs",
    //				"#### adding Module %d '%s' %d\n", mp->modid,
    //				mp->imports[i].label, mp->imports[i].modid));
    //		}
    //		mp->no_imports = import_count;
    //		return;
    //	}

    _errinf.isError = true;
    _errinf.description = "Module " + moduleName + " not found.";

    /*
     * Shouldn't get this far
     */
     //print_module_not_found(module_name(current_module, modbuf)); 
     //return;

}

NodeList Parser::parseObjectGroup(std::ifstream& file, const std::string& objName, const std::string& moduleName, LT what, Objgroup& objgroup)
{
    //int             type;
    //char            token[MAXTOKEN];
    //char            quoted_string_buffer[MAXQUOTESTR];
    //struct node    *np;
    Token token;
    Node::Ptr np(new Node);
    np->label = objName;

    //std::string obj;

    //np = alloc_node(current_module);
    //if (np == nullptr)
    //	return EmptyResult;

    parseToken(file, token);

    //type = get_token(fp, token, MAXTOKEN);
    if (token.type == what)
    {
        parseToken(file, token);
        //type = get_token(fp, token, MAXTOKEN);
        if (token.type != LT::eLEFTBRACKET)
        {
            _errinf.isError = true;
            _errinf.description = formError("Expected \"{\"", token.lexem);
            return EmptyResult;
            //print_error("Expected \"{\"", token, type);
            //free_node(np);
            //return NULL;
            //goto skip;
        }

        do
        {
            //struct objgroup *o;
            //type = get_token(fp, token, MAXTOKEN);
            parseToken(file, token);
            if (token.type != LT::eLABEL)
            {
                _errinf.isError = true;
                _errinf.description = formError("Bad identifier", token.lexem);
                return EmptyResult;
                //print_error("Bad identifier", token, type);
                //free_node(np);
                //return NULL;
                //goto skip;
            }
            //o = (struct objgroup *) malloc(sizeof(struct objgroup));
            //if (!o)
            //{
            //	print_error("Resource failure", token, type);
            //	free_node(np);
            //	return NULL;
            //	//goto skip;
            //}
            //obj = token.lexem;
            //o->line = mibLine;
            //o->name = strdup(token);
            //o->next = *ol;
            //*ol = o;
            objgroup.insert(token.lexem);
            parseToken(file, token);
            //type = get_token(fp, token, MAXTOKEN);
        } while (token.type == LT::eCOMMA);

        if (token.type != LT::eRIGHTBRACKET)
        {
            _errinf.isError = true;
            _errinf.description = formError("Expected \"}\"", token.lexem);
            return EmptyResult;
            //print_error("Expected \"}\" after list", token, type);
            //free_node(np);
            //return NULL;
            //goto skip;
        }
        parseToken(file, token);
        //type = get_token(fp, token, type);
    }

    if (token.type != LT::eSTATUS)
    {
        _errinf.isError = true;
        _errinf.description = formError("Expected STATUS", token.lexem);
        return EmptyResult;
        //print_error("Expected STATUS", token, type);
        //free_node(np);
        //return NULL;
        //goto skip;
    }
    parseToken(file, token);
    //type = get_token(fp, token, MAXTOKEN);


    if (token.type != LT::eCURRENT && token.type != LT::eDEPRECATED && token.type != LT::eOBSOLETE)
    {
        _errinf.isError = true;
        _errinf.description = formError("Bad STATUS value", token.lexem);
        return EmptyResult;
        //	//print_error("Bad STATUS value", token, type);
        //	//free_node(np);
        //	//return NULL;
        //	//goto skip;
    }

    np->status = token.type;

    parseToken(file, token);
    //type = get_token(fp, token, MAXTOKEN);
    if (token.type != LT::eDESCRIPTION)
    {
        _errinf.isError = true;
        _errinf.description = formError("Expected DESCRIPTION", token.lexem);
        return EmptyResult;
        //print_error("Expected DESCRIPTION", token, type);
        //free_node(np);
        //return NULL;
        //goto skip;
    }
    parseToken(file, token);
    //type = get_token(fp, quoted_string_buffer, MAXQUOTESTR);
    if (token.type != LT::eQUOTESTRING)
    {
        _errinf.isError = true;
        _errinf.description = formError("Bad DESCRIPTION", token.lexem);
        return EmptyResult;
        //print_error("Bad DESCRIPTION", quoted_string_buffer, type);
        //free_node(np);
        //return NULL;
    }
    //if (netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID,
    //	NETSNMP_DS_LIB_SAVE_MIB_DESCRS))
    //{
    np->description = token.lexem;
    //}
    parseToken(file, token);
    //type = get_token(fp, token, MAXTOKEN);
    if (token.type == LT::eREFERENCE)
    {
        parseToken(file, token);
        //type = get_token(fp, quoted_string_buffer, MAXQUOTESTR);
        if (token.type != LT::eQUOTESTRING)
        {
            _errinf.isError = true;
            _errinf.description = formError("Bad REFERENCE", token.lexem);
            return EmptyResult;
            //print_error("Bad REFERENCE", quoted_string_buffer, type);
            //free_node(np);
            //return NULL;
        }
        np->reference = token.lexem;
        parseToken(file, token);
        //np->reference = strdup(quoted_string_buffer);
        //type = get_token(fp, token, MAXTOKEN);
    }

    if (token.type != LT::eEQUALS)
    {
        _errinf.isError = true;
        _errinf.description = formError("Expected \"::=\"", token.lexem);
        return EmptyResult;
        //print_error("Expected \"::=\"", token, type);
        //free_node(np);
        //return NULL;
    }

    //skip:
    while (token.type != LT::eEQUALS && token.type != LT::eENDOFFILE)
        parseToken(file, token);
    //type = get_token(fp, token, MAXTOKEN);

    return mergeParsedObjectid(np, file, moduleName);
    //return merge_parse_objectid(np, fp, name);
}


/*
* Parses an OBJECT TYPE macro.
* Returns 0 on error.
*/
NodeList Parser::parseObjType(std::ifstream& file, const std::string& objName, const std::string& moduleName)
{
    //register int    type;
    //char            token[MAXTOKEN];
    //char            nexttoken[MAXTOKEN];
    //char            quoted_string_buffer[MAXQUOTESTR];
    //int             nexttype, tctype;
    //register struct node *np;

    Token token;
    Node::Ptr np(new Node);
    np->label = objName;

    parseToken(file, token);

    //type = get_token(fp, token, MAXTOKEN);
    if (token.type != LT::eSYNTAX)
    {
        _errinf.isError = true;
        _errinf.description = formError("Bad format for OBJECT-TYPE", token.lexem);
        return EmptyResult;
        //print_error("Bad format for OBJECT-TYPE", token, type);
        //return NULL;
    }

    //np = alloc_node(current_module);
    //if (np == NULL)
    //	return (NULL);
    //type = get_token(fp, token, MAXTOKEN);
    parseToken(file, token);

    if (token.type == LT::eOBJECT)
    {
        //type = get_token(fp, token, MAXTOKEN);
        parseToken(file, token);

        if (token.type != LT::eIDENTIFIER)
        {
            _errinf.isError = true;
            _errinf.description = formError("Expected IDENTIFIER", token.lexem);
            return EmptyResult;
            /*print_error("Expected IDENTIFIER", token, type);
            free_node(np);
            return NULL;*/
        }
        token.type = LT::eOBJID;
        //type = OBJID;
    }

    if (token.type == LT::eLABEL)
    {
        int tc_index = _tclist.getTCIndex(token.lexem);

        if (tc_index >= 0)
        {
            token.type = _tclist.getTC(tc_index).type;
            //np->tc_index = tc_index; //hz
        }

        //int             tmp_index;
        //tctype = get_tc(token, current_module, &tmp_index,
        //	&np->enums, &np->ranges, &np->hint);
        //if (tctype == LABEL &&
        //	netsnmp_ds_get_int(NETSNMP_DS_LIBRARY_ID,
        //		NETSNMP_DS_LIB_MIB_WARNINGS) > 1)
        //{
        //	print_error("Warning: No known translation for type", token,
        //		type);
        //}
        //type = tctype;
        //np->tc_index = tmp_index;       /* store TC for later reference */
    }

    np->type = token.type;

    LT lasttype = token.type;

    parseToken(file, token);
    //nexttype = get_token(fp, nexttoken, MAXTOKEN);

    switch (lasttype)
    {
    case LT::eSEQUENCE:

        //parseToken(file, token);
        if (token.type == LT::eOF)
        {
            parseToken(file, token);
            parseToken(file, token);
            //nexttype = get_token(fp, nexttoken, MAXTOKEN);
            //nexttype = get_token(fp, nexttoken, MAXTOKEN);

        }
        break;
    case LT::eINTEGER:
    case LT::eINTEGER32:
    case LT::eUINTEGER32:
    case LT::eUNSIGNED32:
    case LT::eCOUNTER:
    case LT::eGAUGE:
    case LT::eBITSTRING:
    case LT::eLABEL:

        //parseToken(file, token);
        if (token.type == LT::eLEFTBRACKET)
        {
            /*
            * if there is an enumeration list, parse it
            */
            //np->enums = parse_enumlist(fp, &np->enums);

            /*if (!np->enums)
            {
                free_node(np);
                return NULL;
            }*/

            np->enums = parseEnums(file);

            if (np->enums.empty())
            {
                return EmptyResult;
            }

            parseToken(file, token);
            //nexttype = get_token(fp, nexttoken, MAXTOKEN);
        }
        else if (token.type == LT::eLEFTPAREN)
        {
            /*
            * if there is a range list, parse it
            */
            /*np->ranges = parse_ranges(fp, &np->ranges);

            if (!np->ranges)
            {
                free_node(np);
                return NULL;
            }*/

            np->ranges = parseRanges(file);

            if (np->ranges.empty())
            {
                return EmptyResult;
            }

            parseToken(file, token);
            //nexttype = get_token(fp, nexttoken, MAXTOKEN);
        }
        break;
    case LT::eOCTETSTR:
    case LT::eOPAQUE:

        //parseToken(file, token);
        /*
        * parse any SIZE specification
        */
        if (token.type == LT::eLEFTPAREN)
        {
            parseToken(file, token);
            //nexttype = get_token(fp, nexttoken, MAXTOKEN);
            if (token.type == LT::eSIZE)
            {
                parseToken(file, token);
                //nexttype = get_token(fp, nexttoken, MAXTOKEN);
                if (token.type == LT::eLEFTPAREN)
                {
                    np->ranges = parseRanges(file);

                    if (np->ranges.empty())
                        return EmptyResult;

                    //np->ranges = parse_ranges(fp, &np->ranges); 
                    //nexttype = get_token(fp, nexttoken, MAXTOKEN);      /* ) */
                    parseToken(file, token);
                    if (token.type == LT::eRIGHTPAREN)
                    {
                        parseToken(file, token);
                        //nexttype = get_token(fp, nexttoken, MAXTOKEN);
                        break;
                    }
                }
            }

            _errinf.isError = true;
            _errinf.description = formError("Bad SIZE syntax", token.lexem);
            //print_error("Bad SIZE syntax", token, type);
            //free_node(np);
            return EmptyResult;
        }
        break;
    case LT::eOBJID:
    case LT::eNETADDR:
    case LT::eIPADDR:
    case LT::eTIMETICKS:
    case LT::eNULL:
    case LT::eNSAPADDRESS:
    case LT::eCOUNTER64:
        break;
    default:
        _errinf.isError = true;
        _errinf.description = formError("Bad syntax", token.lexem);
        return EmptyResult;
        //print_error("Bad syntax", token, type);
        //free_node(np);
        //return NULL;
    }

    if (token.type == LT::eUNITS)
    {
        parseToken(file, token);
        //type = get_token(fp, quoted_string_buffer, MAXQUOTESTR);
        if (token.type != LT::eQUOTESTRING)
        {
            //print_error("Bad UNITS", quoted_string_buffer, type);
            //free_node(np);
            //return NULL;
            _errinf.isError = true;
            _errinf.description = formError("Bad UNITS", token.lexem);
            return EmptyResult;
        }
        np->units = token.lexem;
        parseToken(file, token);
        //nexttype = get_token(fp, nexttoken, MAXTOKEN);
    }

    if (token.type != LT::eACCESS)
    {
        _errinf.isError = true;
        _errinf.description = formError("Should be ACCESS", token.lexem);
        return EmptyResult;
        /*print_error("Should be ACCESS", nexttoken, nexttype);
        free_node(np);
        return NULL;*/
    }
    parseToken(file, token);
    //type = get_token(fp, token, MAXTOKEN);


    if (token.type != LT::eREADONLY && token.type != LT::eREADWRITE
        && token.type != LT::eWRITEONLY && token.type != LT::eNOACCESS
        && token.type != LT::eREADCREATE && token.type != LT::eACCNOTIFY)
    {
        _errinf.isError = true;
        _errinf.description = formError("Bad ACCESS type", token.lexem);
        return EmptyResult;
        /*print_error("Bad ACCESS type", token, type);
        free_node(np);
        return NULL;*/
    }

    np->access = token.type;

    parseToken(file, token);
    //type = get_token(fp, token, MAXTOKEN);
    if (token.type != LT::eSTATUS)
    {
        _errinf.isError = true;
        _errinf.description = formError("Should be STATUS", token.lexem);
        return EmptyResult;
        //print_error("Should be STATUS", token, type);
        //free_node(np);
        //return NULL;
    }
    parseToken(file, token);
    //type = get_token(fp, token, MAXTOKEN);
    if (token.type != LT::eMANDATORY && token.type != LT::eCURRENT
        && token.type != LT::eOPTIONAL && token.type != LT::eOBSOLETE
        && token.type != LT::eDEPRECATED)
    {
        _errinf.isError = true;
        _errinf.description = formError("Bad STATUS", token.lexem);
        return EmptyResult;
        /*print_error("Bad STATUS", token, type);
        free_node(np);
        return NULL;*/
    }
    np->status = token.type;
    /*
    * Optional parts of the OBJECT-TYPE macro
    */
    parseToken(file, token);
    //type = get_token(fp, token, MAXTOKEN);
    while (token.type != LT::eEQUALS && token.type != LT::eENDOFFILE)
    {
        switch (token.type)
        {
        case LT::eDESCRIPTION:
            //type = get_token(fp, quoted_string_buffer, MAXQUOTESTR);
            parseToken(file, token);

            if (token.type != LT::eQUOTESTRING)
            {
                _errinf.isError = true;
                _errinf.description = formError("Bad DESCRIPTION", token.lexem);
                return EmptyResult;
                //print_error("Bad DESCRIPTION", quoted_string_buffer, type);
                //free_node(np);
                //return NULL;
            }
            /*if (netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID,
                NETSNMP_DS_LIB_SAVE_MIB_DESCRS))
            {*/
            np->description = token.lexem;
            //}
            break;

        case LT::eREFERENCE:
            parseToken(file, token);
            //type = get_token(fp, quoted_string_buffer, MAXQUOTESTR);
            if (token.type != LT::eQUOTESTRING)
            {
                _errinf.isError = true;
                _errinf.description = formError("Bad REFERENCE", token.lexem);
                return EmptyResult;
                //print_error("Bad REFERENCE", quoted_string_buffer, type);
                //free_node(np);
                //return NULL;
            }
            np->reference = token.lexem;
            break;
        case LT::eINDEX:

            if (!np->augments.empty())
            {
                _errinf.isError = true;
                _errinf.description = formError("Cannot have both INDEX and AUGMENTS", token.lexem);
                return EmptyResult;
                //print_error("Cannot have both INDEX and AUGMENTS", token,
                //	type);
                //free_node(np);
                //return NULL;
            }
            //np->indexes = getIndexes(fp, &np->indexes); 
            //if (np->indexes == NULL)
            //{
            //	print_error("Bad INDEX list", token, type);
            //	free_node(np);
            //	return NULL;
            //}
            np->indexes = parseIndexes(file);

            if (np->indexes.empty())
            {
                _errinf.isError = true;
                _errinf.description = formError("Bad INDEX list", token.lexem);
                return EmptyResult;
            }

            break;
        case LT::eAUGMENTS:
            //if (np->indexes)
            if (!np->indexes.empty())
            {
                _errinf.isError = true;
                _errinf.description = formError("Cannot have both INDEX and AUGMENTS", token.lexem);
                return EmptyResult;
                /*print_error("Cannot have both INDEX and AUGMENTS", token,
                    type);
                free_node(np);
                return NULL;*/
            }
            //np->indexes = getIndexes(fp, &np->indexes);
            np->indexes = parseIndexes(file);
            //if (np->indexes == NULL)
            if (np->indexes.empty())
            {
                _errinf.isError = true;
                _errinf.description = formError("Bad AUGMENTS list", token.lexem);
                return EmptyResult;
                /*print_error("Bad AUGMENTS list", token, type);
                free_node(np);
                return NULL;*/
            }

            np->augments = np->indexes.front().ilabel;

            //np->augments = strdup(np->indexes->ilabel);
            //free_indexes(&np->indexes);
            break;
        case LT::eDEFVAL:
            /*
            * Mark's defVal section
            */
            //type = get_token(fp, quoted_string_buffer, MAXTOKEN);
            parseToken(file, token);
            if (token.type != LT::eLEFTBRACKET)
            {
                _errinf.isError = true;
                _errinf.description = formError("Bad DEFAULTVALUE", token.lexem);
                return EmptyResult;
                /*print_error("Bad DEFAULTVALUE", quoted_string_buffer,
                    type);
                free_node(np);
                return NULL;*/
            }

            {
                int             level = 1;
                std::string defbuf;

                while (true)
                {
                    parseToken(file, token);
                    //type = get_token(fp, quoted_string_buffer, MAXTOKEN);
                    if ((token.type == LT::eRIGHTBRACKET && --level == 0)
                        || token.type == LT::eENDOFFILE)
                        break;
                    else if (token.type == LT::eLEFTBRACKET)
                        level++;
                    if (token.type == LT::eQUOTESTRING)
                        defbuf += "\\\"";
                    //strlcat(defbuf, "\\\"", sizeof(defbuf));
                    defbuf += token.lexem;
                    //strlcat(defbuf, quoted_string_buffer, sizeof(defbuf));
                    if (token.type == LT::eQUOTESTRING)
                        defbuf += "\\\"";
                    //strlcat(defbuf, "\\\"", sizeof(defbuf));
                    defbuf += " ";
                    //strlcat(defbuf, " ", sizeof(defbuf));
                }

                if (token.type != LT::eRIGHTBRACKET)
                {
                    _errinf.isError = true;
                    _errinf.description = formError("Bad DEFAULTVALUE", token.lexem);
                    return EmptyResult;
                    /*print_error("Bad DEFAULTVALUE", quoted_string_buffer,
                        type);
                    free_node(np);
                    return NULL;*/
                }

                //defbuf[strlen(defbuf) - 1] = 0;
                np->defaultValue = defbuf;
            }

            break;

        case LT::eNUM_ENTRIES:

            tossObjectIdentifier(file, token);

            if (token.type != LT::eOBJID)
            {
                _errinf.isError = true;
                _errinf.description = formError("Bad Object Identifier", token.lexem);
                return EmptyResult;
            }

            /*if (tossObjectIdentifier(fp) != OBJID)
            {
                print_error("Bad Object Identifier", token, type);
                free_node(np);
                return NULL;
            }*/
            break;

        default:

            _errinf.isError = true;
            _errinf.description = formError("Bad format of optional clauses", token.lexem);
            return EmptyResult;
            /*print_error("Bad format of optional clauses", token, type);
            free_node(np);
            return NULL;*/

        }
        parseToken(file, token);
        //type = get_token(fp, token, MAXTOKEN);
    }
    if (token.type != LT::eEQUALS)
    {
        _errinf.isError = true;
        _errinf.description = formError("Bad format", token.lexem);
        return EmptyResult;
        //print_error("Bad format", token, type);
        //free_node(np);
        //return NULL;
    }
    return mergeParsedObjectid(np, file, moduleName);
    //return merge_parse_objectid(np, fp, name);
}

/*
* Parses a NOTIFICATION-TYPE macro.
* Returns 0 on error.
*/
NodeList Parser::parseNotifType(std::ifstream& file, const std::string& objName, const std::string& moduleName)
{
    //register int    type;
    //char            token[MAXTOKEN];
    //char            quoted_string_buffer[MAXQUOTESTR];
    //register struct node *np;

    Node::Ptr np(new Node);
    np->label = objName;

    Token token;

    //np = alloc_node(current_module);
    //if (np == NULL)
    //	return (NULL);
    //type = get_token(fp, token, MAXTOKEN);
    parseToken(file, token);

    while (token.type != LT::eEQUALS && token.type != LT::eENDOFFILE) //mb
    {
        switch (token.type)
        {
        case LT::eSTATUS:

            parseToken(file, token);
            //type = get_token(fp, token, MAXTOKEN);


            if (token.type != LT::eCURRENT && token.type != LT::eDEPRECATED && token.type != LT::eOBSOLETE)
            {
                _errinf.isError = true;
                _errinf.description = formError("Bad STATUS value", token.lexem);
                return EmptyResult;
                //	//print_error("Bad STATUS value", token, type);
                //	//free_node(np);
                //	//return NULL;
                //	//goto skip;
            }

            np->status = token.type;

            break;
        case LT::eDESCRIPTION:
            //type = get_token(fp, quoted_string_buffer, MAXQUOTESTR);
            parseToken(file, token);

            if (token.type != LT::eQUOTESTRING)
            {
                _errinf.isError = true;
                _errinf.description = formError("Bad DESCRIPTION", token.lexem);
                return EmptyResult;
                //print_error("Bad DESCRIPTION", quoted_string_buffer, type);
                //free_node(np);
                //return NULL;
            }
            //if (netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID,
            //	NETSNMP_DS_LIB_SAVE_MIB_DESCRS))
            //{
            np->description = token.lexem;//strdup(quoted_string_buffer);
            //}
            break;
        case LT::eREFERENCE:
            //type = get_token(fp, quoted_string_buffer, MAXQUOTESTR);
            parseToken(file, token);
            if (token.type != LT::eQUOTESTRING)
            {
                _errinf.isError = true;
                _errinf.description = formError("Bad REFERENCE", token.lexem);
                return EmptyResult;
                //print_error("Bad REFERENCE", quoted_string_buffer, type);
                //free_node(np);
                //return NULL;
            }
            np->reference = token.lexem;//strdup(quoted_string_buffer);
            break;
        case LT::eOBJECTS:
            //np->varbinds = getVarbinds(fp, &np->varbinds);
            np->varbinds = parseVarbinds(file);

            //if (!np->varbinds)
            if (np->varbinds.empty())
            {
                _errinf.isError = true;
                _errinf.description = formError("Bad OBJECTS list", token.lexem);
                return EmptyResult;
                //print_error("Bad OBJECTS list", token, type);
                //free_node(np);
                //return NULL;
            }
            break;
        default:
            /*
            * NOTHING
            */
            break;
        }

        parseToken(file, token);
        //type = get_token(fp, token, MAXTOKEN);
    }
    return mergeParsedObjectid(np, file, moduleName);
    //return merge_parse_objectid(np, fp, name);
}


NodeList Parser::parseCompliance(std::ifstream& file, const std::string& objName, const std::string& moduleName)
{
    //int             type;
    //char            token[MAXTOKEN];
    //char            quoted_string_buffer[MAXQUOTESTR];
    //struct node    *np;
    Node::Ptr np(new Node);
    np->label = objName;
    Token token;

    //np = alloc_node(current_module);
    //if (np == NULL)
    //	return (NULL);
    //type = get_token(fp, token, MAXTOKEN);
    parseToken(file, token);

    if (token.type != LT::eSTATUS)
    {
        _errinf.isError = true;
        _errinf.description = formError("Expected STATUS", token.lexem);
        return EmptyResult;
        //print_error("Expected STATUS", token, type);
        //goto skip;
    }

    //type = get_token(fp, token, MAXTOKEN);
    parseToken(file, token);

    if (token.type != LT::eCURRENT && token.type != LT::eDEPRECATED && token.type != LT::eOBSOLETE)
    {
        _errinf.isError = true;
        _errinf.description = formError("Bad STATUS", token.lexem);
        return EmptyResult;
        //print_error("Bad STATUS", token, type);
        //goto skip;
    }
    np->status = token.type;

    parseToken(file, token);
    //type = get_token(fp, token, MAXTOKEN);
    if (token.type != LT::eDESCRIPTION)
    {
        _errinf.isError = true;
        _errinf.description = formError("Expected DESCRIPTION", token.lexem);
        return EmptyResult;
        //print_error("Expected DESCRIPTION", token, type);
        //goto skip;
    }

    parseToken(file, token);
    //type = get_token(fp, quoted_string_buffer, MAXQUOTESTR);
    if (token.type != LT::eQUOTESTRING)
    {
        _errinf.isError = true;
        _errinf.description = formError("Bad DESCRIPTION", token.lexem);
        return EmptyResult;
        //print_error("Bad DESCRIPTION", quoted_string_buffer, type);
        //goto skip;
    }
    //if (netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID,
    //	NETSNMP_DS_LIB_SAVE_MIB_DESCRS))
    np->description = token.lexem; //strdup(quoted_string_buffer);
    //type = get_token(fp, token, MAXTOKEN);
    parseToken(file, token);

    if (token.type == LT::eREFERENCE)
    {
        //type = get_token(fp, quoted_string_buffer, MAXTOKEN);
        parseToken(file, token);

        if (token.type != LT::eQUOTESTRING)
        {
            _errinf.isError = true;
            _errinf.description = formError("Bad REFERENCE", token.lexem);
            return EmptyResult;
            //print_error("Bad REFERENCE", quoted_string_buffer, type);
            //goto skip;
        }
        np->reference = token.lexem; //strdup(quoted_string_buffer);
        //type = get_token(fp, token, MAXTOKEN);
        parseToken(file, token);
    }

    if (token.type != LT::eMODULE)
    {
        _errinf.isError = true;
        _errinf.description = formError("Expected MODULE", token.lexem);
        return EmptyResult;
        //print_error("Expected MODULE", token, type);
        //goto skip;
    }

    while (token.type == LT::eMODULE)
    {
        std::string module;
        //int             modid = -1;
        //char            modname[MAXTOKEN];
        //type = get_token(fp, token, MAXTOKEN);
        parseToken(file, token);

        if (token.type == LT::eLABEL && token.lexem != moduleName)//strcmp(token, module_name(current_module, modname)))
        {

            LoadStatus status = readModuleInternal(token.lexem);

            if (status != LoadStatus::MODULE_LOADED_OK && status != LoadStatus::MODULE_ALREADY_LOADED)
            {
                _errinf.isError = true;
                _errinf.description = formError("Unknown module", token.lexem);
                return EmptyResult;
            }

            //module = token.lexem;

            //modid = read_module_internal(token);
            //
            //if (modid != MODULE_LOADED_OK && modid != MODULE_ALREADY_LOADED)
            //{
            //	print_error("Unknown module", token, type);
            //	goto skip;
            //}
            //
            //modid = which_module(token);
            parseToken(file, token);
            //type = get_token(fp, token, MAXTOKEN);
        }

        if (token.type == LT::eMANDATORYGROUPS)
        {
            //type = get_token(fp, token, MAXTOKEN);
            parseToken(file, token);

            if (token.type != LT::eLEFTBRACKET)
            {
                _errinf.isError = true;
                _errinf.description = formError("Expected \"{\"", token.lexem);
                return EmptyResult;
                //print_error("Expected \"{\"", token, type);
                //goto skip;
            }
            do
            {
                parseToken(file, token);
                //type = get_token(fp, token, MAXTOKEN);
                if (token.type != LT::eLABEL)
                {
                    _errinf.isError = true;
                    _errinf.description = formError("Bad group name", token.lexem);
                    return EmptyResult;
                    //print_error("Bad group name", token, type);
                    //goto skip;
                }

                if (!complianceLookup(token.lexem))
                {
                    _errinf.isError = true;
                    _errinf.description = formError("Unknown group", token.lexem);
                    return EmptyResult;
                }

                //if (!compliance_lookup(token, modid))
                //	print_error("Unknown group", token, type);
                //type = get_token(fp, token, MAXTOKEN);
                parseToken(file, token);

            } while (token.type == LT::eCOMMA);

            if (token.type != LT::eRIGHTBRACKET)
            {
                _errinf.isError = true;
                _errinf.description = formError("Expected \"}\"", token.lexem);
                return EmptyResult;
                //print_error("Expected \"}\"", token, type);
                //goto skip;
            }

            parseToken(file, token);
            //type = get_token(fp, token, MAXTOKEN);
        }

        while (token.type == LT::eGROUP || token.type == LT::eOBJECT)
        {
            if (token.type == LT::eGROUP)
            {
                parseToken(file, token);
                //type = get_token(fp, token, MAXTOKEN);

                if (token.type != LT::eLABEL)
                {
                    _errinf.isError = true;
                    _errinf.description = formError("Bad group name", token.lexem);
                    return EmptyResult;
                    //print_error("Bad group name", token, type);
                    //goto skip;
                }

                if (!complianceLookup(token.lexem))
                {
                    _errinf.isError = true;
                    _errinf.description = formError("Unknown group", token.lexem);
                    return EmptyResult;
                }
                //if (!compliance_lookup(token, modid))
                //	print_error("Unknown group", token, type);
                parseToken(file, token);

                //type = get_token(fp, token, MAXTOKEN);
            }
            else
            {
                parseToken(file, token);
                //type = get_token(fp, token, MAXTOKEN);
                if (token.type != LT::eLABEL)
                {
                    _errinf.isError = true;
                    _errinf.description = formError("Bad object name", token.lexem);
                    return EmptyResult;
                    //print_error("Bad object name", token, type);
                    //goto skip;
                }

                if (!complianceLookup(token.lexem))
                {
                    _errinf.isError = true;
                    _errinf.description = formError("Unknown group", token.lexem);
                    return EmptyResult;
                }
                //if (!compliance_lookup(token, modid))
                //	print_error("Unknown group", token, type);
                parseToken(file, token);
                //type = get_token(fp, token, MAXTOKEN);
                if (token.type == LT::eSYNTAX)
                    eatSyntax(file, token);
                //type = eat_syntax(fp, token, MAXTOKEN);
                if (token.type == LT::eWRSYNTAX)
                    eatSyntax(file, token);
                //type = eat_syntax(fp, token, MAXTOKEN);
                if (token.type == LT::eMINACCESS)
                {
                    //type = get_token(fp, token, MAXTOKEN);
                    parseToken(file, token);

                    if (token.type != LT::eNOACCESS && token.type != LT::eACCNOTIFY
                        && token.type != LT::eREADONLY && token.type != LT::eWRITEONLY
                        && token.type != LT::eREADCREATE && token.type != LT::eREADWRITE)
                    {
                        _errinf.isError = true;
                        _errinf.description = formError("Bad MIN-ACCESS spec", token.lexem);
                        return EmptyResult;
                        //print_error("Bad MIN-ACCESS spec", token, type);
                        //goto skip;
                    }

                    np->access = token.type;

                    parseToken(file, token);
                    //type = get_token(fp, token, MAXTOKEN);
                }
            }

            if (token.type != LT::eDESCRIPTION)
            {
                _errinf.isError = true;
                _errinf.description = formError("Expected DESCRIPTION", token.lexem);
                return EmptyResult;
                //print_error("Expected DESCRIPTION", token, type);
                //goto skip;
            }
            parseToken(file, token);
            //type = get_token(fp, token, MAXTOKEN);
            if (token.type != LT::eQUOTESTRING)
            {
                _errinf.isError = true;
                _errinf.description = formError("Bad DESCRIPTION", token.lexem);
                return EmptyResult;
                //print_error("Bad DESCRIPTION", token, type);
                //goto skip;
            }

            parseToken(file, token);
            //type = get_token(fp, token, MAXTOKEN);
        }
    }
    //skip:
    //	while (type != EQUALS && type != ENDOFFILE)
    //		type = get_token(fp, quoted_string_buffer, MAXQUOTESTR);

    return mergeParsedObjectid(np, file, moduleName);
    //return merge_parse_objectid(np, fp, name);
}

NodeList Parser::parseModuleIdentity(std::ifstream& file, const std::string& objName, const std::string& moduleName)
{
    //register int    type;
    //char            token[MAXTOKEN];
    //char            quoted_string_buffer[MAXQUOTESTR];
    //register struct node *np;

    Node::Ptr np(new Node);
    np->label = objName;

    Token token;

    //np = alloc_node(current_module);
    //if (np == NULL)
    //	return (NULL);
    //type = get_token(fp, token, MAXTOKEN);

    parseToken(file, token);

    if (token.type != LT::eLASTUPDATED)
    {
        _errinf.isError = true;
        _errinf.description = formError("Expected LAST-UPDATED", token.lexem);
        return EmptyResult;
        //print_error("Expected LAST-UPDATED", token, type);
        //goto skip;
    }

    parseToken(file, token);
    //type = get_token(fp, token, MAXTOKEN);
    if (token.type != LT::eQUOTESTRING)
    {
        _errinf.isError = true;
        _errinf.description = formError("Need STRING for LAST-UPDATED", token.lexem);
        return EmptyResult;
        //print_error("Need STRING for LAST-UPDATED", token, type);
        //goto skip;
    }

    //check_utc(token);

    token.lexem.erase(token.lexem.begin()); //hz
    token.lexem.pop_back();

    if (!checkUTC(token.lexem))
        return EmptyResult;

    parseToken(file, token);

    //type = get_token(fp, token, MAXTOKEN);
    if (token.type != LT::eORGANIZATION)
    {
        _errinf.isError = true;
        _errinf.description = formError("Expected ORGANIZATION", token.lexem);
        return EmptyResult;
        //print_error("Expected ORGANIZATION", token, type);
        //goto skip;
    }

    parseToken(file, token);
    //type = get_token(fp, token, MAXTOKEN);
    if (token.type != LT::eQUOTESTRING)
    {
        _errinf.isError = true;
        _errinf.description = formError("Bad ORGANIZATION", token.lexem);
        return EmptyResult;
        //print_error("Bad ORGANIZATION", token, type);
        //goto skip;
    }

    parseToken(file, token);
    //type = get_token(fp, token, MAXTOKEN);

    if (token.type != LT::eCONTACTINFO)
    {
        _errinf.isError = true;
        _errinf.description = formError("Expected CONTACT-INFO", token.lexem);
        return EmptyResult;
        //print_error("Expected CONTACT-INFO", token, type);
        //goto skip;
    }
    //type = get_token(fp, quoted_string_buffer, MAXQUOTESTR);
    parseToken(file, token);

    if (token.type != LT::eQUOTESTRING)
    {
        _errinf.isError = true;
        _errinf.description = formError("Bad CONTACT-INFO", token.lexem);
        return EmptyResult;
        //print_error("Bad CONTACT-INFO", quoted_string_buffer, type);
        //goto skip;
    }

    parseToken(file, token);
    //type = get_token(fp, token, MAXTOKEN);
    if (token.type != LT::eDESCRIPTION)
    {
        _errinf.isError = true;
        _errinf.description = formError("Expected DESCRIPTION", token.lexem);
        return EmptyResult;
        //print_error("Expected DESCRIPTION", token, type);
        //goto skip;
    }

    parseToken(file, token);
    //type = get_token(fp, quoted_string_buffer, MAXQUOTESTR);
    if (token.type != LT::eQUOTESTRING)
    {
        _errinf.isError = true;
        _errinf.description = formError("Bad DESCRIPTION", token.lexem);
        return EmptyResult;
        //print_error("Bad DESCRIPTION", quoted_string_buffer, type);
        //goto skip;
    }
    //if (netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID,
    //	NETSNMP_DS_LIB_SAVE_MIB_DESCRS))
    //{
    np->description = token.lexem;//strdup(quoted_string_buffer);
    //}
    parseToken(file, token);
    //type = get_token(fp, token, MAXTOKEN);
    while (token.type == LT::eREVISION)
    {
        //type = get_token(fp, token, MAXTOKEN);
        parseToken(file, token);

        if (token.type != LT::eQUOTESTRING)
        {
            _errinf.isError = true;
            _errinf.description = formError("Bad REVISION", token.lexem);
            return EmptyResult;
            //print_error("Bad REVISION", token, type);
            //goto skip;
        }
        //check_utc(token); 

        token.lexem.erase(token.lexem.begin()); //hz
        token.lexem.pop_back();

        if (!checkUTC(token.lexem))
            return EmptyResult;

        parseToken(file, token);
        //type = get_token(fp, token, MAXTOKEN);
        if (token.type != LT::eDESCRIPTION)
        {
            _errinf.isError = true;
            _errinf.description = formError("Expected DESCRIPTION", token.lexem);
            return EmptyResult;
            //print_error("Expected DESCRIPTION", token, type);
            //goto skip;
        }
        //type = get_token(fp, quoted_string_buffer, MAXQUOTESTR);
        parseToken(file, token);

        if (token.type != LT::eQUOTESTRING)
        {
            _errinf.isError = true;
            _errinf.description = formError("Bad DESCRIPTION", token.lexem);
            return EmptyResult;
            //print_error("Bad DESCRIPTION", quoted_string_buffer, type);
            //goto skip;
        }

        parseToken(file, token);
        //type = get_token(fp, token, MAXTOKEN);
    }

    if (token.type != LT::eEQUALS)
    {
        _errinf.isError = true;
        _errinf.description = formError("Expected \"::=\"", token.lexem);
        return EmptyResult;
    }
    //print_error("Expected \"::=\"", token, type);

    return mergeParsedObjectid(np, file, moduleName);
    //skip:
    //	while (type != EQUALS && type != ENDOFFILE)
    //	{
    //		type = get_token(fp, quoted_string_buffer, MAXQUOTESTR);
    //	}
    //	return merge_parse_objectid(np, fp, name);
}

/*
* Parses a capabilities macro
* Returns 0 on error.
*/
NodeList Parser::parseCapabilities(std::ifstream& file, const std::string& objName, const std::string& moduleName)
{
    //int             type;
    //char            token[MAXTOKEN];
    //char            quoted_string_buffer[MAXQUOTESTR];
    //struct node    *np;
    Node::Ptr np(new Node);
    np->label = objName;

    Token token;


    //np = alloc_node(current_module);
    //if (np == NULL)
    //	return (NULL);
    //type = get_token(fp, token, MAXTOKEN);
    parseToken(file, token);

    if (token.type != LT::ePRODREL)
    {
        _errinf.isError = true;
        _errinf.description = formError("Expected PRODUCT-RELEASE", token.lexem);
        return EmptyResult;
        //print_error("Expected PRODUCT-RELEASE", token, type);
        //goto skip;
    }
    //type = get_token(fp, token, MAXTOKEN);
    parseToken(file, token);

    if (token.type != LT::eQUOTESTRING)
    {
        _errinf.isError = true;
        _errinf.description = formError("Expected STRING after PRODUCT-RELEASE", token.lexem);
        return EmptyResult;
        //print_error("Expected STRING after PRODUCT-RELEASE", token, type);
        //goto skip;
    }
    //type = get_token(fp, token, MAXTOKEN);
    parseToken(file, token);

    if (token.type != LT::eSTATUS)
    {
        _errinf.isError = true;
        _errinf.description = formError("Expected STATUS", token.lexem);
        return EmptyResult;
        //print_error("Expected STATUS", token, type);
        //goto skip;
    }
    //type = get_token(fp, token, MAXTOKEN);
    parseToken(file, token);

    if (token.type != LT::eCURRENT && token.type != LT::eOBSOLETE)
    {
        _errinf.isError = true;
        _errinf.description = formError("STATUS should be current or obsolete", token.lexem);
        return EmptyResult;
        //print_error("STATUS should be current or obsolete", token, type);
        //goto skip;
    }

    np->status = token.type;
    //type = get_token(fp, token, MAXTOKEN);
    parseToken(file, token);

    if (token.type != LT::eDESCRIPTION)
    {
        _errinf.isError = true;
        _errinf.description = formError("Expected DESCRIPTION", token.lexem);
        return EmptyResult;
        //print_error("Expected DESCRIPTION", token, type);
        //goto skip;
    }
    //type = get_token(fp, quoted_string_buffer, MAXTOKEN);
    parseToken(file, token);

    if (token.type != LT::eQUOTESTRING)
    {
        _errinf.isError = true;
        _errinf.description = formError("Bad DESCRIPTION", token.lexem);
        return EmptyResult;
        //print_error("Bad DESCRIPTION", quoted_string_buffer, type);
        //goto skip;
    }
    //if (netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID,
    //	NETSNMP_DS_LIB_SAVE_MIB_DESCRS))
    //{
    np->description = token.lexem;//strdup(quoted_string_buffer);
    //}
    //type = get_token(fp, token, MAXTOKEN);
    parseToken(file, token);

    if (token.type == LT::eREFERENCE)
    {
        //type = get_token(fp, quoted_string_buffer, MAXTOKEN);
        parseToken(file, token);

        if (token.type != LT::eQUOTESTRING)
        {
            _errinf.isError = true;
            _errinf.description = formError("Bad REFERENCE", token.lexem);
            return EmptyResult;
            //print_error("Bad REFERENCE", quoted_string_buffer, type);
            //goto skip;
        }
        np->reference = token.lexem;//strdup(quoted_string_buffer);
        //type = get_token(fp, token, type);
        parseToken(file, token);
    }

    while (token.type == LT::eSUPPORTS)
    {
        std::string module;
        //int             modid;
        //struct tree    *tp;

        //type = get_token(fp, token, MAXTOKEN);
        parseToken(file, token);

        if (token.type != LT::eLABEL)
        {
            _errinf.isError = true;
            _errinf.description = formError("Bad module name", token.lexem);
            return EmptyResult;
            //print_error("Bad module name", token, type);
            //goto skip;
        }
        //modid = read_module_internal(token); 

        LoadStatus status = readModuleInternal(token.lexem);

        if (status != LoadStatus::MODULE_LOADED_OK && status != LoadStatus::MODULE_ALREADY_LOADED)
        {
            _errinf.isError = true;
            _errinf.description = formError("Module not found", token.lexem);
            return EmptyResult;
        }

        /*if (modid != MODULE_LOADED_OK && modid != MODULE_ALREADY_LOADED)
        {
            print_error("Module not found", token, type);
            goto skip;
        }*/

        module = token.lexem;
        //modid = which_module(token);
        //type = get_token(fp, token, MAXTOKEN);
        parseToken(file, token);

        if (token.type != LT::eINCLUDES)
        {
            _errinf.isError = true;
            _errinf.description = formError("Expected INCLUDES", token.lexem);
            return EmptyResult;
            //print_error("Expected INCLUDES", token, type);
            //goto skip;
        }
        //type = get_token(fp, token, MAXTOKEN);
        parseToken(file, token);

        if (token.type != LT::eLEFTBRACKET)
        {
            _errinf.isError = true;
            _errinf.description = formError("Expected \"{\"", token.lexem);
            return EmptyResult;
            //print_error("Expected \"{\"", token, type);
            //goto skip;
        }
        do
        {
            //type = get_token(fp, token, MAXTOKEN);
            parseToken(file, token);

            if (token.type != LT::eLABEL)
            {
                _errinf.isError = true;
                _errinf.description = formError("Expected group name", token.lexem);
                return EmptyResult;
                //print_error("Expected group name", token, type);
                //goto skip;
            }

            //tp = find_tree_node(token, modid);

            if (_tree->findNode(token.lexem, module) == nullptr)
            {
                _errinf.isError = true;
                _errinf.description = formError("Group not found in module", token.lexem);
                return EmptyResult;
            }

            //if (!tp)
            //	print_error("Group not found in module", token, type);
            //type = get_token(fp, token, MAXTOKEN);
            parseToken(file, token);

        } while (token.type == LT::eCOMMA);

        if (token.type != LT::eRIGHTBRACKET)
        {
            _errinf.isError = true;
            _errinf.description = formError("Expected \"}\" after group list", token.lexem);
            return EmptyResult;
            //print_error("Expected \"}\" after group list", token, type);
            //goto skip;
        }
        //type = get_token(fp, token, MAXTOKEN);
        parseToken(file, token);

        while (token.type == LT::eVARIATION)
        {
            //type = get_token(fp, token, MAXTOKEN);
            parseToken(file, token);

            if (token.type != LT::eLABEL)
            {
                _errinf.isError = true;
                _errinf.description = formError("Bad object name", token.lexem);
                return EmptyResult;
                //print_error("Bad object name", token, type);
                //goto skip;
            }

            if (_tree->findNode(token.lexem, module) == nullptr)
            {
                _errinf.isError = true;
                _errinf.description = formError("Object not found in module", token.lexem);
                return EmptyResult;
            }

            //tp = find_tree_node(token, modid);
            //if (!tp)
            //	print_error("Object not found in module", token, type);
            //type = get_token(fp, token, MAXTOKEN);
            parseToken(file, token);

            if (token.type == LT::eSYNTAX)
            {
                eatSyntax(file, token);
                //type = eat_syntax(fp, token, MAXTOKEN);
            }
            if (token.type == LT::eWRSYNTAX)
            {
                eatSyntax(file, token);
                //type = eat_syntax(fp, token, MAXTOKEN);
            }
            if (token.type == LT::eACCESS)
            {
                //type = get_token(fp, token, MAXTOKEN);
                parseToken(file, token);

                if (token.type != LT::eACCNOTIFY && token.type != LT::eREADONLY
                    && token.type != LT::eREADWRITE && token.type != LT::eREADCREATE
                    && token.type != LT::eWRITEONLY && token.type != LT::eNOTIMPL)
                {
                    _errinf.isError = true;
                    _errinf.description = formError("Bad ACCESS", token.lexem);
                    return EmptyResult;
                    //print_error("Bad ACCESS", token, type);
                    //goto skip;
                }
                np->access = token.type;
                //type = get_token(fp, token, MAXTOKEN);
                parseToken(file, token);
            }

            if (token.type == LT::eCREATEREQ)
            {
                //type = get_token(fp, token, MAXTOKEN);
                parseToken(file, token);

                if (token.type != LT::eLEFTBRACKET)
                {
                    _errinf.isError = true;
                    _errinf.description = formError("Expected \"{\"", token.lexem);
                    return EmptyResult;
                    //print_error("Expected \"{\"", token, type);
                    //goto skip;
                }
                do
                {
                    parseToken(file, token);
                    //type = get_token(fp, token, MAXTOKEN);
                    if (token.type != LT::eLABEL)
                    {
                        _errinf.isError = true;
                        _errinf.description = formError("Bad object name in list", token.lexem);
                        return EmptyResult;
                        //print_error("Bad object name in list", token,
                        //	type);
                        //goto skip;
                    }
                    //type = get_token(fp, token, MAXTOKEN);
                    parseToken(file, token);

                } while (token.type == LT::eCOMMA);

                if (token.type != LT::eRIGHTBRACKET)
                {
                    _errinf.isError = true;
                    _errinf.description = formError("Expected \"}\" after list", token.lexem);
                    return EmptyResult;
                    //print_error("Expected \"}\" after list", token, type);
                    //goto skip;
                }
                //type = get_token(fp, token, MAXTOKEN);
                parseToken(file, token);
            }

            if (token.type == LT::eDEFVAL)
            {
                int             level = 1;
                //type = get_token(fp, token, MAXTOKEN);
                parseToken(file, token);

                if (token.type != LT::eLEFTBRACKET)
                {
                    _errinf.isError = true;
                    _errinf.description = formError("Expected \"{\" after DEFVAL", token.lexem);
                    return EmptyResult;
                    //print_error("Expected \"{\" after DEFVAL", token,
                    //	type);
                    //goto skip;
                }

                do
                {
                    //type = get_token(fp, token, MAXTOKEN);
                    parseToken(file, token);

                    if (token.type == LT::eLEFTBRACKET)
                        level++;
                    else if (token.type == LT::eRIGHTBRACKET)
                        level--;

                } while ((token.type != LT::eRIGHTBRACKET || level != 0) && token.type != LT::eENDOFFILE);

                if (token.type != LT::eRIGHTBRACKET)
                {
                    _errinf.isError = true;
                    _errinf.description = formError("Missing \"}\" after DEFVAL", token.lexem);
                    return EmptyResult;
                    //print_error("Missing \"}\" after DEFVAL", token, type);
                    //goto skip;
                }

                parseToken(file, token);
                //type = get_token(fp, token, MAXTOKEN);
            }

            if (token.type != LT::eDESCRIPTION)
            {
                _errinf.isError = true;
                _errinf.description = formError("Expected DESCRIPTION", token.lexem);
                return EmptyResult;
                //print_error("Expected DESCRIPTION", token, type);
                //goto skip;
            }
            //type = get_token(fp, quoted_string_buffer, MAXTOKEN);
            parseToken(file, token);

            if (token.type != LT::eQUOTESTRING)
            {
                _errinf.isError = true;
                _errinf.description = formError("Bad DESCRIPTION", token.lexem);
                return EmptyResult;
                //print_error("Bad DESCRIPTION", quoted_string_buffer, type);
                //goto skip;
            }
            //type = get_token(fp, token, MAXTOKEN);
            parseToken(file, token);
        }
    }

    if (token.type != LT::eEQUALS)
    {
        _errinf.isError = true;
        _errinf.description = formError("Expected \"::=\"", token.lexem);
        return EmptyResult;
    }
    //print_error("Expected \"::=\"", token, type);

    return mergeParsedObjectid(np, file, moduleName);
    //skip:
    //	while (type != EQUALS && type != ENDOFFILE)
    //	{
    //		type = get_token(fp, quoted_string_buffer, MAXQUOTESTR);
    //	}
    //return merge_parse_objectid(np, fp, name);
}

/*
* Parses an asn type.  Structures are ignored by this parser.
* Returns NULL on error.
*/
NodeList Parser::parseASN(std::ifstream& file, Token& token, const std::string& objName, const std::string& moduleName)
{
    //int             type, i;
    //char            token[MAXTOKEN];
    //char            quoted_string_buffer[MAXQUOTESTR];
    //char           *hint = NULL;
    //char           *descr = NULL;
    //struct tc      *tcp;

    //Node::Ptr np(new Node);
    //np->label = objName;
    TC tc;
    //Token token;
    int level = 0;

    parseToken(file, token);
    //type = get_token(fp, token, MAXTOKEN);
    if (token.type == LT::eSEQUENCE || token.type == LT::eCHOICE)
    {
        //level = 0;
        parseToken(file, token);
        while (token.type != LT::eENDOFFILE)
        {
            if (token.type == LT::eLEFTBRACKET)
            {
                level++;
            }
            else if (token.type == LT::eRIGHTBRACKET && --level == 0)
            {
                parseToken(file, token);
                return EmptyResult;
                //*ntype = get_token(fp, ntoken, MAXTOKEN);
                //return NULL;
            }
            parseToken(file, token);
        }

        _errinf.isError = true;
        _errinf.description = formError("Expected \"}\"", token.lexem);
        //return EmptyResult;
        //print_error("Expected \"}\"", token, type);
        //return NULL;
    }
    else if (token.type == LT::eLEFTBRACKET)
    {
        //struct node    *np;
        //int             ch_next = '{';

        file.unget();
        //ungetc(ch_next, fp);

        auto nodes = parseObjectid(file, objName, moduleName);
        //np = parse_objectid(fp, name);

        if (!nodes.empty())
        {
            parseToken(file, token);
            return nodes;
        }

        //return EmptyResult;

        //if (np != NULL)
        //{
        //	*ntype = get_token(fp, ntoken, MAXTOKEN);
        //	return np;
        //}
        //return NULL;
    }
    else if (token.type == LT::eLEFTSQBRACK)
    {
        bool size = false;

        do
        {
            parseToken(file, token);
            //type = get_token(fp, token, MAXTOKEN);
        } while (token.type != LT::eENDOFFILE && token.type != LT::eRIGHTSQBRACK);

        if (token.type != LT::eRIGHTSQBRACK)
        {
            _errinf.isError = true;
            _errinf.description = formError("Expected \"]\"", token.lexem);
            return EmptyResult;
            //print_error("Expected \"]\"", token, type);
            //return NULL;
        }
        //type = get_token(fp, token, MAXTOKEN);
        parseToken(file, token);

        if (token.type == LT::eIMPLICIT)
            parseToken(file, token);
        //type = get_token(fp, token, MAXTOKEN);

        LT lasttype = token.type;

        parseToken(file, token);
        //*ntype = get_token(fp, ntoken, MAXTOKEN);
        if (token.type == LT::eLEFTPAREN)
        {
            switch (lasttype)
            {
            case LT::eOCTETSTR:
                //*ntype = get_token(fp, ntoken, MAXTOKEN);
                parseToken(file, token);

                //if (*ntype != SIZE)
                if (token.type != LT::eSIZE)
                {
                    _errinf.isError = true;
                    _errinf.description = formError("Expected SIZE", token.lexem);
                    return EmptyResult;
                    //print_error("Expected SIZE", ntoken, *ntype);
                    //return NULL;
                }
                size = true;
                //*ntype = get_token(fp, ntoken, MAXTOKEN);
                parseToken(file, token);

                if (token.type != LT::eLEFTPAREN)
                    //if (*ntype != LEFTPAREN)
                {
                    _errinf.isError = true;
                    _errinf.description = formError("Expected \"(\"", token.lexem);
                    return EmptyResult;
                    //print_error("Expected \"(\" after SIZE", ntoken,
                    //	*ntype);
                    //return NULL;
                }
                /* FALL THROUGH */
            case LT::eINTEGER:
                //*ntype = get_token(fp, ntoken, MAXTOKEN);
                parseToken(file, token);

                do
                {
                    if (token.type != LT::eNUMBER)
                    {
                        _errinf.isError = true;
                        _errinf.description = formError("Expected NUMBER", token.lexem);
                        return EmptyResult;
                    }
                    //if (*ntype != NUMBER)
                        //print_error("Expected NUMBER", ntoken, *ntype);
                    //*ntype = get_token(fp, ntoken, MAXTOKEN);
                    parseToken(file, token);

                    //if (*ntype == RANGE)
                    if (token.type == LT::eRANGE)
                    {
                        //*ntype = get_token(fp, ntoken, MAXTOKEN);
                        parseToken(file, token);

                        if (token.type != LT::eNUMBER)
                        {
                            _errinf.isError = true;
                            _errinf.description = formError("Expected NUMBER", token.lexem);
                            return EmptyResult;
                        }

                        parseToken(file, token);
                        //if (*ntype != NUMBER)
                            //print_error("Expected NUMBER", ntoken, *ntype);
                        //*ntype = get_token(fp, ntoken, MAXTOKEN);
                    }
                } while (token.type == LT::eBAR);//(*ntype == BAR);

                //if (*ntype != RIGHTPAREN)
                if (token.type != LT::eRIGHTPAREN)
                {
                    _errinf.isError = true;
                    _errinf.description = formError("Expected \")\"", token.lexem);
                    return EmptyResult;
                    //print_error("Expected \")\"", ntoken, *ntype);
                    //return NULL;
                }
                //*ntype = get_token(fp, ntoken, MAXTOKEN);
                parseToken(file, token);

                if (size)
                {
                    //if (*ntype != RIGHTPAREN)
                    if (token.type != LT::eRIGHTPAREN)
                    {
                        _errinf.isError = true;
                        _errinf.description = formError("Expected \")\"", token.lexem);
                        return EmptyResult;
                        //print_error("Expected \")\" to terminate SIZE",
                        //	ntoken, *ntype);
                        //return NULL;
                    }
                    //*ntype = get_token(fp, ntoken, MAXTOKEN);
                    parseToken(file, token);
                }
            }
        }
        //return EmptyResult;
        //return NULL;
    }
    else
    {
        if (token.type == LT::eCONVENTION)
        {
            while (token.type != LT::eSYNTAX && token.type != LT::eENDOFFILE)
            {
                if (token.type == LT::eDISPLAYHINT)
                {
                    //type = get_token(fp, token, MAXTOKEN);
                    parseToken(file, token);

                    if (token.type != LT::eQUOTESTRING)
                    {
                        _errinf.isError = true;
                        _errinf.description = formError("DISPLAY-HINT must be string", token.lexem);
                        return EmptyResult;
                        //print_error("DISPLAY-HINT must be string", token,
                        //	type);
                    }
                    //else
                    //{
                    tc.hint = token.lexem;
                    //np->hint = token.lexem;
                    //free(hint);
                    //hint = token.lexem;//strdup(token);
                //}
                }
                else if (token.type == LT::eDESCRIPTION)
                {
                    //type = get_token(fp, quoted_string_buffer, MAXQUOTESTR);
                    parseToken(file, token);

                    if (token.type != LT::eQUOTESTRING)
                    {
                        _errinf.isError = true;
                        _errinf.description = formError("DESCRIPTION must be string", token.lexem);
                        return EmptyResult;
                        //print_error("DESCRIPTION must be string", token,
                        //	type);
                    }
                    //else
                    //{
                    tc.description = token.lexem;
                    //np->description = token.lexem;
                    //free(descr);
                    //descr = strdup(quoted_string_buffer);
                //}
                }
                else
                    parseToken(file, token);
                //type =
                //get_token(fp, quoted_string_buffer, MAXQUOTESTR);
            }
            //type = get_token(fp, token, MAXTOKEN);
            parseToken(file, token);

            if (token.type == LT::eOBJECT)
            {
                //type = get_token(fp, token, MAXTOKEN);
                parseToken(file, token);

                if (token.type != LT::eIDENTIFIER)
                {
                    _errinf.isError = true;
                    _errinf.description = formError("Expected IDENTIFIER", token.lexem);
                    return EmptyResult;
                    //print_error("Expected IDENTIFIER", token, type);
                    //goto err;
                }

                token.type = LT::eOBJID;
                //np->type = LT::eOBJID;
                //type = OBJID;
            }
        }
        else if (token.type == LT::eOBJECT)
        {
            //type = get_token(fp, token, MAXTOKEN);
            parseToken(file, token);

            if (token.type != LT::eIDENTIFIER)
            {
                _errinf.isError = true;
                _errinf.description = formError("Expected IDENTIFIER", token.lexem);
                return EmptyResult;
                //print_error("Expected IDENTIFIER", token, type);
                //goto err;
            }

            token.type = LT::eOBJID;
            //np->type = LT::eOBJID;
            //type = OBJID;
        }

        if (token.type == LT::eLABEL)
        {
            int ind = _tclist.getTCIndex(token.lexem);// , moduleName);

            if (ind != -1)
            {

                token.type = _tclist.getTC(ind).type;
            }

            //type = get_tc(token, current_module, NULL, NULL, NULL, NULL);
        }

        /*
        * textual convention
        */
        //for (i = 0; i < MAXTC; i++) 
        //{
        //	if (tclist[i].type == 0)
        //		break;
        //}

        /*if (i == MAXTC)
        {
            print_error("Too many textual conventions", token, type);
            goto err;
        }*/
        if (!(token.type & LT::eSYNTAX_MASK))
        {
            _errinf.isError = true;
            _errinf.description = formError("Textual convention doesn't map to real type", token.lexem);
            return EmptyResult;
            //print_error("Textual convention doesn't map to real type",
            //	token, type);
            //goto err;
        }

        tc.module = moduleName;
        tc.descriptor = objName;
        tc.type = token.type;

        //tcp = &tclist[i];
        //tcp->modid = current_module;
        //tcp->descriptor = strdup(name);
        //tcp->hint = hint;
        //tcp->description = descr;
        //tcp->type = type;
        //*ntype = get_token(fp, ntoken, MAXTOKEN);
        parseToken(file, token);

        //if (*ntype == LEFTPAREN)
        if (token.type == LT::eLEFTPAREN)
        {
            tc.ragnes = parseRanges(file);
            //tcp->ranges = parse_ranges(fp, &tcp->ranges);
            parseToken(file, token);
            //*ntype = get_token(fp, ntoken, MAXTOKEN);
        }
        //else if (*ntype == LEFTBRACKET)
        else if (token.type == LT::eLEFTBRACKET)
        {
            /*
            * if there is an enumeration list, parse it
            */
            tc.enums = parseEnums(file);
            //tcp->enums = parse_enumlist(fp, &tcp->enums);
            parseToken(file, token);
            //*ntype = get_token(fp, ntoken, MAXTOKEN);
        }

        _tclist.addTC(tc);
        //return NULL;
        //return EmptyResult;
    }

    return EmptyResult;

    //err:
    //	SNMP_FREE(descr);
    //	SNMP_FREE(hint);
    //	return NULL;
}

NodeList Parser::parseTrap(std::ifstream& file, const std::string& objName, const std::string& moduleName)
{
    //register int    type;
    //char            token[MAXTOKEN];
    //char            quoted_string_buffer[MAXQUOTESTR];
    //register struct node *np;
    Node::Ptr np(new Node);
    np->label = objName;
    np->modules.push_back(moduleName);

    Token token;

    //np = alloc_node(current_module);
    //if (np == NULL)
    //	return (NULL);
    //type = get_token(fp, token, MAXTOKEN);
    parseToken(file, token);

    while (token.type != LT::eEQUALS && token.type != LT::eENDOFFILE)
    {
        switch (token.type)
        {
        case LT::eDESCRIPTION:
            //type = get_token(fp, quoted_string_buffer, MAXQUOTESTR);
            parseToken(file, token);

            if (token.type != LT::eQUOTESTRING)
            {
                _errinf.isError = true;
                _errinf.description = formError("Bad DESCRIPTION", token.lexem);
                return EmptyResult;
                //print_error("Bad DESCRIPTION", quoted_string_buffer, type);
                //free_node(np);
                //return NULL;
            }
            //if (netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID,
            //	NETSNMP_DS_LIB_SAVE_MIB_DESCRS))
            //{
            np->description = token.lexem;//strdup(quoted_string_buffer);
            //}
            break;
        case LT::eREFERENCE:
            /* I'm not sure REFERENCEs are legal in smiv1 traps??? */
            //type = get_token(fp, quoted_string_buffer, MAXQUOTESTR);
            parseToken(file, token);

            if (token.type != LT::eQUOTESTRING)
            {
                _errinf.isError = true;
                _errinf.description = formError("Bad REFERENCE", token.lexem);
                return EmptyResult;
                //print_error("Bad REFERENCE", quoted_string_buffer, type);
                //free_node(np);
                //return NULL;
            }
            np->reference = token.lexem;//strdup(quoted_string_buffer);
            break;
        case LT::eENTERPRISE:
            //type = get_token(fp, token, MAXTOKEN);
            parseToken(file, token);

            if (token.type == LT::eLEFTBRACKET)
            {
                //type = get_token(fp, token, MAXTOKEN);
                parseToken(file, token);

                if (token.type != LT::eLABEL)
                {
                    _errinf.isError = true;
                    _errinf.description = formError("Bad trap format", token.lexem);
                    return EmptyResult;
                    //print_error("Bad Trap Format", token, type);
                    //free_node(np);
                    //return NULL;
                }
                np->parentName = token.lexem;//strdup(token);
                /*
                * Get right bracket
                */
                //type = get_token(fp, token, MAXTOKEN);
                parseToken(file, token);
            }
            else if (token.type == LT::eLABEL)
            {
                np->parentName = token.lexem;//strdup(token);
            }
            else
            {
                return EmptyResult;
                //free_node(np);
                //return NULL;
            }
            break;
        case LT::eVARIABLES:
            //np->varbinds = getVarbinds(fp, &np->varbinds);
            np->varbinds = parseVarbinds(file);
            //if (!np->varbinds)
            if (np->varbinds.empty())
            {
                _errinf.isError = true;
                _errinf.description = formError("Bad VARIABLES list", token.lexem);
                return EmptyResult;
                //print_error("Bad VARIABLES list", token, type);
                //free_node(np);
                //return NULL;
            }
            break;
        default:
            /*
            * NOTHING
            */
            break;
        }
        //type = get_token(fp, token, MAXTOKEN);
        parseToken(file, token);
    }
    //type = get_token(fp, token, MAXTOKEN);
    parseToken(file, token);
    //np->label = strdup(name);

    if (token.type != LT::eNUMBER)
    {
        _errinf.isError = true;
        _errinf.description = formError("Expected a NUMBER", token.lexem);
        return EmptyResult;
        //print_error("Expected a Number", token, type);
        //free_node(np);
        //return NULL;
    }
    np->subid = strtoul(token.lexem.c_str(), nullptr, 10);

    //np->next = alloc_node(current_module);

    //if (np->next == NULL)
    //{
    //	free_node(np);
    //	return (NULL);
    //}

    /* Catch the syntax error */
    if (np->parentName.empty())
    {
        return EmptyResult;
        //free_node(np->next);
        //free_node(np);
        //gMibError = MODULE_SYNTAX_ERROR;
        //return (NULL);
    }

    return NodeList({ np });
    //np->next->parent = np->parent;
    //np->parent = (char *)malloc(strlen(np->parent) + 2);
    //if (np->parent == NULL)
    //{
    //	free_node(np->next);
    //	free_node(np);
    //	return (NULL);
    //}
    //strcpy(np->parent, np->next->parent);
    //strcat(np->parent, "#");
    //np->next->label = strdup(np->parent);
    //return np;
}

/*
* Parses a MACRO definition
* Expect BEGIN, discard everything to end.
* Returns 0 on error.
*/
NodeList Parser::parseMacro(std::ifstream& file)
{
    //register int    type;
    //char            token[MAXTOKEN];
    //struct node    *np;
    //int             iLine = mibLine;

    Node::Ptr np(new Node);

    Token token;


    //np = alloc_node(current_module);
    //if (np == NULL)
    //	return (NULL);
    //type = get_token(fp, token, sizeof(token));
    parseToken(file, token);

    while (token.type != LT::eEQUALS && token.type != LT::eENDOFFILE)
    {
        parseToken(file, token);
        //type = get_token(fp, token, sizeof(token));
    }

    if (token.type != LT::eEQUALS)
    {
        return EmptyResult;
        //if (np)
        //	free_node(np);
        //return NULL;
    }

    while (token.type != LT::eBEGIN && token.type != LT::eENDOFFILE)
    {
        parseToken(file, token);
        //type = get_token(fp, token, sizeof(token));
    }

    if (token.type != LT::eBEGIN)
    {
        return EmptyResult;
        //if (np)
        //	free_node(np);
        //return NULL;
    }

    while (token.type != LT::eEND && token.type != LT::eENDOFFILE)
    {
        parseToken(file, token);
        //type = get_token(fp, token, sizeof(token));
    }

    if (token.type != LT::eEND)
    {
        return EmptyResult;
        //if (np)
        //	free_node(np);
        //return NULL;
    }

    //if (netsnmp_ds_get_int(NETSNMP_DS_LIBRARY_ID,
    //	NETSNMP_DS_LIB_MIB_WARNINGS))
    //{
    //	snmp_log(LOG_WARNING,
    //		"%s MACRO (lines %d..%d parsed and ignored).\n", name,
    //		iLine, mibLine);
    //}

    return NodeList({ np });
    //return np;
}


/*
Merge the parsed object identifier with the existing node.
If there is a problem with the identifier, release the existing node.
*/
NodeList Parser::mergeParsedObjectid(Node::Ptr& np, std::ifstream& file, const std::string& moduleName)
{
    //struct node    *nnp;
    /*
    * printf("merge defval --> %s\n",np->defaultValue);
    */

    auto res = parseObjectid(file, np->label, moduleName);

    if (res.empty())
    {
        np.reset();
        return res;
    }

    auto& last = res.back();
    np->parentName = last->parentName;
    np->subid = last->subid;

    return res;

    //Node::Ptr nnp = parse_objectid(fp, name);

    //if (nnp)
    //{

    //	/*
    //	* apply last OID sub-identifier data to the information
    //	*/
    //	/*
    //	* already collected for this node.
    //	*/
    //	struct node    *headp, *nextp;
    //	int             ncount = 0;
    //	nextp = headp = nnp;
    //	while (nnp->next)
    //	{
    //		nextp = nnp;
    //		ncount++;
    //		nnp = nnp->next;
    //	}

    //	np->label = nnp->label;
    //	np->subid = nnp->subid;
    //	np->modid = nnp->modid;
    //	np->parent = nnp->parent;
    //	if (nnp->filename != NULL)
    //	{
    //		free(nnp->filename);
    //	}
    //	free(nnp);

    //	if (ncount)
    //	{
    //		nextp->next = np;
    //		np = headp;
    //	}
    //}
    //else
    //{
    //	free_node(np);
    //	np = NULL;
    //}
}


/*
* Parse a sequence of object subidentifiers for the given name.
* The "label OBJECT IDENTIFIER ::=" portion has already been parsed.
*
* The majority of cases take this form :
* label OBJECT IDENTIFIER ::= { parent 2 }
* where a parent label and a child subidentifier number are specified.
*
* Variations on the theme include cases where a number appears with
* the parent, or intermediate subidentifiers are specified by label,
* by number, or both.
*
* Here are some representative samples :
* internet        OBJECT IDENTIFIER ::= { iso org(3) dod(6) 1 }
* mgmt            OBJECT IDENTIFIER ::= { internet 2 }
* rptrInfoHealth  OBJECT IDENTIFIER ::= { snmpDot3RptrMgt 0 4 }
*
* Here is a very rare form :
* iso             OBJECT IDENTIFIER ::= { 1 }
*
* Returns NULL on error.  When this happens, memory may be leaked.
*/
NodeList Parser::parseObjectid(std::ifstream& file, const std::string& objName, const std::string& moduleName)
{
    //register int    count;
    //register struct subid_ss *op, *nop;
    //int             length;
    //struct subid_ss  loid[32];
    //struct node    *np, *root = NULL, *oldnp = NULL;
    //struct tree    *tp;
    Node::Ptr np = nullptr;
    auto oids = parseOIDlist(file);
    NodeList res;

    // —четчик анонимных узлов
    // —лучай: rptrInfoHealth  OBJECT IDENTIFIER ::= { snmpDot3RptrMgt 0 4 }
    static unsigned int anonymous = 0;


    if (oids.empty())
    {
        _errinf.isError = true;
        _errinf.description = formError("Bad object identifier", "");
        return res;
    }

    //if ((length = getoid(fp, loid, 32)) == 0)
    //{
    //	print_error("Bad object identifier", NULL, CONTINUE);
    //	return NULL;
    //}

    /*
    * Handle numeric-only object identifiers,
    *  by labelling the first sub-identifier
    */

    if (oids[0].label.empty())
    {
        if (oids.size() == 1)
        {
            _errinf.isError = true;
            _errinf.description = formError("Attempt to define a root oid", "");
            return res;
        }

        auto& oid = oids[0];

        //the module for the MIB roots has an empty name
        auto mibRootsModule = _tree->findModule("");
        auto const& roots = mibRootsModule->nodes;

        for (auto const& root : roots)
        {
            if (root.second->subid == oid.subid)
            {
                oid.label = root.first;
                break;
            }
        }
    }

    //op = loid;
    //if (!op->label)
    //{
    //	if (length == 1)
    //	{
    //		print_error("Attempt to define a root oid", name, OBJECT);
    //		return NULL;
    //	}
    //	for (tp = tree_head; tp; tp = tp->next_peer)
    //		if ((int)tp->subid == op->subid)
    //		{
    //			op->label = strdup(tp->label);
    //			break;
    //		}
    //}

    /*
    * Handle  "label OBJECT-IDENTIFIER ::= { subid }"
    */

    if (oids.size() == 1)
    {
        np = Node::Ptr(new Node);
        np->subid = oids[0].subid;
        np->label = oids[0].label;
        np->modules.push_back(moduleName);
        res.push_back(np);
        return res;
    }

    /*if (length == 1)
    {
        op = loid;
        np = alloc_node(op->modid);
        if (np == NULL)
            return (NULL);
        np->subid = op->subid;
        np->label = strdup(name);
        np->parent = op->label;
        return np;
    }*/

    /*
    * For each parent-child subid pair in the subid array,
    * create a node and link it into the node list.
    */

    size_t size = oids.size();
    for (size_t i = 0; i < size - 1; ++i)
    {
        auto& noid = oids[i + 1];
        np = Node::Ptr(new Node);


        np->parentName = oids[i].label;
        np->modules.push_back(moduleName);

        if (i == size - 2)
        {
            np->label = objName;
        }
        else
        {
            if (noid.label.empty())
            {
                noid.label = "anonymous#" + std::to_string(anonymous++);
            }
            np->label = noid.label;
        }

        if (noid.subid != -1)
            np->subid = noid.subid;
        else
        {
            res.clear();
            break;
            //return res;
        }

        res.push_back(np);
    }

    return res;

    //	for (count = 0, op = loid, nop = loid + 1; count < (length - 1);
    //		count++, op++, nop++)
    //	{
    //		/*
    //		* every node must have parent's name and child's name or number
    //		*/
    //		/*
    //		* XX the next statement is always true -- does it matter ??
    //		*/
    //		if (op->label && (nop->label || (nop->subid != -1)))
    //		{
    //			np = alloc_node(nop->modid);
    //			if (np == NULL)
    //				goto err;
    //			if (root == NULL)
    //				root = np;
    //
    //			np->parent = strdup(op->label);
    //			if (count == (length - 2))
    //			{
    //				/*
    //				* The name for this node is the label for this entry
    //				*/
    //				np->label = strdup(name);
    //				if (np->label == NULL)
    //					goto err;
    //			}
    //			else
    //			{
    //				if (!nop->label)
    //				{
    //					nop->label = (char *)malloc(20 + ANON_LEN);
    //					if (nop->label == NULL)
    //						goto err;
    //					sprintf(nop->label, "%s%d", ANON, anonymous++);
    //				}
    //				np->label = strdup(nop->label);
    //			}
    //			if (nop->subid != -1)
    //				np->subid = nop->subid;
    //			else
    //				print_error("Warning: This entry is pretty silly",
    //					np->label, CONTINUE);
    //
    //			/*
    //			* set up next entry
    //			*/
    //			if (oldnp)
    //				oldnp->next = np;
    //			oldnp = np;
    //		}                       /* end if(op->label... */
    //	}
    //
    //out:
    //	/*
    //	* free the loid array
    //	*/
    //	for (count = 0, op = loid; count < length; count++, op++)
    //	{
    //		if (op->label)
    //			free(op->label);
    //	}
    //
    //	return root;
    //
    //err:
    //	for (; root; root = np)
    //	{
    //		np = root->next;
    //		free_node(root);
    //	}
    //	goto out;
}


/*
* Takes a list of the form:
* { iso org(3) dod(6) 1 }
* and creates several nodes, one for each parent-child pair.
* Returns 0 on error.
*/
std::vector<SubID> Parser::parseOIDlist(std::ifstream& file)
{
    //register int    count;
    //int             type;
    //char            token[MAXTOKEN];
    std::vector<SubID> res;
    SubID sub;
    Token token;

    parseToken(file, token);

    if (token.type != LT::eLEFTBRACKET)
    {
        _errinf.isError = true;
        _errinf.description = formError("Expected \"{\"", token.lexem);
        //print_error("Expected \"{\"", token, type);
        return res;
    }

    //if ((type = get_token(fp, token, MAXTOKEN)) != LEFTBRACKET)
    //{
    //	print_error("Expected \"{\"", token, type);
    //	return 0;
    //}
    parseToken(file, token);
    //type = get_token(fp, token, MAXTOKEN);
    for (int count = 0; count < 32; count++)//, id++) //hz
    {
        sub.label.clear();
        sub.subid = -1;
        //id->label = NULL;
        //id->modid = current_module;
        //id->subid = -1;
        if (token.type == LT::eRIGHTBRACKET)
            return res;
        //return count;
        if (token.type == LT::eLABEL)
        {
            /*
            * this entry has a label
            */
            sub.label = token.lexem;
            parseToken(file, token);
            //id->label = strdup(token);
            //type = get_token(fp, token, MAXTOKEN);
            if (token.type == LT::eLEFTPAREN)
            {
                parseToken(file, token);
                //type = get_token(fp, token, MAXTOKEN);
                if (token.type == LT::eNUMBER)
                {
                    sub.subid = strtoul(token.lexem.c_str(), nullptr, 10);
                    //id->subid = strtoul(token, NULL, 10);
                    parseToken(file, token);

                    if (token.type != LT::eRIGHTPAREN)
                    {
                        _errinf.isError = true;
                        _errinf.description = formError("Expected \")\"", token.lexem);
                        res.clear();
                        return res;
                    }

                    parseToken(file, token);
                    //if ((type =
                    //	get_token(fp, token, MAXTOKEN)) != RIGHTPAREN)
                    //{
                    //	print_error("Expected a closing parenthesis",
                    //		token, type);
                    //	return 0;
                    //}
                }
                else
                {
                    _errinf.isError = true;
                    _errinf.description = formError("Expected a number", token.lexem);
                    res.clear();
                    return res;
                    //print_error("Expected a number", token, type);
                    //return 0;
                }
            }
            //else
            //{
            //	continue;
            //}
        }
        else if (token.type == LT::eNUMBER)
        {
            /*
            * this entry  has just an integer sub-identifier
            */
            sub.subid = strtoul(token.lexem.c_str(), nullptr, 10);
            parseToken(file, token);
            //id->subid = strtoul(token, NULL, 10);
        }
        else
        {
            _errinf.isError = true;
            _errinf.description = formError("Expected label or number", token.lexem);
            res.clear();
            return res;
            //print_error("Expected label or number", token, type);
            //return 0;
        }

        res.push_back(sub);
        //parseToken(file, token);
        //type = get_token(fp, token, MAXTOKEN);
    }
    //print_error("Too long OID", token, type);
    //return 0;

    res.clear();
    return res;
}

EnumList Parser::parseEnums(std::ifstream& file)
{

    Token token;
    EnumList res;

    parseToken(file, token);

    while (token.type != LT::eENDOFFILE)
    {
        if (token.type == LT::eRIGHTBRACKET)
            break;
        /* some enums use "deprecated" to indicate a no longer value label */
        /* (EG: IP-MIB's IpAddressStatusTC) */
        if (token.type == LT::eLABEL || token.type == LT::eDEPRECATED)
        {
            /*
            * this is an enumerated label
            */
            //*epp =
            //	(struct enum_list *) calloc(1, sizeof(struct enum_list));
            //if (*epp == NULL)
            //	return (NULL);
            /*
            * a reasonable approximation for the length
            */

            Enum enm;

            enm.label = token.lexem;

            //(*epp)->label = strdup(token);
            //type = get_token(fp, token, MAXTOKEN);
            parseToken(file, token);

            if (token.type != LT::eLEFTPAREN)
            {
                _errinf.isError = true;
                _errinf.description = formError("Expected \"(\"", token.lexem);
                return res;
                //print_error("Expected \"(\"", token, type);
                //return NULL;
            }

            parseToken(file, token);
            //type = get_token(fp, token, MAXTOKEN);
            if (token.type != LT::eNUMBER)
            {
                _errinf.isError = true;
                _errinf.description = formError("Expected integer", token.lexem);
                return res;
                //print_error("Expected integer", token, type);
                //return NULL;
            }
            enm.value = strtol(token.lexem.c_str(), nullptr, 10);

            //(*epp)->value = strtol(token, NULL, 10);
            //type = get_token(fp, token, MAXTOKEN);
            parseToken(file, token);

            if (token.type != LT::eRIGHTPAREN)
            {
                _errinf.isError = true;
                _errinf.description = formError("Expected \")\"", token.lexem);
                return res;

                //print_error("Expected \")\"", token, type);
                //return NULL;
            }
            //epp = &(*epp)->next;
            res.push_back(enm);
        }

        parseToken(file, token);
    }

    if (token.type == LT::eENDOFFILE)
    {
        _errinf.isError = true;
        _errinf.description = formError("Expected \"}\"", token.lexem);
        return res;
        //print_error("Expected \"}\"", token, type);
        //return NULL;
    }

    return res;
    //*retp = ep;
    //return ep;
}

RangeList Parser::parseRanges(std::ifstream& file)
{

    Token token;
    Range rng;
    RangeList res;


    //int low = 0, high = 0;
    //char            nexttoken[MAXTOKEN];
    //int             nexttype;
    //struct range_list* rp = NULL, ** rpp = &rp;
    //int             size = 0, taken = 1;
    bool size = false, taken = true;

    //free_ranges(retp);

    //nexttype = get_token(fp, nexttoken, MAXTOKEN);
    parseToken(file, token);

    //if (nexttype == SIZE)
    if (token.type == LT::eSIZE)
    {
        size = true;
        taken = false;

        parseToken(file, token);

        if (token.type != LT::eLEFTPAREN)
        {
            _errinf.isError = true;
            _errinf.description = formError("Expected \"(\" after SIZE", token.lexem);
            return res;
        }

        //nexttype = get_token(fp, nexttoken, MAXTOKEN);
        //if (nexttype != LEFTPAREN)
        //{
        //	print_error("Expected \"(\" after SIZE", nexttoken, nexttype);
        //	return NULL;
        //}
    }

    do
    {
        if (!taken)
            parseToken(file, token);
        //nexttype = get_token(fp, nexttoken, MAXTOKEN);
        else
            taken = false;



        rng.high = rng.low = strtoul(token.lexem.c_str(), nullptr, 10);

        parseToken(file, token);

        //nexttype = get_token(fp, nexttoken, MAXTOKEN);
        //if (nexttype == RANGE)
        if (token.type == LT::eRANGE)
        {
            parseToken(file, token);
            //nexttype = get_token(fp, nexttoken, MAXTOKEN);
            //errno = 0;
            rng.high = strtoul(token.lexem.c_str(), nullptr, 10);
            //if (errno == ERANGE)
            //{
                //if (netsnmp_ds_get_int(NETSNMP_DS_LIBRARY_ID,
                //	NETSNMP_DS_LIB_MIB_WARNINGS))
                //	snmp_log(LOG_WARNING,
                //		"Warning: Upper bound not handled correctly (%s != %d): At line %d in %s\n",
                //		nexttoken, high, mibLine, File);
            //}
            //nexttype = get_token(fp, nexttoken, MAXTOKEN);
            parseToken(file, token);
        }

        res.push_back(rng);
        //*rpp = (struct range_list*)calloc(1, sizeof(struct range_list));
        //if (*rpp == NULL)
        //	break;
        //(*rpp)->low = low;
        //(*rpp)->high = high;
        //rpp = &(*rpp)->next;

    } while (token.type == LT::eBAR);//(nexttype == BAR);

    if (size)
    {
        if (token.type != LT::eRIGHTPAREN)
        {
            _errinf.isError = true;
            _errinf.description = formError("Expected \")\" after SIZE", token.lexem);
            res.clear();
            return res;
        }

        //if (nexttype != RIGHTPAREN)
        //	print_error("Expected \")\" after SIZE", nexttoken, nexttype);
        //nexttype = get_token(fp, nexttoken, nexttype);
        parseToken(file, token);
    }

    if (token.type != LT::eRIGHTPAREN)
    {
        _errinf.isError = true;
        _errinf.description = formError("Expected \")\"", token.lexem);
        res.clear();
    }

    //if (nexttype != RIGHTPAREN)
    //	print_error("Expected \")\"", nexttoken, nexttype);
    //
    //*retp = rp;
    //return rp;


    return res;
}

IndexList Parser::parseIndexes(std::ifstream& file)
{
    Token token;
    Index indx;
    IndexList res;

    //int             type;
    //char            token[MAXTOKEN];
    //char            nextIsImplied = 0;
    bool nextIsImplied = false;

    //struct index_list* mylist = NULL;
    //struct index_list** mypp = &mylist;

    //free_indexes(retp);

    //type = get_token(fp, token, MAXTOKEN);
    parseToken(file, token);

    if (token.type != LT::eLEFTBRACKET)
    {
        return res;
    }

    parseToken(file, token);
    //type = get_token(fp, token, MAXTOKEN);
    while (token.type != LT::eRIGHTBRACKET && token.type != LT::eENDOFFILE)
    {
        if ((token.type == LT::eLABEL) || (token.type & LT::eSYNTAX_MASK))
        {
            indx.ilabel = token.lexem;
            indx.isimplied = nextIsImplied;

            res.push_back(indx);

            nextIsImplied = false;
            //*mypp =
            //	(struct index_list*)calloc(1, sizeof(struct index_list));
            //if (*mypp)
            //{
            //	(*mypp)->ilabel = _strdup(token);
            //	(*mypp)->isimplied = nextIsImplied;
            //	mypp = &(*mypp)->next;
            //	nextIsImplied = 0;
            //}
        }
        else if (token.type == LT::eIMPLIED)
        {
            nextIsImplied = true;
        }
        //type = get_token(fp, token, MAXTOKEN);
        parseToken(file, token);
    }

    //*retp = mylist;
    //return mylist;


    return res;
}

VarbindList Parser::parseVarbinds(std::ifstream& file)
{
    //int             type;
    //char            token[MAXTOKEN];

    //struct varbind_list* mylist = NULL;
    //struct varbind_list** mypp = &mylist;

    //free_varbinds(retp);

    Token token;
    VarbindList res;

    parseToken(file, token);
    //type = get_token(fp, token, MAXTOKEN);

    if (token.type != LT::eLEFTBRACKET)
    {
        return res;
        //return NULL;
    }

    parseToken(file, token);
    //type = get_token(fp, token, MAXTOKEN);
    while (token.type != LT::eRIGHTBRACKET && token.type != LT::eENDOFFILE)
    {
        if (token.type == LT::eLABEL || token.type & LT::eSYNTAX_MASK)
        {
            res.push_back(token.lexem);
            //*mypp =
            //	(struct varbind_list*)calloc(1,
            //		sizeof(struct
            //			varbind_list));
            //if (*mypp)
            //{
            //	(*mypp)->vblabel = _strdup(token);
            //	mypp = &(*mypp)->next;
            //}
        }

        parseToken(file, token);
        //type = get_token(fp, token, MAXTOKEN);
    }

    //*retp = mylist;
    //return mylist;
    return res;
}

bool Parser::checkUTC(const std::string& utc)
{
    int len, year, month, day, hour, minute;

    len = utc.size();//strlen(utc);

    if (utc.back() != 'Z' && utc.back() != 'z')
    {
        _errinf.isError = true;
        _errinf.description = formError("Timestamp should end with Z", utc);
        return false;
        //print_error("Timestamp should end with Z", utc, QUOTESTRING);
        //return;
    }

    if (len == 11)
    {
        len = sscanf(utc.c_str(), "%2d%2d%2d%2d%2dZ",
            &year, &month, &day, &hour, &minute);

        year += 1900;
    }
    else if (len == 13)
        len = sscanf(utc.c_str(), "%4d%2d%2d%2d%2dZ",
            &year, &month, &day, &hour, &minute);
    else
    {
        _errinf.isError = true;
        _errinf.description = formError("Bad timestamp format (11 or 13 characters)", utc);
        //print_error("Bad timestamp format (11 or 13 characters)",
        //	utc, QUOTESTRING);
        return false;
    }

    if (len != 5)
    {
        _errinf.isError = true;
        _errinf.description = formError("Bad timestamp format", utc);
        return false;
        //print_error("Bad timestamp format", utc, QUOTESTRING);
        //return;
    }

    if (month < 1 || month > 12)
    {
        _errinf.isError = true;
        _errinf.description = formError("Bad month in timestamp", utc);
        return false;
    }
    //print_error("Bad month in timestamp", utc, QUOTESTRING);
    if (day < 1 || day > 31)
    {
        _errinf.isError = true;
        _errinf.description = formError("Bad day in timestamp", utc);
        return false;
    }
    //print_error("Bad day in timestamp", utc, QUOTESTRING);
    if (hour < 0 || hour > 23)
    {
        _errinf.isError = true;
        _errinf.description = formError("Bad hour in timestamp", utc);
        return false;
    }
    //print_error("Bad hour in timestamp", utc, QUOTESTRING);
    if (minute < 0 || minute > 59)
    {
        _errinf.isError = true;
        _errinf.description = formError("Bad minute in timestamp", utc);
        return false;
    }
    //print_error("Bad minute in timestamp", utc, QUOTESTRING);
    return true;
}

void Parser::tossObjectIdentifier(std::ifstream& file, Token& token)
{
    //int             type;
    //char            token[MAXTOKEN];
    int             bracketCount = 1;

    token.type = LT::eOTHER;

    parseToken(file, token);
    //type = get_token(fp, token, MAXTOKEN);

    if (token.type != LT::eLEFTBRACKET)
        return;

    while ((token.type != LT::eRIGHTBRACKET || bracketCount > 0) && token.type != LT::eENDOFFILE)
    {
        parseToken(file, token);
        //type = get_token(fp, token, MAXTOKEN);
        if (token.type == LT::eLEFTBRACKET)
            bracketCount++;
        else if (token.type == LT::eRIGHTBRACKET)
            bracketCount--;
    }

    if (token.type == LT::eRIGHTBRACKET)
        token.type = LT::eOBJID;
    //else
    //	return 0;
}

void Parser::eatSyntax(std::ifstream& file, Token& token)
{
    //int             type, nexttype;
    //struct node* np = alloc_node(current_module);
    //char            nexttoken[MAXTOKEN];

    //if (!np)
    //	return 0;

    //type = get_token(fp, token, maxtoken);
    //nexttype = get_token(fp, nexttoken, MAXTOKEN);

    parseToken(file, token);
    LT lasttype = token.type;
    parseToken(file, token);

    switch (lasttype)
    {
    case LT::eINTEGER:
    case LT::eINTEGER32:
    case LT::eUINTEGER32:
    case LT::eUNSIGNED32:
    case LT::eCOUNTER:
    case LT::eGAUGE:
    case LT::eBITSTRING:
    case LT::eLABEL:

        //if (nexttype == LEFTBRACKET)
        if (token.type == LT::eLEFTBRACKET)
        {
            /*
            * if there is an enumeration list, parse it
            */
            parseEnums(file);
            //np->enums = parse_enumlist(fp, &np->enums);
            //nexttype = get_token(fp, nexttoken, MAXTOKEN);

            parseToken(file, token);
        }
        else if (token.type == LT::eLEFTPAREN)
        {
            /*
            * if there is a range list, parse it
            */
            parseRanges(file);
            //np->ranges = parse_ranges(fp, &np->ranges);
            //nexttype = get_token(fp, nexttoken, MAXTOKEN);
            parseToken(file, token);
        }
        break;
    case LT::eOCTETSTR:
    case LT::eOPAQUE:
        /*
        * parse any SIZE specification
        */

        if (token.type == LT::eLEFTPAREN)
        {
            parseToken(file, token);
            //nexttype = get_token(fp, nexttoken, MAXTOKEN);
            if (token.type == LT::eSIZE)
            {
                parseToken(file, token);
                //nexttype = get_token(fp, nexttoken, MAXTOKEN);
                if (token.type == LT::eLEFTPAREN)
                {
                    parseRanges(file);
                    //np->ranges = parse_ranges(fp, &np->ranges);
                    //nexttype = get_token(fp, nexttoken, MAXTOKEN);      /* ) */
                    parseToken(file, token);

                    if (token.type == LT::eRIGHTPAREN)
                    {
                        parseToken(file, token);
                        //nexttype = get_token(fp, nexttoken, MAXTOKEN);
                        break;
                    }
                }
            }

            _errinf.isError = true;
            _errinf.description = formError("Bad SIZE syntax", token.lexem);
            return;
            //print_error("Bad SIZE syntax", token, type);
            //free_node(np);
            //return nexttype;
        }
        break;
    case LT::eOBJID:
    case LT::eNETADDR:
    case LT::eIPADDR:
    case LT::eTIMETICKS:
    case LT::eNULL:
    case LT::eNSAPADDRESS:
    case LT::eCOUNTER64:
        break;
    default:

        _errinf.isError = true;
        _errinf.description = formError("Bad syntax", token.lexem);
        //parseToken(file, token);

        //print_error("Bad syntax", token, type);
        //free_node(np);
        //return nexttype;
    }
    //free_node(np);
    //return nexttype;
}

bool Parser::complianceLookup(const std::string& name, const std::string& moduleName)
{

    if (moduleName.empty())
    {
        _objgroups.insert(name);
        return true;
    }

    return _tree->findNode(name, moduleName) != nullptr;


    //if (modid == -1)
    //{
    //	struct objgroup* op =
    //		(struct objgroup*)malloc(sizeof(struct objgroup));
    //	if (!op)
    //		return 0;
    //	op->next = objgroups;
    //	op->name = strdup(name);
    //	op->line = mibLine;
    //	objgroups = op;
    //	return 1;
    //}
    //else
    //	return find_tree_node(name, modid) != NULL;
}

Parser::LoadStatus Parser::readModuleInternal(const std::string& moduleName)
{
    //struct module* mp;
    //FILE* fp;
    //struct node* np;

    auto mp = _tree->findModule(moduleName);

    if (mp && mp->moduleName == moduleName)
    {

        if (mp->isParsed)
            return LoadStatus::MODULE_ALREADY_LOADED;

        std::ifstream file(mp->fileName);

        if (!file.is_open())
            return LoadStatus::MODULE_LOAD_FAILED;

        size_t oldLine = _line;
        std::string oldModule = _moduleName;

        _line = 1;
        _moduleName = moduleName;

        auto res = parse(file);

        _line = oldLine;
        _moduleName = std::move(oldModule);

        file.close();

        if (_errinf.isError)
            return LoadStatus::MODULE_SYNTAX_ERROR;

        return LoadStatus::MODULE_LOADED_OK;

    }

    return LoadStatus::MODULE_NOT_FOUND;

    //	for (mp = module_head; mp; mp = mp->next)
    //		if (!label_compare(mp->name, name))
    //		{
    //			const char* oldFile = File;
    //			int             oldLine = mibLine;
    //			int             oldModule = current_module;
    //
    //			if (mp->no_imports != -1)
    //			{
    //				DEBUGMSGTL(("parse-mibs", "Module %s already loaded\n",
    //					name));
    //				return MODULE_ALREADY_LOADED;
    //			}
    //			if ((fp = fopen(mp->file, "r")) == NULL)
    //			{
    //				int rval;
    //				if (errno == ENOTDIR || errno == ENOENT)
    //					rval = MODULE_NOT_FOUND;
    //				else
    //					rval = MODULE_LOAD_FAILED;
    //				snmp_log_perror(mp->file);
    //				return rval;
    //			}
    //#ifdef HAVE_FLOCKFILE
    //			flockfile(fp);
    //#endif
    //			mp->no_imports = 0; /* Note that we've read the file */
    //			File = mp->file;
    //			mibLine = 1;
    //			current_module = mp->modid;
    //			/*
    //			 * Parse the file
    //			 */
    //			np = parse(fp, NULL);
    //#ifdef HAVE_FUNLOCKFILE
    //			funlockfile(fp);
    //#endif
    //			fclose(fp);
    //			File = oldFile;
    //			mibLine = oldLine;
    //			current_module = oldModule;
    //			if ((np == NULL) && (gMibError == MODULE_SYNTAX_ERROR))
    //				return MODULE_SYNTAX_ERROR;
    //			return MODULE_LOADED_OK;
    //		}
    //
    //	return MODULE_NOT_FOUND;

}

Parser::LoadStatus Parser::readImportReplacements(const std::string& oldModuleName, Module::Imports& imports)
{
    //struct module_compatability* mcp;

    /*
     * Look for matches first
     */

    auto& import = imports.back();
    size_t sizeLabels = import.labels.size();
    size_t sizeMC = _modCompats.size();

    for (size_t i = 0; i < sizeLabels; ++i)
    {
        auto const& label = import.labels[i];

        for (size_t j = 0; j < sizeMC; ++j)
        {
            auto const& mc = _modCompats[j];

            if (mc.oldModule == oldModuleName)
            {
                if (mc.tag.empty() || (!mc.prefix && label == mc.tag) || (mc.prefix && label.find(mc.tag) != std::string::npos))
                {
                    auto status = readModuleInternal(mc.newModule);

                    if (status != LoadStatus::MODULE_NOT_FOUND)
                    {
                        size_t sizeImports = imports.size();
                        for (size_t k = 0; k < sizeImports; ++k)
                            if (imports[k].module == mc.newModule)
                            {
                                imports[k].labels.push_back(label);
                                import.labels.erase(import.labels.begin() + i);
                                return status;
                            }

                        imports.emplace_back(mc.newModule, Strs{ label });
                        import.labels.erase(import.labels.begin() + i);
                    }

                    return status;
                }
            }
        }
    }

    return readModuleReplacements(oldModuleName);

    //for (mcp = module_map_head; mcp; mcp = mcp->next)
    //{
    //	if (!label_compare(mcp->old_module, old_module_name))
    //	{
    //
    //		if (                /* exact match */
    //			(mcp->tag_len == 0 && (mcp->tag == NULL || !label_compare(mcp->tag, identifier->label))) ||
    //			/*
    //			 * prefix match
    //			 */
    //			(mcp->tag_len != 0 && !strncmp(mcp->tag, identifier->label, mcp->tag_len))
    //			)
    //		{
    //
    //			if (netsnmp_ds_get_int(NETSNMP_DS_LIBRARY_ID,
    //				NETSNMP_DS_LIB_MIB_WARNINGS))
    //			{
    //				snmp_log(LOG_WARNING,
    //					"Importing %s from replacement module %s instead of %s (%s)\n",
    //					identifier->label, mcp->new_module,
    //					old_module_name, File);
    //			}
    //			(void)netsnmp_read_module(mcp->new_module);
    //			identifier->modid = which_module(mcp->new_module);
    //			return 1;         /* finished! */
    //		}
    //	}
    //}
    //
    ///*
    // * If no exact match, load everything relevant
    // */
    //return read_module_replacements(old_module_name);
}

Parser::LoadStatus Parser::readModuleReplacements(const std::string& oldModuleName)
{

    size_t size = _modCompats.size();

    for (size_t i = 0; i < size; ++i)
    {
        auto const& mc = _modCompats[i];
        if (mc.oldModule == oldModuleName)
        {
            return readModuleInternal(mc.newModule);
        }
    }

    return LoadStatus::MODULE_NOT_FOUND;

    //struct module_compatability* mcp;

    //for (mcp = module_map_head; mcp; mcp = mcp->next)
    //{
    //	if (!label_compare(mcp->old_module, name))
    //	{
    //		if (netsnmp_ds_get_int(NETSNMP_DS_LIBRARY_ID,
    //			NETSNMP_DS_LIB_MIB_WARNINGS))
    //		{
    //			snmp_log(LOG_WARNING,
    //				"Loading replacement module %s for %s (%s)\n",
    //				mcp->new_module, name, File);
    //		}
    //		(void)netsnmp_read_module(mcp->new_module);
    //		return 1;
    //	}
    //}
    //return 0;
}

void Parser::scanObjlist(const NodeList& root, const Module::Ptr& mp, Objgroup& objlist)
{

    for (auto it = objlist.begin(); it != objlist.end(); )
    {
        bool found = false;

        for (auto const& node : root)
            if (node->label == *it)
            {
                it = objlist.erase(it);
                found = true;
                break;
            }

        if (!found)
        {
            auto const& imports = mp->imports;
            size_t sizeImp = imports.size();

            for (size_t i = 0; i < sizeImp; ++i)
            {
                auto const& import = imports[i];
                size_t sizeLab = import.labels.size();

                for (size_t j = 0; j < sizeLab; ++j)
                    if (import.labels[j] == *it)
                    {
                        it = objlist.erase(it);
                        found = true;
                        break;
                    }

                if (found)
                    break;
            }
        }

        if (!found)
            it++;
    }


    if (!objlist.empty())
    {
        _errinf.isError = true;
        _errinf.description = "Unidentified objects in " + _moduleName + ":";

        size_t size = objlist.size();

        for (auto const& obj : objlist)
            _errinf.description += " " + obj + ",";

        //for (size_t i = 0; i < size; ++i)
        //	_errinf.description += " " + objlist[i] + ",";

        _errinf.description.pop_back();
    }

    //int             oLine = mibLine;

    //while (list)
    //{
    //	struct objgroup* gp = list;
    //	struct node* np;
    //	list = list->next;
    //	np = root;
    //	while (np)
    //		if (label_compare(np->label, gp->name))
    //			np = np->next;
    //		else
    //			break;
    //	if (!np)
    //	{
    //		int i;
    //		struct module_import* mip;
    //		/* if not local, check if it was IMPORTed */
    //		for (i = 0, mip = mp->imports; i < mp->no_imports; i++, mip++)
    //			if (strcmp(mip->label, gp->name) == 0)
    //				break;
    //		if (i == mp->no_imports)
    //		{
    //			mibLine = gp->line;
    //			print_error(error, gp->name, QUOTESTRING);
    //		}
    //	}
    //	free(gp->name);
    //	free(gp);
    //}
    //mibLine = oLine;
}

std::string Parser::formError(const std::string& str, const std::string& lexem)
{
    std::cout << "Loading " + _moduleName + ": ERROR" << std::endl;
    return str + " (" + lexem + "): At line " + std::to_string(_line) + " in " + _moduleName;
}

//Error Parser::scan_objlist(const NodeList& list, std::vector<std::string>& objgroup)
//{
//
//
//	while (list)
//	{
//		struct objgroup *gp = list;
//		struct node    *np;
//		list = list->next;
//		np = root;
//		while (np)
//			if (label_compare(np->label, gp->name))
//				np = np->next;
//			else
//				break;
//		if (!np)
//		{
//			int i;
//			struct module_import *mip;
//			/* if not local, check if it was IMPORTed */
//			for (i = 0, mip = mp->imports; i < mp->no_imports; i++, mip++)
//				if (strcmp(mip->label, gp->name) == 0)
//					break;
//			if (i == mp->no_imports)
//			{
//				mibLine = gp->line;
//				print_error(error, gp->name, QUOTESTRING);
//			}
//		}
//		free(gp->name);
//		free(gp);
//	}
//	mibLine = oLine;
//}


void Parser::parseToken(std::ifstream& file, Token& token)
{
    char ch;// , ch_next;
    //register char  *cp = token;
    //register int    hash = 0;
    //register struct tok *tp;
    //int             too_long = 0;

    enum { bdigits, xdigits, other } seenSymbols;

    bool more = true;

    while (more)
    {
        token.lexem.clear();
        more = false;

        /*
        * skip all white space
        */
        do
        {
            ch = file.get();
            //ch = netsnmp_getc(fp);
            if (ch == '\n')
                ++_line;
            //	mibLine++;
        } while (isspace(ch) && ch != EOF);

        token.lexem += ch;
        //*cp++ = ch;
        //*cp = '\0';
        switch (ch)
        {
        case EOF:
            token.type = LT::eENDOFFILE;
            return;
            //return ENDOFFILE;
        case '"':
            parseQuoteString(file, token);
            return;
        case '\'':                 /* binary or hex constant */
            seenSymbols = bdigits;
            while ((ch = file.get()) != EOF && ch != '\'')
            {
                switch (seenSymbols)
                {
                case bdigits:
                    if (ch == '0' || ch == '1')
                        break;
                    seenSymbols = xdigits;
                    /* FALL THROUGH */
                case xdigits:
                    if (isxdigit(ch))
                        break;
                    seenSymbols = other;
                case other:
                    break;
                }
                token.lexem += ch;
                /*if (cp - token < maxtlen - 2)
                    *cp++ = ch;*/
            }
            if (ch == '\'')
            {
                unsigned long   val = 0;
                //char           *run = token + 1;
                ch = file.get();
                switch (ch)
                {
                case EOF:
                    token.type = LT::eENDOFFILE;
                    return;
                case 'b':
                case 'B':
                    if (seenSymbols > bdigits)
                    {
                        token.lexem += '\'';
                        token.type = LT::eLABEL;
                        return;
                        /**cp++ = '\'';
                        *cp = 0;
                        return LABEL;*/
                    }
                    for (int i = 1; i < token.lexem.size(); ++i)
                        val = val * 2 + token.lexem[i] - '0';
                    //while (run != token.lexem.back())
                    //	val = val * 2 + *run++ - '0';
                    break;
                case 'h':
                case 'H':
                    if (seenSymbols > xdigits)
                    {
                        token.lexem += '\'';
                        token.type = LT::eLABEL;
                        return;
                        /**cp++ = '\'';
                        *cp = 0;
                        return LABEL;*/
                    }
                    for (int i = 1; i < token.lexem.size(); ++i)
                    {
                        ch = token.lexem[i];
                        if ('0' <= ch && ch <= '9')
                            val = val * 16 + ch - '0';
                        else if ('a' <= ch && ch <= 'f')
                            val = val * 16 + ch - 'a' + 10;
                        else if ('A' <= ch && ch <= 'F')
                            val = val * 16 + ch - 'A' + 10;
                    }
                    break;
                default:
                    token.lexem += '\'';
                    token.type = LT::eLABEL;
                    return;
                    /**cp++ = '\'';
                    *cp = 0;
                    return LABEL;*/
                }
                token.lexem = std::to_string(val);
                token.type = LT::eNUMBER;
                return;
                //sprintf(token, "%ld", val);
                //return NUMBER;
            }
            token.lexem += ch;
            token.type = LT::eLABEL;
            return;

            //return LABEL;
        case '(':
            token.type = LT::eLEFTPAREN;
            return;
            //return LEFTPAREN;
        case ')':
            token.type = LT::eRIGHTPAREN;
            return;
            //return RIGHTPAREN;
        case '{':
            token.type = LT::eLEFTBRACKET;
            return;
            //return LEFTBRACKET;
        case '}':
            token.type = LT::eRIGHTBRACKET;
            return;
            //return RIGHTBRACKET;
        case '[':
            token.type = LT::eLEFTSQBRACK;
            return;
            //return LEFTSQBRACK;
        case ']':
            token.type = LT::eRIGHTSQBRACK;
            return;
            //return RIGHTSQBRACK;
        case ';':
            token.type = LT::eSEMI;
            return;
            //return SEMI;
        case ',':
            token.type = LT::eCOMMA;
            return;
            //return COMMA;
        case '|':
            token.type = LT::eBAR;
            return;
            //return BAR;
        case '.':
            token.lexem += file.get();
            //ch_next = file.get();
            //ch_next = netsnmp_getc(fp);
            if (token.lexem.back() == '.')
            {
                token.type = LT::eRANGE;
                return;
                //return RANGE;
            }
            file.unget();
            token.type = LT::eLABEL;
            return;
            //ungetc(ch_next, fp);
            //return LABEL;
        case ':':
            //ch_next = file.get();
            token.lexem += file.get();
            //ch_next = netsnmp_getc(fp);
            if (token.lexem.back() != ':')
            {
                file.unget();
                token.type = LT::eLABEL;
                return;
                //ungetc(ch_next, fp);
                //return LABEL;
            }
            token.lexem += file.get();
            //ch_next = file.get();
            //ch_next = netsnmp_getc(fp);
            if (token.lexem.back() != '=')
            {
                file.unget();
                token.type = LT::eLABEL;
                return;
                //ungetc(ch_next, fp);
                //return LABEL;
            }
            token.type = LT::eEQUALS;
            return;
            //return EQUALS;
        case '-':
            ch = file.get();
            //ch_next = netsnmp_getc(fp);
            if (ch == '-')
            {
                token.lexem += ch;
                while ((ch != EOF) && (ch != '\n'))
                    ch = file.get();

                if (ch == '\n')
                    ++_line;

                more = true;
                continue;
                //parseToken(file, token);
                //return;
            }
            file.unget();
            //ungetc(ch_next, fp);
            /* fallthrough */
        default:
            /*
            * Accumulate characters until end of token is found.  Then attempt to
            * match this token as a reserved word.  If a match is found, return the
            * type.  Else it is a label.
            */
            if (!isLabelChar(ch))
            {
                token.type = LT::eLABEL;
                return;
            }

            //bool more = true;
            while (true)
            {
                while (isLabelChar(ch = file.get()))
                    token.lexem += ch;


                file.unget();

                auto it = _lexemTypes.find(token.lexem);

                if (it != _lexemTypes.end())
                {
                    token.type = it->second;

                    if (token.type != LT::eCONTINUE)
                        return;

                    while (isspace((ch = file.get())));

                    if (ch == '\n')
                        ++_line;

                    /*if (ch_next == '\n')
                        mibLine++;*/

                    if (ch == EOF)
                    {
                        token.type = LT::eENDOFFILE;
                        return;
                    }

                    if (isalnum(ch))
                    {
                        token.lexem += ch;
                        continue;
                    }
                }

                break;
            }

            if (token.lexem[0] == '-' || isdigit((unsigned char)(token.lexem[0])))
            {
                for (int i = 1; i < token.lexem.size(); ++i)
                    if (!isdigit((unsigned char)token.lexem[i]))
                    {
                        token.type = LT::eLABEL;
                        return;
                    }

                token.type = LT::eNUMBER;
                return;
            }

            token.type = LT::eLABEL;
        }
    }
}

void Parser::parseQuoteString(std::ifstream& file, Token& token)
{
    //token.lexem.clear();

    for (char ch = file.get(); ch != EOF; ch = file.get())
    {
        if (ch == '\n')
        {
            if (token.lexem.back() != ' ')
                token.lexem += ' ';

            ++_line;
            continue;
        }

        if (ch == '\r' || (token.lexem.back() == ' ' && ch == ' '))
            continue;

        token.lexem += ch;

        //formatted_quote << ch;

        //token.lexem += ch;

        if (ch == '"')
        {
            //std::stringstream formatted_quote(token.lexem);
            //std::string word;
            //token.lexem.clear();


            //while (formatted_quote >> word)
            //{
            //	token.lexem += word + ' ';
            //}
            //token.lexem.pop_back();

            token.type = LT::eQUOTESTRING;
            return;
        }
    }

    token.type = LT::eENDOFFILE;
}

NodeList Parser::parse(std::ifstream& file)
{
    //char            LT[MAXLexemType];
    //char            name[MAXLexemType + 1];

    _line = 1;
    _moduleName.clear();
    _errinf.isError = false;
    _errinf.description.clear();

    Token token;
    NodeList result;
    NodeList nodes;
    Module::Ptr mp;

    //std::string LT;
    std::string moduleName;
    std::string name;

    LT lasttype = LT::eLABEL;

#define BETWEEN_MIBS          1
#define IN_MIB                2
    int             state = BETWEEN_MIBS;
    //Node    *np, *nnp;
    //Objgroup groups, objects, notifs;

    //DEBUGMSGTL(("parse-file", "Parsing file:  %s...\n", File));

    //if (last_err_module)
    //	free(last_err_module);
    //last_err_module = NULL;

    //np = root;
    //if (np != NULL)
    //{
    //	/*
    //	* now find end of chain
    //	*/
    //	while (np->next)
    //		np = np->next;
    //}

    while (token.type != LT::eENDOFFILE)
    {
        if (lasttype == LT::eCONTINUE)
            lasttype = token.type;
        else
        {
            parseToken(file, token);
            lasttype = token.type;
        }
        //type = lasttype = parseToken(file, token);

        switch (token.type)
        {
        case LT::eEND:
            if (state != IN_MIB)
            {
                _errinf.isError = true;
                _errinf.description = formError("Error, END before start of MIB", token.lexem);
                return EmptyResult;
                //print_error("Error, END before start of MIB", NULL, type);
                //gMibError = MODULE_SYNTAX_ERROR;
                //result.clear();
                //return result;
            }
            else
            {
                //struct module  *mp;
//#ifdef TEST
//				printf("\nNodes for Module %s:\n", name);
//				print_nodes(stdout, root);
//#endif

                scanObjlist(result, mp, _objgroups);
                if (_errinf.isError)
                    return EmptyResult;

                scanObjlist(result, mp, _objects);
                if (_errinf.isError)
                    return EmptyResult;

                scanObjlist(result, mp, _notifs);
                if (_errinf.isError)
                    return EmptyResult;

                _tree->linkUp(result);

                if (!result.empty())
                {
                    _errinf.isError = true;
                    _errinf.description = "Can't link up nodes of " + _moduleName;
                    return EmptyResult;
                }

                //auto it = _moduleList->_moduleList.find(moduleName);
                //
                //if (it != _moduleList->_moduleList.end())
                //{
                //}

                //for (mp = module_head; mp; mp = mp->next)
                //	if (mp->modid == current_module)
                //		break;
                //scan_objlist(root, mp, objgroups, "Undefined OBJECT-GROUP");
                //scan_objlist(root, mp, objects, "Undefined OBJECT");
                //scan_objlist(root, mp, notifs, "Undefined NOTIFICATION");
                //objgroups = oldgroups;
                //objects = oldobjects;
                //notifs = oldnotifs;
                //do_linkup(mp, root);
                //np = root = NULL;
            }
            state = BETWEEN_MIBS;

            continue;
        case LT::eIMPORTS:

            parseImports(file, moduleName);

            if (_errinf.isError)
                return EmptyResult;

            //parse_imports(fp);
            continue;
        case LT::eEXPORTS:

            while (token.type != LT::eSEMI && token.type != LT::eENDOFFILE)
                parseToken(file, token);

            //while (type != SEMI && type != ENDOFFILE)
            //	type = get_LexemType(fp, LT, MAXLexemType);
            continue;
            //case LABEL:
            //case INTEGER:
            //case INTEGER32:
            //case UINTEGER32:
            //case UNSIGNED32:
            //case COUNTER:
            //case COUNTER64:
            //case GAUGE:
            //case IPADDR:
            //case NETADDR:
            //case NSAPADDRESS:
            //case OBJSYNTAX:
            //case APPSYNTAX:
            //case SIMPLESYNTAX:
            //case OBJNAME:
            //case NOTIFNAME:
            //case KW_OPAQUE:
            //case TIMETICKS:
            //	break;
        case LT::eENDOFFILE:
            continue;
        default:
            break;
            //strlcpy(name, LT, sizeof(name));
            //type = get_LexemType(fp, LT, MAXLexemType);
            //nnp = NULL;
            //if (type == MACRO)
            //{
            //	nnp = parse_macro(fp, name);
            //	if (nnp == NULL)
            //	{
            //		print_error("Bad parse of MACRO", NULL, type);
            //		gMibError = MODULE_SYNTAX_ERROR;
            //		/*
            //		* return NULL;
            //		*/
            //	}
            //	free_node(nnp); /* IGNORE */
            //	nnp = NULL;
            //}
            //else
            //	print_error(name, "is a reserved word", lasttype);
            //continue;           /* see if we can parse the rest of the file */
        }
        name = token.lexem;
        //strlcpy(name, LT, sizeof(name));
        parseToken(file, token);
        //type = get_LexemType(fp, LT, MAXLexemType);
        //nnp = NULL;

        /*
        * Handle obsolete method to assign an object identifier to a
        * module
        */
        if (lasttype == LT::eLABEL && token.type == LT::eLEFTBRACKET)
        {
            while (token.type != LT::eRIGHTBRACKET && token.type != LT::eENDOFFILE)
                parseToken(file, token);

            //while (type != RIGHTBRACKET && type != ENDOFFILE)
            //	type = get_LexemType(fp, LT, MAXLexemType);

            if (token.type == LT::eENDOFFILE)
            {
                _errinf.isError = true;
                _errinf.description = formError("Expected \"}\"", token.lexem);
                return EmptyResult;
                //result.clear();
                //return result;
                //print_error("Expected \"}\"", LT, type);
                //gMibError = MODULE_SYNTAX_ERROR;
                //goto error;
                //return NULL;
            }

            parseToken(file, token);
            //type = get_LexemType(fp, LT, MAXLexemType);
        }

        switch (token.type)
        {
        case LT::eDEFINITIONS:

            if (state != BETWEEN_MIBS)
            {
                _errinf.isError = true;
                _errinf.description = formError("Error, nested MIBS", token.lexem);
                return EmptyResult;
                //result.clear();
                //return result;
                //print_error("Error, nested MIBS", NULL, type);
                //gMibError = MODULE_SYNTAX_ERROR;
                //goto error;
                //return NULL;
            }
            state = IN_MIB;

            //auto itt = _moduleList->_moduleList.find(name);

            //if (itt == _moduleList->_moduleList.end())
            //{
            //	//new module
            //}

            while (token.type != LT::eENDOFFILE)
            {
                parseToken(file, token);
                if (token.type == LT::eBEGIN)
                    break;
            }

            _moduleName = moduleName = name;

            mp = _tree->findModule(moduleName);

            if (!mp)
            {
                _errinf.isError = true;
                _errinf.description = "Module " + token.lexem + " not found.";
                return EmptyResult;
            }

            mp->isParsed = true;

            //current_module = which_module(name);
            //oldgroups = objgroups;
            //objgroups = NULL;
            //oldobjects = objects;
            //objects = NULL;
            //oldnotifs = notifs;
            //notifs = NULL;
            //if (current_module == -1)
            //{
            //	new_module(name, File);
            //	current_module = which_module(name);
            //}
            //DEBUGMSGTL(("parse-mibs", "Parsing MIB: %d %s\n",
            //	current_module, name));
            //while ((type = get_LexemType(fp, LT, MAXLexemType)) != ENDOFFILE)
            //	if (type == BEGIN)
            //		break;
            break;
        case LT::eOBJTYPE:
        {
            nodes = parseObjType(file, name, moduleName);

            if (_errinf.isError)
                return EmptyResult;
            //if (nodes.empty())
            //{
            //	return EmptyResult;
            //	//result.clear();
            //	//return result;
            //}
            //result.merge(nodes);
            //nnp = parse_objecttype(fp, name);
            //if (nnp == NULL)
            //{
            //	//if (!netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_MIB_SAVE_ERRORS))
            //	//	print_error("Bad parse of OBJECT-TYPE", NULL, type);
            //	gMibError = MODULE_SYNTAX_ERROR;
            //	goto error;
            //	//return NULL;
            //}
            break;
        }
        case LT::eOBJGROUP:
        {
            nodes = parseObjectGroup(file, name, moduleName, LT::eOBJECT, _objects);

            if (_errinf.isError)
                return EmptyResult;
            //if (nodes.empty())
            //{
            //	return EmptyResult;
            //	//result.clear();
            //	//return result;
            //}
            //result.merge(nodes);
            //nnp = parse_objectgroup(fp, name, OBJECTS, &objects);
            //if (nnp == NULL)
            //{
            //	//if (!netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_MIB_SAVE_ERRORS))
            //	//	print_error("Bad parse of OBJECT-GROUP", NULL, type);
            //	gMibError = MODULE_SYNTAX_ERROR;
            //	goto error;
            //	//return NULL;
            //}
            break;
        }
        case LT::eNOTIFGROUP:
        {
            nodes = parseObjectGroup(file, name, moduleName, LT::eNOTIFICATIONS, _notifs);

            if (_errinf.isError)
                return EmptyResult;
            //if (nodes.empty())
            //{
            //	return EmptyResult;
            //	//result.clear();
            //	//return result;
            //}
            //result.merge(nodes);
            //nnp = parse_objectgroup(fp, name, NOTIFICATIONS, &notifs);
            //if (nnp == NULL)
            //{
            //	print_error("Bad parse of NOTIFICATION-GROUP", NULL, type);
            //	gMibError = MODULE_SYNTAX_ERROR;
            //	goto error;
            //	//return NULL;
            //}
            break;
        }
        case LT::eTRAPTYPE:
        {
            nodes = parseTrap(file, name, moduleName);

            if (_errinf.isError)
                return EmptyResult;
            //if (nodes.empty())
            //{
            //	return EmptyResult;
            //	//result.clear();
            //	//return result;
            //}
            //result.merge(nodes);

            //nnp = parse_trapDefinition(fp, name);
            //if (nnp == NULL)
            //{
            //	print_error("Bad parse of TRAP-TYPE", NULL, type);
            //	gMibError = MODULE_SYNTAX_ERROR;
            //	goto error;
            //	//return NULL;
            //}
            break;
        }
        case LT::eNOTIFTYPE:
        {
            nodes = parseNotifType(file, name, moduleName);

            if (_errinf.isError)
                return EmptyResult;
            //if (nodes.empty())
            //{
            //	return EmptyResult;
            //	//result.clear();
            //	//return result;
            //}
            //result.merge(nodes);
            //nnp = parse_notificationDefinition(fp, name);
            //if (nnp == NULL)
            //{
            //	print_error("Bad parse of NOTIFICATION-TYPE", NULL, type);
            //	gMibError = MODULE_SYNTAX_ERROR;
            //	goto error;
            //	//return NULL;
            //}
            break;
        }
        case LT::eCOMPLIANCE:
        {
            nodes = parseCompliance(file, name, moduleName);

            if (_errinf.isError)
                return EmptyResult;
            //if (nodes.empty())
            //{
            //	return EmptyResult;
            //	//result.clear();
            //	//return result;
            //}
            //result.merge(nodes);
            //nnp = parse_compliance(fp, name);
            //if (nnp == NULL)
            //{
            //	print_error("Bad parse of MODULE-COMPLIANCE", NULL, type);
            //	gMibError = MODULE_SYNTAX_ERROR;
            //	goto error;
            //	//return NULL;
            //}
            break;
        }
        case LT::eAGENTCAP:
        {
            nodes = parseCapabilities(file, name, moduleName);

            if (_errinf.isError)
                return EmptyResult;
            //if (nodes.empty())
            //{
            //	return EmptyResult;
            //	//result.clear();
            //	//return result;
            //}
            //result.merge(nodes);
            //nnp = parse_capabilities(fp, name);
            //if (nnp == NULL)
            //{
            //	print_error("Bad parse of AGENT-CAPABILITIES", NULL, type);
            //	gMibError = MODULE_SYNTAX_ERROR;
            //	goto error;
            //	//return NULL;
            //}
            break;
        }
        case LT::eMACRO:
        {
            nodes = parseMacro(file);

            if (_errinf.isError)
                return EmptyResult;
            //if (nodes.empty())
            //{
            //	return EmptyResult;
            //	//result.clear();
            //	//return result;
            //}

            nodes.clear();

            //result.merge(nodes);

            //nnp = parse_macro(fp, name);
            //if (nnp == NULL)
            //{
            //	print_error("Bad parse of MACRO", NULL, type);
            //	gMibError = MODULE_SYNTAX_ERROR;
            //	/*
            //	* return NULL;
            //	*/
            //}
            //free_node(nnp);     /* IGNORE */
            //nnp = NULL;
            break;
        }
        case LT::eMODULEIDENTITY:
        {
            nodes = parseModuleIdentity(file, name, moduleName);

            if (_errinf.isError)
                return EmptyResult;
            //if (nodes.empty())
            //{
            //	return EmptyResult;
            //	//result.clear();
            //	//return result;
            //}
            //result.merge(nodes);
            //nnp = parse_moduleIdentity(fp, name);
            //if (nnp == NULL)
            //{
            //	print_error("Bad parse of MODULE-IDENTITY", NULL, type);
            //	gMibError = MODULE_SYNTAX_ERROR;
            //	goto error;
            //	//return NULL;
            //}
            break;
        }
        case LT::eOBJIDENTITY:
        {
            nodes = parseObjectGroup(file, name, moduleName, LT::eOBJECT, _objects);

            if (_errinf.isError)
                return EmptyResult;
            //if (nodes.empty())
            //{
            //	return EmptyResult;
            //	//result.clear();
            //	//return result;
            //}
            //result.merge(nodes);
            //nnp = parse_objectgroup(fp, name, OBJECTS, &objects);
            //if (nnp == NULL)
            //{
            //	print_error("Bad parse of OBJECT-IDENTITY", NULL, type);
            //	gMibError = MODULE_SYNTAX_ERROR;
            //	goto error;
            //	//return NULL;
            //}
            break;
        }
        case LT::eOBJECT:
        {
            parseToken(file, token);
            //type = get_LexemType(fp, LT, MAXLexemType);
            if (token.type != LT::eIDENTIFIER)
            {
                _errinf.isError = true;
                _errinf.description = formError("Expected IDENTIFIER", token.lexem);
                return EmptyResult;
                //result.clear();
                //return result;
                //print_error("Expected IDENTIFIER", LT, type);
                //gMibError = MODULE_SYNTAX_ERROR;
                //goto error;
                //return NULL;
            }
            parseToken(file, token);
            //type = get_LexemType(fp, LT, MAXLexemType);
            if (token.type != LT::eEQUALS)
            {
                _errinf.isError = true;
                _errinf.description = formError("Expected \"::=\"", token.lexem);
                return EmptyResult;
                //result.clear();
                //return result;
                //print_error("Expected \"::=\"", LT, type);
                //gMibError = MODULE_SYNTAX_ERROR;
                //goto error;
                //return NULL;
            }
            //nnp = parse_objectid(fp, name);
            nodes = parseObjectid(file, name, moduleName);

            if (_errinf.isError)
                return EmptyResult;
            //if (nodes.empty())
            //{
            //	return EmptyResult;
            //	//result.clear();
            //	//return result;
            //}

            //result.merge(nodes);
            //if (nnp == NULL)
            //{
            //	print_error("Bad parse of OBJECT IDENTIFIER", NULL, type);
            //	gMibError = MODULE_SYNTAX_ERROR;
            //	goto error;
            //	//return NULL;
            //}
            break;
        }
        case LT::eEQUALS:
        {
            nodes = parseASN(file, token, name, moduleName);

            if (_errinf.isError)
                return EmptyResult;

            //if (!nodes.empty())
            //{
            //	result.merge(nodes);
            //}

            //nnp = parse_asntype(fp, name, &type, LT);
            lasttype = LT::eCONTINUE;
            break;
        }
        case LT::eENDOFFILE:
            break;
        default:

            _errinf.isError = true;
            _errinf.description = formError("Bad operator", token.lexem);
            return EmptyResult;

            //result.clear();
            //return result;
            //print_error("Bad operator", LT, type);
            //gMibError = MODULE_SYNTAX_ERROR;
            //goto error;

            //return NULL;
        }

        if (!nodes.empty())
            result.merge(nodes);

        //if (nnp)
        //{
        //	if (np)
        //		np->next = nnp;
        //	else
        //		np = root = nnp;
        //	while (np->next)
        //		np = np->next;
        //	if (np->type == TYPE_OTHER)
        //		np->type = type;
        //}
    }

    std::cout << "Loading " + _moduleName + ": OK" << std::endl;
    return result;
    //DEBUGMSGTL(("parse-file", "End of file (%s)\n", File));
    //return root;

//error:
//
//	for (np = root; np; np = np->next)
//		free_node(np);
//
//	return NULL;
}

const ErrorInfo& Parser::lastErrorInfo()
{
    return _errinf;
}

/* Copyright (c) 2002 - 2003 Jeffrey Myers
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named license.txt that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

// parser.cpp
#include "parser.h"
#include "textUtils.h"

CSGMLParser::CSGMLParser()
{
}

CSGMLParser::CSGMLParser( const char* file )
{
	load (file);
}

CSGMLParser::CSGMLParser(FILE *file )
{
	load (file);
}

CSGMLParser::CSGMLParser( std::string data )
{
	load (data);
}

CSGMLParser::~CSGMLParser()
{

}

bool CSGMLParser::load ( const char* file )
{
	FILE	*fp = fopen(file,"rt");
	if (!fp)
		return false;
	bool ret = load ( fp );
	fclose(fp);
	return ret;
}

bool CSGMLParser::load ( FILE *file )
{
	if (!file)
		return false;

	std::string data;
	unsigned int len;
	fseek(file,SEEK_END,0);
	len = ftell(file);
	fseek(file,SEEK_SET,0);
	
	char *p = (char*)malloc(len);
	fread(p,len,1,file);
	fclose(file);

	data = p;
	free(p);

	return load(data);
}

bool CSGMLParser::load ( std::string data )
{
	rootBlock.elements.clear();
	rootBlock.name = "";
	rootBlock.subBlocks.clear();

	std::string::iterator strItr = data.begin();
	while (strItr != data.end())
	{
		if (*strItr == 13)
			strItr = data.erase(strItr);
		else
			strItr ++;
	}

	// now break up the file into a vector with each element being one line
	std::vector<std::string> lines = tokenize(data,std::string("\n"));

	// scan the lines and remove blanks or comments
	std::vector<std::string>::iterator lineItr = lines.begin();

	while ( lineItr != lines.end() )
	{
		std::string theLine = *lineItr;
		*lineItr = trim_whitespace(*lineItr);
		theLine = *lineItr;
		if ( !theLine.size() || theLine[0] != '<')	// all lines MUST have <> in them
			lineItr = lines.erase(lineItr);
		else
			lineItr++;
	}

	lineItr = lines.begin();

	// ok lets fill this sucker in
	if (!parseBlock(rootBlock,lineItr,lines))
		return false;	// the map was not complete

	// now read thru the blocks calling the callbacks if there are any
	bool parseKids = true;
	callblockCallbacks(rootBlock,NULL,parseKids);

	return true;
}


void* CSGMLParser::callblockCallbacks ( trParserBlock &block, void * parrent, bool &parseChildren )
{
	void *newParent = parrent;
	tmCallbackNameMap::iterator itr = callbacks.find(toupper(block.name));
	if ( itr != callbacks.end())
		newParent = itr->second->process(block,parrent,parseChildren);
	else
		parseChildren = true;

	if ( parseChildren )
	{
		std::vector<trParserBlock>::iterator children = block.subBlocks.begin();

		while ( children != block.subBlocks.end())
		{
			bool kidsParseKids = true;
			callblockCallbacks( *children, newParent,kidsParseKids);
			children++;
		}
	}
	return newParent;
}

trParserBlock* CSGMLParser::findBlock ( std::string name, trParserBlock* startBlock )
{
	if (!startBlock)
		startBlock = &rootBlock;

	if (compare_nocase(startBlock->name,name) ==0)
		return startBlock;

	std::vector<trParserBlock>::iterator itr = startBlock->subBlocks.begin();

	while ( itr != startBlock->subBlocks.end() )
	{
		trParserBlock* ret = findBlock(name, &(*itr));
		if (ret)
			return ret;
		itr++;
	}
	return NULL;
}

trParserBlock* CSGMLParser::findBlock ( const char *name, trParserBlock* startBlock )
{
	if (!name)
		return NULL;

	return findBlock(std::string(name),startBlock);
}

bool CSGMLParser::setCallback ( std::string name, CBaseParserObjectCallback* callback )
{
	if (!callback)
		return false;

	callbacks[toupper(name)] = callback;
	return true;
}

bool CSGMLParser::setCallback ( const char *name, CBaseParserObjectCallback* callback )
{
	if (!name)
		return false;
	return setCallback(std::string(name),callback);
}

bool CSGMLParser::parseLine ( std::string &line, trBlockElement &element )
{
	if (!line.size())
		return false;

	std::string::iterator itr = line.begin();

	element.blockEnd = false;
	element.blockStart = false;
	element.name = "";
	element.value = "";

	bool gotTagStart = false;
	bool gotName = false;
	bool hasValue = false;
	bool inQuotes = false;
	bool hitEndSlash = false;
	bool endProper = false;

	while ( itr != line.end() )
	{
		if ( !gotTagStart )
		{
			if (*itr == '<')
				gotTagStart = true;
		}
		else
		{
			if (!gotName)
			{
				if ( *itr =='/')
					element.blockEnd = true;
				else if ( isWhitespace(*itr) || *itr == '>')
					gotName = true;
				else
					element.name += *itr;

				if (gotName && *itr == '>')
					element.blockStart = true;
			}
			else	// we have the tagname find a value
			{
				if (!hasValue)
				{
					if ( *itr == '=' )
						hasValue = true;
				}
				else
				{
					if (!isWhitespace(*itr) && !element.value.size())	// start of the thing
					{
						if (*itr == '\"')
							inQuotes = true;
						else
							element.value += *itr;
					}
					else if (element.value.size())
					{
						if (inQuotes)
						{
							if (*itr == '\"')
								inQuotes = false;
							else
								element.value += *itr;
						}
						else if (*itr == '/')
						{
							element.blockStart = false;
							element.blockEnd = false;
							hitEndSlash = true;
							endProper = true;
							itr = line.end();
						}
						else if (*itr == '>')
						{
							if (!hitEndSlash)
								element.blockStart = true;
						}
						else if (!isWhitespace(*itr))
							element.value += *itr;
					}
				}
			}
		}
		if ( itr != line.end() )
		{
			if (*itr == '>')	// end of the tag, don't care about the rest
			{
				endProper = true;
				itr = line.end();
			}
			else
				itr++;
		}
	}
	return endProper;
}

bool CSGMLParser::parseBlock ( trParserBlock &block, std::vector<std::string>::iterator &itr, std::vector<std::string> &lines )
{
	trBlockElement	element;
	bool inBlock = false;

	// find the first block start, thats our block
	parseLine(*(itr++),element);
	while ( !element.blockStart && itr != lines.end() )
		parseLine(*(itr++),element);

	inBlock = true;

	block.name = element.name;

	while ( inBlock && itr != lines.end() )
	{
		bool err = !parseLine(*(itr),element);

		if (err)
			return false;

		if (element.blockEnd)
		{
			inBlock = false;
			itr++;
		}
		else
		{
			if (element.blockStart)// start of a new block, let the recursion begin
			{
				//	itr++;
				trParserBlock	subBlock;
				err = !parseBlock(subBlock,itr,lines);
				if (err)
					return false;
				else
					block.subBlocks.push_back(subBlock);
			}
			else	// just a regular element
			{
				itr++;
				tpStringPair item;
				item.first = element.name;
				item.second = element.value;
				block.elements.push_back(item);
			}
		}
	}

	return true;
}


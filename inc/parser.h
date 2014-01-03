/* 
* Copyright (c) 2002 - 2003 Jeffrey Myers
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named license.txt that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/
// Parser.h

#ifndef _PARSER_H_
#define _PARSER_H_
#  pragma warning(disable: 4786)

#include <string>
#include <map>
#include <vector>

typedef std::pair<std::string,std::string> tpStringPair;
typedef std::vector<tpStringPair> tvStringPairList;

typedef struct trParserBlock
{
	std::string name;
	tvStringPairList	elements;
	std::vector<trParserBlock>	subBlocks;
}trParserBlock;

typedef struct 
{
	std::string name;
	std::string value;
	bool		blockStart;
	bool		blockEnd;
}trBlockElement;

class CBaseParserObjectCallback
{
public:
	virtual ~CBaseParserObjectCallback(){};

	// callback to be called for each block of this type.
	// can return a pointer to some object that will be passed to any child objects.
	virtual void* process ( trParserBlock &block, void * parrent, bool &parseChildren ) = 0;
};

typedef std::map<std::string, CBaseParserObjectCallback*> tmCallbackNameMap;

class CSGMLParser
{
public:
	CSGMLParser();
	CSGMLParser( const char* file );
	CSGMLParser( FILE *file );
	CSGMLParser( std::string data );
	~CSGMLParser();

	bool load ( const char* file );
	bool load ( FILE *file );
	bool load ( std::string data );

	trParserBlock* findBlock ( std::string name, trParserBlock* startBlock = NULL );
	trParserBlock* findBlock ( const char *name, trParserBlock* startBlock = NULL );

	bool setCallback ( std::string name, CBaseParserObjectCallback* callback );
	bool setCallback ( const char *name, CBaseParserObjectCallback* callback );

	trParserBlock*	getRootBlock ( void ) {return &rootBlock;}
protected:
	bool parseLine ( std::string &line, trBlockElement &element );
	bool parseBlock ( trParserBlock &block, std::vector<std::string>::iterator &itr, std::vector<std::string> &lines );

	void* callblockCallbacks ( trParserBlock &block, void * parrent, bool &parseChildren );
	tmCallbackNameMap	callbacks;

	trParserBlock rootBlock;
};
#endif // parser
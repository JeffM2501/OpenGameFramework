// prefs.cpp

#include "prefs.h"

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#else
#include<stdlib.h>
#endif //WIN32

#include <stdio.h>
#include <stdlib.h>

// global "keeper"
CPrefsManager *gPrefs = NULL;

void SetPrefs ( CPrefsManager *prefs )
{
	gPrefs = prefs;
}

CPrefsManager* GetPrefs ( void )
{
	return gPrefs;
}

// prefs class
CPrefsManager::CPrefsManager()
{
	itemMap.clear();
}

CPrefsManager::~CPrefsManager()
{
	Update();
}

const char* CPrefsManager::GetPrefsPath ( void )
{
	std::string	filePathName;
#ifdef _WIN32

	char	szSysDir[MAX_PATH] = {0};
	char	buffer[MAX_PATH] = {0};

	// do the old way
	GetWindowsDirectory(szSysDir,MAX_PATH);
	filePathName = szSysDir;
	filePathName += "\\";

	// see if we can get it the new way
	HRESULT hr;
	if (SUCCEEDED (hr = SHGetSpecialFolderPath(NULL, buffer,  CSIDL_PERSONAL, FALSE)))
	{
		strcpy(szSysDir, buffer);
		strcat(szSysDir, "\\");
		filePathName = szSysDir;
	}

#else
	// do some sort of home dir thing here
	filePathName =  getenv("HOME");
#endif

	static COSFile	temp;
	temp.OSName(filePathName.c_str());

	return temp.GetStdName();
}

const char* CPrefsManager::GetPrefsFileName ( const char *pathName )
{
	std::string	filePathName;
	static COSFile temp(pathName);
#ifdef _WIN32
	filePathName += temp.GetOSName();	
	filePathName += ".cfg";
#else
	// do some sort of home dir thing here
	filePathName += "/.";
	filePathName += temp.GetOSName();
#endif
	temp.OSName(filePathName.c_str());
	return temp.GetStdName();
}

void CPrefsManager::Init ( std::string fileName )
{
	Init(fileName.c_str());
}

void CPrefsManager::Init ( const char *pathName )
{
	if (!pathName || (file.GetOSName()=='\0'))
		return;

	itemMap.clear();

	std::string	filePathName = GetPrefsPath();
	filePathName += GetPrefsFileName(pathName);	
	file.SetUseGlobalPath(false);
	file.StdName(filePathName.c_str());

	file.Open("rt");
	if (!file.IsOpen())
		return;

	char szName[256];
	char szData[256];

	std::string	name;
	std::string data;

	bool bDone = false;
	while (!bDone)
	{
		szName[0] = 0;
		szData[0] = 0;

		std::string line = file.ReadLine();
		if (!line.size())
			bDone = true;
		else
		{
			sscanf(line.c_str(),"%s",szName);
			strcpy(szData,line.c_str()+strlen(szName)+1);

			name = szName;
			data = szData;
			itemMap[name] = data;
		}
	}
	file.Close();
}

void CPrefsManager::Update ( void )
{
	if ((file.GetOSName()=='\0') || (GetNumItems()<1))
		return;

	file.Open("wt");
	if (!file.IsOpen())
		return;

	tmPrefsMap::iterator	itr = itemMap.begin();
	std::string line;

	while (itr != itemMap.end())
	{
		line = itr->first + "\t" + itr->second + "\n";
		file.Write((void*)line.c_str(),(int)line.size());
		itr++;
	}
	file.Close();
}

int CPrefsManager::GetNumItems ( void )
{
	return (int)itemMap.size();
}

bool CPrefsManager::ItemExists ( const char* szName )
{
	if (!szName)
		return false;

	std::string	str = szName;

	if (itemMap.find(str) == itemMap.end())
		return false;
	return true;
}

void CPrefsManager::SetItem ( const char* szName, int iData )
{
	if (!szName)
		return;

	char	szData[256];
	sprintf(szData,"%d",iData);

	std::string	name = szName;
	std::string data = szData;
	itemMap[name] = data;
}

void CPrefsManager::SetItem ( const char* szName, float fData )
{
	if (!szName)
		return;

	char	szData[256];
	sprintf(szData,"%f",fData);

	std::string	name = szName;
	std::string data = szData;

	itemMap[name] = data;
}

void CPrefsManager::SetItem ( const char* szName, char* pData )
{
	if (!szName || !pData)
		return;

	std::string	name = szName;
	std::string data = pData;

	itemMap[name] = data;
}

int CPrefsManager::GetItemI ( const char* szName )
{
	if (!szName)
		return 0;

	std::string	name = szName;
	tmPrefsMap::iterator	itr = itemMap.find(name);

	if (itr == itemMap.end())
		return 0;

	return atoi(itr->second.c_str());
}

float CPrefsManager::GetItemF ( const char* szName )
{
	if (!szName)
		return 0;

	std::string	name = szName;

	tmPrefsMap::iterator	itr = itemMap.find(name);

	if (itr == itemMap.end())
		return 0;

	return (float)atof(itr->second.c_str());
}

const char* CPrefsManager::GetItemS ( const char* szName )
{
	if (!szName)
		return 0;

	std::string	name = szName;
	tmPrefsMap::iterator	itr = itemMap.find(name);

	if (itr == itemMap.end())
		return 0;
	return itr->second.c_str();
}


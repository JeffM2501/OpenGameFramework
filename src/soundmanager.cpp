/* soundmanager
* Copyright (c) 2004 Jeffery A Myers
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named LICENSE that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

// sound manager

#include "soundmanager.h"
#include "OSFile.h"

#include "textUtils.h"
#include "fmod.h"
#include <string>
#include <map>

#include <map>
#include <string>
#include <vector>
#include <algorithm>

void GetUperName_sound ( char *szData )
{
#ifdef	_WIN32
	strupr(szData);
#else
	while(*szData) {
		*szData = toupper(*szData);
		szData++;
	}
#endif
}

std::string getSampleName ( COSFile &file, bool forceUpper = true )
{
	std::string temp;
	if (forceUpper)
		temp = string_util::toupper(file.GetStdName());
	else
		temp = file.GetStdName();

	if ( file.GetExtension() )
		temp.erase(temp.end()-strlen(file.GetExtension())-1,temp.end());
	return temp;
}	

std::string getSampleName ( const std::string name )
{
	std::string temp = name;
	GetUperName_sound((char*)temp.c_str());
	return temp;
}

bool validSoundExtenstion ( const char * ext )
{
	if (!ext)
		return false;

	std::string toTest = ext;
	GetUperName_sound((char*)toTest.c_str());

	if (toTest == "WAV")
		return true;
	if (toTest == "MP3")
		return true;
	if (toTest == "OOG")
		return true;

	return false;
}

// default init struct
trSoundManagerInit::trSoundManagerInit()
{
	outputType = eSoundAutoDetect;					
	tryAlternateOutput = true;	
	speakerType = eSterio;				
	soundBufferSize = 2 * 1024*1024;		
	streamBufferSize = 1 * 1024*1024;		
	OSWindow = NULL;					
	minHardwareChans = -1;		
	maxChannels = 32;				
	preferedDriver = -1;		
	sampleRate = eSound44khz;
}

typedef struct 
{
	COSFile				file;
	std::string			name;
	int					refCount;
	FSOUND_SAMPLE		*stdSample;
	FSOUND_SAMPLE		*posSample;

	bool load ( bool normalSample = true )
	{
		//if (stdSample)

		//stdSample = FSOUND_Sample_Load();
	}
}trSoundSampleRecord;

typedef std::map<tSoundID,trSoundSampleRecord>	tmSoundIDSampleMap;
typedef std::map<std::string,tSoundID>			tmSoundNameIDMap;

typedef struct 
{
	SoundEvent			*event;
	float				pos;
}trSoundEventRecord;

typedef std::vector<trSoundEventRecord>			tmSoundEventList;

typedef struct 
{
	trSoundSampleRecord	&sample;
	tSoundID			sampleID;
	bool				loop;
	tmSoundEventList	terminateEvents;
	tmSoundEventList	loopEvents;
	tmSoundEventList	positionEvents;
	bool				positional;
	float				pos[3];
	float				vel[3];
	float				volume;
	float				pan;
	bool				mute;
}trSoundInstanceRecord;

typedef std::map<int,trSoundInstanceRecord>		tmSoundInstanceMap;

// stream info
struct trSoundStreamRecord
{
	trSoundStreamRecord()
	{
		stream = NULL;
		refCount = 0;
	}

	COSFile	file;
	std::string name;
	int			refCount;
	FSOUND_STREAM	*stream;
	bool load ( void )
	{
		if (stream)
			return true;
		
		stream = FSOUND_Stream_Open(file.GetFullOSPath(),0,0,0);

		return stream != NULL;
	}

	FSOUND_STREAM* getFModStream ( void )
	{
		if (!stream)
			load();

		refCount++;
		return stream;
	}

	void flushFModStream ( void )
	{
		if (!stream)
			return;

		refCount--;
		if (refCount < 1)
		{
			FSOUND_Stream_Close(stream);
			stream = NULL;
			refCount = 0;
		}
	}
};

typedef std::map<tStreamID,trSoundStreamRecord>		tmSoundIDStreamMap;
typedef std::map<std::string,tStreamID>				tmStreamNameMap;

typedef struct
{
	tStreamID	stream;
	bool		playing;
	bool		pause;
	bool		loop;
	bool		isNet;
	int			channel;
	int			refCount;
	void init ( void )
	{
		stream = 0;
		playing = false;
		pause = false;
		loop = false;
		isNet = false;
		channel = -1;
		refCount = 0;
	}
}trStreamInstanceRecord;

typedef std::map<tStreamInstnace,trStreamInstanceRecord>	tmStreamInstanceMap;

typedef std::vector<tStreamID>						tvPlayList;

typedef struct 
{
	bool random;
	bool loop;
	bool paused;
	int playlistPos;
	tStreamID stream;
	tStreamInstnace	instance;
	void init ( void )
	{
		random = false;
		loop = false;
		paused = false;
		playlistPos = -1;
		stream = 0;
		instance = 0;
	}
}trPlayListInfo;

class SoundManager::SoundManagerInfo
{
public:
	SoundManagerInfo();
	bool	inited;
	tSoundID	lastID;
	tStreamID	lastStreamID;
	int			lastInstance;
	int			lastStreamInstance;
	tmSoundIDSampleMap	samples;
	tmSoundNameIDMap	sampleNames;
	tmSoundInstanceMap	sountInstances;
	tmSoundIDStreamMap	streams;
	tmStreamNameMap		streamNames;
	tmStreamInstanceMap	streamInstances;
	tvPlayList			playlist;
	trPlayListInfo		playListInfo;

	trStreamInstanceRecord* getStreamInstance ( tStreamInstnace instance )
	{
		tmStreamInstanceMap::iterator itr = streamInstances.find(instance);
		if ( itr == streamInstances.end() )
			return NULL;
		return &(itr->second);
	}

	bool killStreamInstance ( tStreamInstnace instance )
	{
		tmStreamInstanceMap::iterator itr = streamInstances.find(instance);
		if ( itr == streamInstances.end() )
			return FALSE;

		streamInstances.erase(itr);
		return true;
	}

	tStreamInstnace newStreamInstance ( trStreamInstanceRecord &instance )
	{
		streamInstances[++lastStreamInstance] = instance;
		return lastStreamInstance;
	}
};

SoundManager::SoundManagerInfo::SoundManagerInfo()
{
	inited = false;
	lastID = 0;
	lastStreamID = 0;
	lastInstance = -1;
	lastStreamInstance = -1;
	playListInfo.init();
}

// main sound manager----------------------------------------------------------------------------------------

SoundManager::SoundManager()
{
	info = new SoundManagerInfo;
}

SoundManager::~SoundManager()
{
	delete(info);
}

bool SoundManager::init ( trSoundManagerInit &initStruct )
{
	if (info->inited)
		return false;

	initParams = initStruct;

	int outputtype;
	int altOutputtype;
	switch (initParams.outputType)
	{
		case eSoundAutoDetect:
			outputtype = altOutputtype = -1;
			break;
		case eSoundNone:
			outputtype = FSOUND_OUTPUT_NOSOUND;
			altOutputtype = FSOUND_OUTPUT_NOSOUND;
			break;
#ifdef _WIN32
			case eSoundPrimary:
				outputtype = FSOUND_OUTPUT_DSOUND;
				altOutputtype = FSOUND_OUTPUT_ASIO;
				break;
			case eSoundBasic:
				outputtype = FSOUND_OUTPUT_WINMM;
				altOutputtype = FSOUND_OUTPUT_DSOUND;
				break;
			case eSoundAlternate:
				outputtype = FSOUND_OUTPUT_ASIO;
				altOutputtype = FSOUND_OUTPUT_ASIO;
				break;
#else
	#ifdef __APPLE__
				case eSoundPrimary:
				case eSoundBasic:
				case eSoundAlternate:
					outputtype = altOutputtype = FSOUND_OUTPUT_MAC;
					break;
	#else	// linux
				case eSoundPrimary:
					outputtype = FSOUND_OUTPUT_DSOUND;
					altOutputtype = FSOUND_OUTPUT_ASIO;
					break;
				case eSoundBasic:
					outputtype = FSOUND_OUTPUT_WINMM;
					altOutputtype = FSOUND_OUTPUT_DSOUND;
					break;
				case eSoundAlternate:
					outputtype = FSOUND_OUTPUT_ASIO;
					altOutputtype = FSOUND_OUTPUT_ASIO;
					break;
	#endif// apple
#endif// win 32
	}

	if (!FSOUND_SetOutput(initParams.tryAlternateOutput ? altOutputtype : outputtype))
		return false;

	if (!FSOUND_SetDriver(initParams.preferedDriver))
		return false;

	if ( initParams.OSWindow != NULL )
		FSOUND_SetHWND(initParams.OSWindow);

	if ( initParams.minHardwareChans != -1 )
		FSOUND_SetMinHardwareChannels(initParams.minHardwareChans);

	int sampleRate = 44100;

	switch(initParams.sampleRate)
	{
		case eSound8khz:				//8000 TotalCrap Quality
			sampleRate  =  8000;
			break;
		case eSound11khz:			//11025 Phone Quality
			sampleRate  =  11025;
			break;
		case eSound22khz:			//22050 Radio quality
			sampleRate  =  22050;
			break;
		case eSound44khz:				//44100 CD quality
			sampleRate  =  44100;
			break;
	}

	if (!FSOUND_Init(sampleRate,initParams.maxChannels,0))
	{
		if (initParams.tryAlternateOutput)
		{
			if (!FSOUND_SetOutput(outputtype))
				return false;

			if (!FSOUND_Init(sampleRate,initParams.maxChannels,0))
				return false;
		}
		else
			return false;
	}
	info->inited = true;

	return true;
}

void SoundManager::kill ( void )
{	
	if (!info->inited)
		return;
	FSOUND_Close();

	info->inited = false;
}

void SoundManager::update ( void )
{
	if (!info->inited)
		return;
	FSOUND_Update();
}

int SoundManager::loadSounds ( std::string &dir, bool recursive )
{
	COSDir	directory(dir.c_str());
	COSFile	file;

	int count = 0;
	while (directory.GetNextFile(file,"*.*",recursive))
	{
		std::string soundName = std::string(file.GetStdName());
		if (loadSound(soundName) != 0)
			count++;
	}
	return count;
}

tSoundID SoundManager::loadSound ( std::string &file )
{
	COSFile	soundFile(file.c_str());

	if (!validSoundExtenstion(soundFile.GetExtension()))
		return 0;
	if (soundFile.GetFileTitle() && soundFile.GetFileTitle()[0] == '.')
		return 0;

	tSoundID soundID = ++info->lastID;

	trSoundSampleRecord	sample;
	
	sample.name = getSampleName(soundFile);
	sample.file = soundFile;
	sample.posSample = NULL;
	sample.stdSample = NULL;
	sample.refCount = 0;

	info->samples[soundID] = sample;
	info->sampleNames[sample.name] = soundID;

	return soundID;
}

tSoundID SoundManager::getSoundID ( std::string &name )
{
	tmSoundNameIDMap::iterator itr = info->sampleNames.find(getSampleName(name));
	if (itr == info->sampleNames.end())
		return 0;
	return itr->second;
}

tSoundID SoundManager::getSoundID ( const char *name )
{
	tmSoundNameIDMap::iterator itr = info->sampleNames.find(getSampleName(std::string(name)));
	if (itr == info->sampleNames.end())
		return 0;
	return itr->second;
}

tSoundInstnace SoundManager::playSample ( tSoundID	sound,  bool startPaused, bool loop)
{
	return 0;
}

tSoundInstnace SoundManager::playSample3d ( tSoundID	sound, float *pos, float	*vel,  bool startPaused, bool loop )
{
	return 0;
}

void SoundManager::stop ( tSoundID	sound )
{
}

void SoundManager::volume ( tSoundInstnace sound, float vol )
{
}

float SoundManager::volume ( tSoundInstnace sound )
{
	return 0;
}

void SoundManager::mute ( tSoundInstnace sound )
{
}

void SoundManager::unmute ( tSoundInstnace sound )
{
}

void SoundManager::pan ( tSoundInstnace sound, float panVal )
{
}

float SoundManager::pan ( tSoundInstnace sound )
{
	return 0;
}

void SoundManager::pause ( tSoundInstnace sound )
{
}

void SoundManager::resume( tSoundInstnace sound )
{
}

void SoundManager::loop( tSoundInstnace sound )
{
}

void SoundManager::unloop( tSoundInstnace sound )
{
}

void SoundManager::set3dPos ( tSoundInstnace sound, float *pos )
{
}

void SoundManager::set3dVel ( tSoundInstnace sound, float *vel )
{
}

void SoundManager::set3dDist ( tSoundInstnace sound, float minDist, float maxDist )
{
}

bool SoundManager::playing ( tSoundInstnace sound )
{
	return false;
}

bool SoundManager::looped ( tSoundInstnace sound )
{
	return false;
}

void SoundManager::terminateEvent ( tSoundInstnace sound, SoundEvent *eventHandaler )
{
}

void SoundManager::loopEvent ( tSoundInstnace sound, SoundEvent *eventHandaler )
{
}

void SoundManager::positionEvent ( tSoundInstnace sound, SoundEvent *eventHandaler, float pos )
{
}

void SoundManager::setMasterVolume ( float vol )
{
	if (vol >1)
		vol = 1.0f;
	else if (vol <0)
		vol = 0;


	FSOUND_SetSFXMasterVolume(vol*255);
}

float SoundManager::getMasterVolume ( void )
{
	return 0;
}

void SoundManager::set3dDopler ( float scale )
{
}

void SoundManager::set3dDistance ( float scale )
{
}

void SoundManager::set3dFalloff ( float scale )
{
}

void SoundManager::set3dListener ( tr3dSoundListener	&listener )
{
}

int SoundManager::loadFileStreams ( std::string &dir, bool recursive )
{
	COSDir	directory(dir.c_str());
	COSFile	file;

	int count = 0;
	while (directory.GetNextFile(file,"*.*",recursive))
	{
		if (loadStream(std::string(file.GetStdName())) != 0)
			count++;
	}
	return count;
}

tStreamID SoundManager::loadStream ( std::string &file )
{
	COSFile theFile(file.c_str());
	
	if (!validSoundExtenstion(theFile.GetExtension()))
		return 0;

	std::string streamName = getSampleName(theFile);
	if ( info->streamNames.find(streamName) != info->streamNames.end())
		return info->streamNames.find(streamName)->second;

	trSoundStreamRecord	stream;
	stream.file = theFile;
	stream.name = getSampleName(theFile,false);

	tStreamID streamID = ++info->lastStreamID;
	
	info->streams[streamID] = stream;
	info->streamNames[streamName] = streamID;

	return streamID;
}

tStreamID SoundManager::getStreamID ( std::string &name )
{
	tmStreamNameMap::iterator itr = info->streamNames.find(getSampleName(name));
	if ( itr == info->streamNames.end())
		return 0;

	return itr->second;
}

tStreamID SoundManager::getStreamID ( const char *name )
{
	if (!name)
		return 0;
	return getStreamID(std::string(name));
}

std::vector<std::string> SoundManager::getPlaylist ( void )
{
	std::vector<std::string> temp;

	tvPlayList::iterator itr = info->playlist.begin();

	while ( itr != info->playlist.end())
		temp.push_back(info->streams[*itr++].name);
	return temp;
}

void SoundManager::setPlaylist ( std::vector<std::string> playlist )
{

	std::vector<std::string>::iterator itr = playlist.begin();
	 while (itr != playlist.end())
		 loadStream(*itr++);
}

int SoundManager::findStream ( std::string stream )
{
	return findStream(getStreamID(stream));
}

int SoundManager::findStream ( tStreamID stream )
{
	if (stream == 0)
		return -1;

	tvPlayList::iterator itr = info->playlist.begin();
	int count = 0;
	while(itr != info->playlist.end())
	{
		if (*itr == stream)
			return count;
		count++;
		itr++;
	}
	return -1;
}

void SoundManager::randomPlay ( bool rand )
{
	info->playListInfo.random = rand;
}

bool SoundManager::randomPlay ( void )
{
	return info->playListInfo.random;
}


void SoundManager::loopPlay ( bool loop )
{	
	info->playListInfo.loop = loop;
}

bool SoundManager::loopPlay ( void )
{
	return info->playListInfo.loop;
}

tStreamInstnace SoundManager::play ( void )
{
	if (info->playListInfo.paused)
		unpause();

	if ( info->playListInfo.stream )
		return info->playListInfo.instance;

	int posToPlay = info->playListInfo.playlistPos;
	if ( info->playListInfo.random )
		posToPlay = rand()%(int)info->playlist.size();

	info->playListInfo.playlistPos = posToPlay;
	trStreamInstanceRecord instance;
	instance.init();

	instance.stream = info->playlist[posToPlay];
	instance.refCount++;

	instance.channel = FSOUND_Stream_Play(FSOUND_FREE,info->streams[instance.stream].getFModStream());
	if ( instance.channel == -1 )
		return 0;

	instance.playing = true;
	info->playListInfo.paused = false;
	info->playListInfo.instance = info->newStreamInstance(instance);

	return info->playListInfo.instance;
}

void SoundManager::play ( tStreamID stream )
{
	if (stream == 0)
		return;

	stop();

	info->playListInfo.playlistPos = -1;

	trStreamInstanceRecord instance;
	instance.init();

	instance.stream = stream;
	instance.refCount++;

	instance.channel = FSOUND_Stream_Play(FSOUND_FREE,info->streams[instance.stream].getFModStream());
	if ( instance.channel == -1 )
		return ;

	instance.playing = true;
	info->playListInfo.instance = info->newStreamInstance(instance);
	info->playListInfo.paused = false;
}

void SoundManager::next ( void )
{
	stop();

	int posToPlay = ++info->playListInfo.playlistPos;
	if ( info->playListInfo.random )
		posToPlay = rand()%(int)info->playlist.size();
	else if (posToPlay > (int)info->playlist.size())
		posToPlay = 0;

	info->playListInfo.playlistPos = posToPlay;
	trStreamInstanceRecord instance;
	instance.init();

	instance.stream = info->playlist[posToPlay];
	instance.refCount++;

	instance.channel = FSOUND_Stream_Play(FSOUND_FREE,info->streams[instance.stream].getFModStream());
	if ( instance.channel == -1 )
		return;

	instance.playing = true;
	info->playListInfo.instance = info->newStreamInstance(instance);
	info->playListInfo.paused = false;
}	

void SoundManager::prev ( void )
{
	stop();

	int posToPlay = --info->playListInfo.playlistPos;
	if ( info->playListInfo.random )
		posToPlay = rand()%(int)info->playlist.size();
	else if ( posToPlay < 0 )
		posToPlay = (int)info->playlist.size()+1;

	info->playListInfo.playlistPos = posToPlay;
	trStreamInstanceRecord instance;
	instance.init();

	instance.stream = info->playlist[posToPlay];
	instance.refCount++;

	instance.channel = FSOUND_Stream_Play(FSOUND_FREE,info->streams[instance.stream].getFModStream());
	if ( instance.channel == -1 )
		return;

	instance.playing = true;
	info->playListInfo.instance = info->newStreamInstance(instance);
	info->playListInfo.paused = false;
}

void SoundManager::stop ( void )
{
	if ( !info->playListInfo.stream || info->playListInfo.instance < 0)
		return;

	FSOUND_Stream_Stop(info->streams[info->playListInfo.stream].stream);
	info->streams[info->playListInfo.stream].flushFModStream();
	info->killStreamInstance(info->playListInfo.instance);
	info->playListInfo.instance = -1;
	info->playListInfo.paused = false;
}

void SoundManager::pause ( void )
{
	if ( info->playListInfo.paused )
		return;

	if ( !info->playListInfo.stream || info->playListInfo.instance < 0)
		return;

	FSOUND_Stream_Stop(info->streams[info->playListInfo.stream].stream);

	info->playListInfo.paused = true;
}

void SoundManager::unpause ( void )
{
	if ( !info->playListInfo.paused )
		return;

	if ( !info->playListInfo.stream || info->playListInfo.instance < 0)
		return;

	FSOUND_Stream_Play(info->getStreamInstance(info->playListInfo.instance)->channel,info->streams[info->playListInfo.stream].stream);

	info->playListInfo.paused = false;
}

void SoundManager::mute ( void )
{
}

void SoundManager::unmute ( void )
{
}

void SoundManager::volume ( float vol )
{
	if ( !info->playListInfo.stream || info->playListInfo.instance < 0)
		return;

	if (vol >1)
		vol = 1.0f;
	else if (vol <0)
		vol = 0;
	FSOUND_SetVolume(info->getStreamInstance(info->playListInfo.instance)->channel,(255.0f * vol ));
}

float SoundManager::volume ( void )
{
	if ( !info->playListInfo.stream || info->playListInfo.instance < 0)
		return 0;

	int vol = FSOUND_GetVolume(info->getStreamInstance(info->playListInfo.instance)->channel);
	return (float)vol/255.0f;
}

void SoundManager::pan ( float panVal )
{
}

float SoundManager::pan ( void )
{
	return 0;
}

bool SoundManager::playing ( void )
{
	return false;
}

bool SoundManager::looped ( void )
{
	return false;
}

float SoundManager::length ( void )
{
	return 0;
}

float SoundManager::position ( void )
{
	return 0;
}

std::string SoundManager::currentStreamName ( void )
{
	return std::string("");
}

void SoundManager::streamChangeEvent ( StreamEvent *eventHandaler )
{
}

int SoundManager::getNumCDDATracks ( char drive )
{
	return 0;
}

std::string SoundManager::getCDDBKey ( char drive , int track )
{
	return std::string("");
}

void SoundManager::openCDDATray ( char drive  )
{
}

void SoundManager::closeCDDATray ( char drive )
{
}

int SoundManager::getNumDrivers ( void )
{
	return 0;
}

std::string SoundManager::getDriverName ( int driver )
{
	return std::string("");
}

int SoundManager::getDriver ( void )
{
	return 0;
}

teSoundOuputType	SoundManager::getOutputType ( void )
{
	return eSoundPrimary;
}

int SoundManager::getMaxChannels ( void )
{
	return 0;
}

int SoundManager::getChannelsPlaying ( void )
{
	return 0;
}


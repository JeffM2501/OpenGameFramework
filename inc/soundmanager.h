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

// TODO
// make CD playing pull track name from CDDB
// allow for multiple streams
// hook up a recording stream for voice chat
// send the recoding stream out over the network
// receve and mix multiple network recorded streams
// compress said streams ( simple zlib may be enough )
// allow for DSP effects on a per channel basis ( echo, dopler, etc.. )

#ifndef _SOUND_MANAGER_
#define _SOUND_MANAGER_

#include <vector>
#include <string>

class SoundManager;

// modes for the selection of the sound systems backend output system
// the primary mode should work fine for most everyone.
// basic should produce sound output on all sysytems, but will not take advantage of hardware features such as 3d sound, or mix properly with other OS sounds.
// alternate mode can produce better output then primary, but may not be supported in older hardware, or all operating system installs.
// *note, on the macintosh there is only one sound output method, all choices map to the same.
// no sound mode causes the API to function but will not initalise or play any sounds.

typedef enum
{
	eSoundNone = 0,		// Dummy mode for no sound output
	eSoundPrimary,		// Win32: Direct sound (DSOUND), Linux: Open Sound System ( OSS ), OSX: Mac SoundManager
	eSoundBasic,			// Win32: Windows Multimedia(WINMM), Linux: Enlightened Sound Daemon ( ESD ), OSX: Mac SoundManager
	eSoundAlternate,	// Win32: Low latency ASIO ( ASIO), Linux: Advanced Linux Sound Architecture (ALSA), OSX: Mac SoundManager  
	eSoundAutoDetect	// default
}teSoundOuputType;

typedef enum
{
	eSound8khz,				//8000 TotalCrap Quality
	eSound11khz,			//11025 Phone Quality
	eSound22khz,			//22050 Radio quality
	eSound44khz				//44100 CD quality
}teSoundSampleRate;

// Surround sound modes will only reliably work with eSoundPrimary on windows (DSOUND).
// Other OSs will most likely default to sterio unless mono is chosen, due to lack of hardware driver support on other OSs
// For the encoded modes ( dolby digital and DTS ) a sound card that supports encoding and a decoding recever are required
typedef enum
{
	eSterio = 0,			// normal PC speakers using 2 speakers, posibly with a sub.. ** DEFAULT
	eHeadphones,			// normal 2 speaker headphpones
	eMono,						// single speaker, or monural output
	eSurround,				// surrournd sound system using 5 or more speakers
	eDolbyDigital,		// dolby digital 5.1 encoded surround sound system
	eQuad,						// 4 speaker quadraphonic surround sound system
	eDTS							// DTS encoded surround sound system
}teSoundSpeakerType;

typedef struct trSoundManagerInit
{
	trSoundManagerInit();
	teSoundOuputType		outputType;					// output mode ( defaults to eSoundPrimary )
	bool								tryAlternateOutput;	// try the alternate output type first ( defaults to true )
	teSoundSpeakerType	speakerType;				// speaker mode ( defaults to eSterio );
	int									soundBufferSize;		// amount of ram to use for mixing sounds ( 2 megs is default )
	int									streamBufferSize;		// amount of ram to use for streaming sounds ( 1 meg default )
	void								*OSWindow;					// the handle to the OS window ( WIN32, direct sound )
	int									minHardwareChans;		// the minimum required hardware 3d sounds needed ( will default to software if needed )
	int									maxChannels;				// the total maximum number of sounds that will play at any one time ( default to 32 )
	int									preferedDriver;			// the prefred sound card ( default is -1 for system default card )
	std::string					networkStreamProxy;	// URL to proxy server to use for network streaming operations ( default to none )
	teSoundSampleRate		sampleRate;					// sample rate for the end mix ( defaults to 44khz )
}trSoundManagerInit;

// typedefs for returned sounds
typedef int tSoundInstnace;
typedef unsigned int tSoundID;

// typedefs for returned streams
typedef int tStreamInstnace;
typedef unsigned int tStreamID;

// event callback classes
class SoundEvent
{
public:
	virtual ~SoundEvent(){return;}
	virtual bool event ( SoundManager &soundmanager, tSoundInstnace instnace, float pos, bool loop ) = 0;
};

class StreamEvent
{
public:
	virtual ~StreamEvent(){return;}
	virtual bool event ( SoundManager &soundmanager, tStreamID stream, std::string &streamname ) = 0;
};

// main sound manager
// TODO, singleton?
class SoundManager
{
public:
	SoundManager();
	~SoundManager();

	// general----------------------------------------------------------------------------------------
	// inits the sound system with the specified options
	bool init ( trSoundManagerInit &initStruct );

	// stop all playback, unload all sounds and streams, and shutdown/release the sound system
	void kill ( void );

	// give the sound manager some time to update it's stuff, call this every so often ( once a frame is best );
	void update ( void );

	// sounds------------------------------------------------------------------------------------------
	// sounds are sound effects
	// they are played multiple times, they are short, they can be 2d or 3d sounds.
	// they are all mixed together, they allways come from files, and they are loaded in ram

	// loads all the sound files found in a directory MP3, OGG, WAV, recursivly if specified
	// returns the number of sounds found
	int loadSounds ( std::string &dir, bool recursive = true );

	// loads the specified sound
	tSoundID loadSound ( std::string &file );

	// for any given sound name return it's sound ID. Sound Names are the path to the sound from the scan dir sans extension
	tSoundID getSoundID ( std::string &name );
	tSoundID getSoundID ( const char *name );

	// play the specified sound 2d
	tSoundInstnace playSample ( tSoundID	sound,  bool startPaused = false, bool loop = false );

	// play the specified sound as 3d using the pos and vel
	tSoundInstnace playSample3d ( tSoundID	sound, float *pos, float	*vel,  bool startPaused = false, bool loop = false );

	// do stuff to a playing sound instnace
	void stop ( tSoundID	sound );
	void volume ( tSoundInstnace sound, float vol );
	float volume ( tSoundInstnace sound );
	void mute ( tSoundInstnace sound );
	void unmute ( tSoundInstnace sound );
	void pan ( tSoundInstnace sound, float panVal );
	float pan ( tSoundInstnace sound );
	void pause ( tSoundInstnace sound );
	void resume( tSoundInstnace sound );
	void loop( tSoundInstnace sound );
	void unloop( tSoundInstnace sound );
	void set3dPos ( tSoundInstnace sound, float *pos );
	void set3dVel ( tSoundInstnace sound, float *vel );
	void set3dDist ( tSoundInstnace sound, float minDist, float maxDist );
	bool playing ( tSoundInstnace sound );
	bool looped ( tSoundInstnace sound );

	// callback classes called when events happen in the sound instance
	// called when the sound ends
	void terminateEvent ( tSoundInstnace sound, SoundEvent *eventHandaler );

	// called when the sound loops
	void loopEvent ( tSoundInstnace sound, SoundEvent *eventHandaler );

	// called when the sound hits the specified postion ( from 0 to 1 )
	void positionEvent ( tSoundInstnace sound, SoundEvent *eventHandaler, float pos );

	// sound system controll
	void setMasterVolume ( float vol );
	float getMasterVolume ( void );

	// global 3d params
	void set3dDopler ( float scale );
	void set3dDistance ( float scale );
	void set3dFalloff ( float scale );

	// the 3space description of a listener
	typedef struct 
	{
		float		pos[3];
		float		vel[3];
		float		forward[3];
		float		up[3];
	}tr3dSoundListener;

	// tell us where the current listener is in 3space
	void set3dListener ( tr3dSoundListener	&listener );

	// streams----------------------------------------------------------------------------------------------------
	// streams are music tracks
	// they are played one at a time, they are long, they are 2d sound
	// they are streamed from the source and not loaded in ram
	// they can come from files, URLs, or Audio CDs

	// loads any sounds found in the folder, MP3, OGG, WAV
	int loadFileStreams ( std::string &dir, bool recursive = true );

	// loads the specified stream
	// files are loaded from disk ( mp2, mp3, ogg, raw, and wav )
	// streams that begin with http:// are pulled from the internet ( mp3 and ogg only )
	// streams that begin with cdda:// are streamed from the cdrom drive specified ( 0 for default ) and track
	// cd example "cdda://0/5" play the 5th track on the default CDRom Drive.
	tStreamID loadStream ( std::string &file );

	// get the ID of the specified stream
	tStreamID getStreamID ( std::string &name );
	tStreamID getStreamID ( const char *name );

	// return the current list of streams in the order they will play
	std::vector<std::string> getPlaylist ( void );

	// set the order of play for the streamlist
	// streams not loaded will be loaded
	// loaded streams not specified will be unloaded
	void setPlaylist ( std::vector<std::string> playlist );

	// find the first instance of a stream in the playlist
	int findStream ( std::string stream );
	int findStream ( tStreamID stream );

	// raondomize the playlist ( in this mode order in the playlist does not mater, will not play the same song back to back, even if looped )
	void randomPlay ( bool rand );
	bool randomPlay ( void );

	// loop the playlist
	void loopPlay ( bool loop );
	bool loopPlay ( void );

	// play from current playlist pos
	tStreamInstnace play ( void );

	// play from specified playlist pos ( first instance in playlist );
	void play ( tStreamID stream );

	// playlist skiping
	void next ( void );
	void prev ( void );

	// do stuff to a playing stream
	void stop ( void );
	void pause ( void );
	void unpause ( void );
	void mute ( void );
	void unmute ( void );

	void volume ( float vol );
	float volume ( void );

	void pan ( float panVal );
	float pan ( void );

	bool playing ( void );
	bool looped ( void );

	// returns the current stream length in seconds
	float length ( void );
	
	// returns a value between 0 and 1 represting where the current playing stream is
	float position ( void );

	// the the name of the current stream playing ( if stream has an ID tag with a name, then the name will come from that ).
	std::string currentStreamName ( void );

	// event callback to be called when streams change during playlist
	// helpfull for trackname display
	void streamChangeEvent ( StreamEvent *eventHandaler );

	// cdrom utilities--------------------------------------------------------------------------------
	// drive is a character representing the drive letter of the CDROm on windows, or the drive number on linux.
	// the number '0' is used to indicate the default CDDA device.
	// used to help the calling app build up it's playlist
	int getNumCDDATracks ( char drive = '0' ); // '0' for default

	// generates a cddb key for a track query
	std::string getCDDBKey ( char drive = '0', int track = 0);

	// open/close the CD tray
	void openCDDATray ( char drive = '0'  );
	void closeCDDATray ( char drive = '0' );

	// global sound system info------------------------------------------------------------------------

	// return the number of soundcard drivers on the system
	int getNumDrivers ( void );

	// get the name of the specified sound driver
	std::string getDriverName ( int driver );

	// get the ID of the driver currently in use
	int getDriver ( void );

	// get the output mode currently in use
	teSoundOuputType	getOutputType ( void );

	// get the max number of playing sounds we ended up with ( counts sound and streams )
	int getMaxChannels ( void );

	// get the current number of sounds playing ( counts sounds and streams ) 
	int getChannelsPlaying ( void );

protected:
	tr3dSoundListener	listener;
	trSoundManagerInit	initParams;

	class SoundManagerInfo;
	SoundManagerInfo		*info;

};

#endif//_SOUND_MANAGER_

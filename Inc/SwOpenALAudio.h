/* ============================================================================
	SwOpenALAudio.h:
	Copyright 2007 Roman Switch` Dzieciol. All Rights Reserved.
============================================================================ */
#ifndef SWOPENALAUDIO_H
#define SWOPENALAUDIO_H


// Includes.
#include "SwOpenAL.h"

/*------------------------------------------------------------------------------------
	Helpers
------------------------------------------------------------------------------------*/

// Utility Macros.
#define safecall(f) \
{ \
	guard(f); \
	INT Error=f; \
	if( Error==0 ) \
		debugf( NAME_Warning, TEXT("%s failed: %i"), TEXT(#f), Error ); \
	unguard; \
}
#define silentcall(f) \
{ \
	guard(f); \
	f; \
	unguard; \
}

inline FString GetALErrorString(ALenum err)
{
	return FString(appFromAnsi(alGetString(err)));
}

inline FString GetALCErrorString(ALenum err)
{
	return FString(appFromAnsi(alcGetString(NULL,err)));
}

inline FromFVector( FVector v, ALfloat* al )
{
	// AL: RHS, X=Right, Y=Up, Z=To
	// UE: ?HS, X=From, Y=Right, Z=Up

	al[0] =  v.Y * 0.02;
	al[1] =  v.Z * 0.02;
	al[2] = -v.X * 0.02;
}

inline FromFRotator( FRotator r, ALfloat* al )
{

	FCoords Coords = GMath.UnitCoords / r;

	ALfloat* at = al;
	ALfloat* up = al + 3;

	FromFVector( Coords.XAxis, at);
	FromFVector( Coords.ZAxis, up);
}


/*inline ToFVector( FVector v, ALfloat al[3] )
{
	v.X = -al[2] * 50.0f;
	v.Y = al[0] * 50.0f;
	v.Z = al[1] * 50.0f;
}*/

class FPlayingSound
{
public:
	ALuint		Source;
	AActor*		Actor;
	INT			Id;
	UBOOL		Is3D;
	USound*		Sound;
	FVector		Location;
	FLOAT		Volume;
	FLOAT		Radius;
	FLOAT		Pitch;
	FLOAT		Priority;

	FPlayingSound()
	:	Source	(0)
	,	Actor	(NULL)
	,	Id		(0)
	,	Is3D	(0)
	,	Sound	(NULL)
	,	Priority(0)
	,	Volume	(0)
	,	Radius	(0)
	,	Pitch	(0)
	{}

	FPlayingSound( ALuint InSource )
	:	Source	(InSource)
	,	Actor	(NULL)
	,	Id		(0)
	,	Is3D	(0)
	,	Sound	(NULL)
	,	Priority(0)
	,	Volume	(0)
	,	Radius	(0)
	,	Pitch	(0)
	{}

	FPlayingSound( AActor* InActor, INT InId, USound* InSound, FVector InLocation, FLOAT InVolume, FLOAT InRadius, FLOAT InPitch, FLOAT InPriority, ALuint InSource )
	:	Source	(InSource)
	,	Actor	(InActor)
	,	Id		(InId)
	,	Is3D    (0)
	,	Sound	(InSound)
	,	Location(InLocation)
	,	Volume	(InVolume)
	,	Radius	(InRadius)
	,	Pitch	(InPitch)
	,	Priority(InPriority)
	{}
};


/*------------------------------------------------------------------------------------
	USwOpenALAudio.
------------------------------------------------------------------------------------*/

class SWOPENAL_API USwOpenALAudio : public UAudioSubsystem
{
	DECLARE_CLASS(USwOpenALAudio,UAudioSubsystem,CLASS_Config)


	// Configuration.
	INT				Channels;
	BYTE			OutputRate;
	BYTE			MusicVolume;
	BYTE			SoundVolume;
	FLOAT			AmbientFactor;
	FLOAT			DopplerSpeed;

	FLOAT			DopplerFactor;
	FLOAT			SpeedOfSound;
	BYTE			DistanceModel;
	FLOAT			RolloffFactor;
	FLOAT			MinDistance;


	BITFIELD		UseNativeHardware;
	BITFIELD		UseGenericHardware;

	INT				MaxChannels;

	/*BITFIELD		UseFilter;
	BITFIELD		UseSurround;
	BITFIELD		UseStereo;
	BITFIELD		UseCDMusic;
	BITFIELD		UseDigitalMusic;
	BITFIELD		ReverseStereo;
	INT				Latency;
	UBOOL			AudioStats;
	UBOOL			DetailStats;
	DWORD			OutputMode;*/

	// Variables.
	BITFIELD		Initialized;
	INT				FreeSlot;
	UViewport*		Viewport;
	/*FPlayingSound	PlayingSounds[MAX_EFFECTS_CHANNELS];
	DOUBLE			LastTime;
	UMusic*			CurrentMusic;
	BYTE			CurrentCDTrack;
	INT				FreeSlot;
	FLOAT			MusicFade;*/

	
	// OpenAL
	ALCdevice*				device;
	ALCcontext*				context;
	TArray<ALuint>			Buffers;
	//TArray<ALuint>		Sources;
	INT						SupportedChannels;
	TArray<FPlayingSound>	Sounds;
	ALfloat					ListenerPos[3];
	ALfloat					ListenerVel[3];
	
	// Constructor.
	USwOpenALAudio();
	void StaticConstructor();

	// UObject interface.
	void Destroy();
	void PostEditChange();
	void ShutdownAfterError();

	// UAudioSubsystem interface.
	virtual UBOOL Init();
	virtual void SetViewport( UViewport* Viewport );
	virtual UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Ar );
	virtual void Update( FPointRegion Region, FCoords& Listener );
	virtual void RegisterMusic( UMusic* Music );
	virtual void RegisterSound( USound* Music );
	virtual void UnregisterSound( USound* Sound );
	virtual void UnregisterMusic( UMusic* Music );
	virtual UBOOL PlaySound( AActor* Actor, INT Id, USound* Sound, FVector Location, FLOAT Volume, FLOAT Radius, FLOAT Pitch );
	virtual void StopSoundId( INT Id );		// DEUS_EX CNN
	virtual void NoteDestroy( AActor* Actor );
	virtual UBOOL GetLowQualitySetting();
	virtual UViewport* GetViewport();
	virtual void RenderAudioGeometry( FSceneNode* Frame );
	virtual void PostRender( FSceneNode* Frame );
	virtual void SetInstantSoundVolume( BYTE newSoundVolume );		// DEUS_EX CNN - instantly sets the sound volume
	virtual void SetInstantSpeechVolume( BYTE newSpeechVolume );		// DEUS_EX CNN - instantly sets the speech volume
	virtual void SetInstantMusicVolume( BYTE newMusicVolume );		// DEUS_EX CNN - instantly sets the music volume



	// USwOpenALAudio internals
	void SetVolumes();
	void StopSound( INT Index );
	void StopAudio();

	ALuint LoadBuffer( USound* Sound )
	{
		if( Sound )
		{
			if( !Sound->Handle )
				RegisterSound( Sound );
		
			if( Sound->Handle )
				return reinterpret_cast<ALuint>(Sound->Handle);
		}
		return NULL;
	}


	ALuint GetBuffer( USound* Sound )
	{
		return reinterpret_cast<ALuint>(Sound->Handle);
	}


	FLOAT SoundPriority( UViewport* Viewport, FVector Location, FLOAT Volume, FLOAT Radius )
	{
		return Volume * (1.0 - (Location - (Viewport->Actor->ViewTarget?Viewport->Actor->ViewTarget:Viewport->Actor)->Location).Size()/Radius);
	}
};


#endif
/* ----------------------------------------------------------------------------
	The End.
---------------------------------------------------------------------------- */
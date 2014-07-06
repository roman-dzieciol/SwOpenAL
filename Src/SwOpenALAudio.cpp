/* ============================================================================
	SwOpenALAudio.cpp:
	Copyright 2007 Roman Switch` Dzieciol. All Rights Reserved.
============================================================================ */

// Includes.
#include "SwOpenALAudio.h"
#include "SwSoundLoader.h"


// Register USwOpenALAudio.
IMPLEMENT_CLASS(USwOpenALAudio);




/*------------------------------------------------------------------------------------
	USwOpenALAudio.
------------------------------------------------------------------------------------*/

USwOpenALAudio::USwOpenALAudio()
{
	swguard(USwOpenALAudio::USwOpenALAudio);

	Initialized = 0;
	FreeSlot = 0;
	Viewport = NULL;

	device = NULL;
	context = NULL;
	
	unguard;
}

void USwOpenALAudio::StaticConstructor()
{
	swguard(USwOpenALAudio::StaticConstructor);

	SW_LOG( SW_NAME, TEXT("%s -- %s :: "), SW_LOGP );

	/*UEnum* OutputRates = new( GetClass(), TEXT("OutputRates") )UEnum( NULL );
		new( OutputRates->Names )FName( TEXT("8000Hz" ) );
		new( OutputRates->Names )FName( TEXT("11025Hz") );
		new( OutputRates->Names )FName( TEXT("16000Hz") );
		new( OutputRates->Names )FName( TEXT("22050Hz") );
		new( OutputRates->Names )FName( TEXT("32000Hz") );
		new( OutputRates->Names )FName( TEXT("44100Hz") );
		new( OutputRates->Names )FName( TEXT("48000Hz") );

	new(GetClass(),TEXT("OutputRate"),      RF_Public)UByteProperty  (CPP_PROPERTY(OutputRate),			TEXT("Audio"), CPF_Config, OutputRates );
	new(GetClass(),TEXT("Channels"), 		RF_Public)UIntProperty   (CPP_PROPERTY(Channels),			TEXT("Audio"), CPF_Config );
	new(GetClass(),TEXT("MusicVolume"),     RF_Public)UByteProperty  (CPP_PROPERTY(MusicVolume),		TEXT("Audio"), CPF_Config );
	new(GetClass(),TEXT("SoundVolume"),     RF_Public)UByteProperty  (CPP_PROPERTY(SoundVolume),		TEXT("Audio"), CPF_Config );
	new(GetClass(),TEXT("AmbientFactor"),   RF_Public)UFloatProperty (CPP_PROPERTY(AmbientFactor),		TEXT("Audio"), CPF_Config );
	new(GetClass(),TEXT("DopplerSpeed"),    RF_Public)UFloatProperty (CPP_PROPERTY(DopplerSpeed),		TEXT("Audio"), CPF_Config );

	new(GetClass(),TEXT("MaxChannels"),     RF_Public)UIntProperty   (CPP_PROPERTY(MaxChannels),	TEXT("Audio"), CPF_Config );*/

	/*new(GetClass(),TEXT("UseFilter"),       RF_Public)UBoolProperty  (CPP_PROPERTY(UseFilter      ), TEXT("Audio"), CPF_Config );
	new(GetClass(),TEXT("UseSurround"),     RF_Public)UBoolProperty  (CPP_PROPERTY(UseSurround    ), TEXT("Audio"), CPF_Config );
	new(GetClass(),TEXT("UseStereo"),       RF_Public)UBoolProperty  (CPP_PROPERTY(UseStereo      ), TEXT("Audio"), CPF_Config );
	new(GetClass(),TEXT("UseCDMusic"),      RF_Public)UBoolProperty  (CPP_PROPERTY(UseCDMusic     ), TEXT("Audio"), CPF_Config );
	new(GetClass(),TEXT("UseDigitalMusic"), RF_Public)UBoolProperty  (CPP_PROPERTY(UseDigitalMusic), TEXT("Audio"), CPF_Config );
	new(GetClass(),TEXT("ReverseStereo"),   RF_Public)UBoolProperty  (CPP_PROPERTY(ReverseStereo  ), TEXT("Audio"), CPF_Config );
	new(GetClass(),TEXT("Latency"),         RF_Public)UIntProperty   (CPP_PROPERTY(Latency        ), TEXT("Audio"), CPF_Config );*/

	// Default values
	OutputRate = 5;
	Channels = 32;
	MusicVolume = 153;
	SoundVolume = 204;
	AmbientFactor = 0.7;
	DopplerSpeed = 17000;

	MaxChannels = 256;

	DopplerFactor = 1.0;
	SpeedOfSound = 343.4;
	DistanceModel = 0;

	unguard;
}


/*------------------------------------------------------------------------------------
	UObject Interface.
------------------------------------------------------------------------------------*/

void USwOpenALAudio::PostEditChange()
{
	swguard(USwOpenALAudio::PostEditChange);

	SW_LOG( SW_NAME, TEXT("%s -- %s :: "), SW_LOGP );

	// Validate configurable variables.
	OutputRate      = Clamp(OutputRate,(BYTE)0,(BYTE)6);
	DopplerSpeed    = Clamp(DopplerSpeed,1.f,100000.f);
	AmbientFactor   = Clamp(AmbientFactor,0.f,10.f);

	SetVolumes();

	unguard;
}

void USwOpenALAudio::Destroy()
{
	swguard(USwOpenALAudio::Destroy);

	SW_LOG( SW_NAME, TEXT("%s -- %s :: "), SW_LOGP );

	if( Initialized )
	{
		// Unhook.
		USound::Audio = NULL;
		UMusic::Audio = NULL;

		// Shut down viewport.
		SetViewport( NULL );

		// Shutdown OpenAL
		if( device != NULL )
		{
			if( context != NULL )
			{
				alcMakeContextCurrent(NULL); 
				alcDestroyContext(context);
			}
			alcCloseDevice(device);
		}

		debugf( NAME_Exit, TEXT("SwOpenAL audio subsystem shut down.") );
	}

	Super::Destroy();

	unguard;
}

void USwOpenALAudio::ShutdownAfterError()
{
	swguard(USwOpenALAudio::ShutdownAfterError);

	SW_LOG( SW_NAME, TEXT("%s -- %s :: "), SW_LOGP );

	// Unhook.
	USound::Audio = NULL;
	UMusic::Audio = NULL;

	// Safely shut down.
	debugf( NAME_Exit, TEXT("USwOpenALAudio::ShutdownAfterError") );

	/*safecall(AudioStopOutput());
	if( Viewport )
		safecall(AudioShutdown());*/

	// Shutdown OpenAL
	if( device != NULL )
	{
		if( context != NULL )
		{
			alcMakeContextCurrent(NULL); 
			alcDestroyContext(context);
		}
		alcCloseDevice(device);
	}

	Super::ShutdownAfterError();
	unguard;
}


/*------------------------------------------------------------------------------------
	UAudioSubsystem Interface.
------------------------------------------------------------------------------------*/

UBOOL USwOpenALAudio::Init()
{
	swguard(USwOpenALAudio::Init);

	// Initialize Open AL
	SW_LOG( SW_NAME, TEXT("%s -- %s :: "), SW_LOGP );



	// Create device & context
	alGetError();

	// get default device info
	const ALCchar* str_defdev = alcGetString(NULL,ALC_DEFAULT_DEVICE_SPECIFIER);
	const ALCchar* str_defcap = alcGetString(NULL,ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER);
	SW_LOG( SW_NAME, TEXT("ALC_DEFAULT_DEVICE_SPECIFIER: %s"), appFromAnsi(str_defdev) );
	SW_LOG( SW_NAME, TEXT("ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER: %s"), appFromAnsi(str_defcap) );

	// open default device
	guard( USwOpenALAudio::Init::CreateDevice );
	//const ALCchar* str_selected = NULL;
	const ALCchar* str_selected = "Generic Software";
	//const ALCchar* str_selected = "Generic Hardware";
	device = alcOpenDevice(str_selected); 
	if( device == NULL ) 
		SW_RETURN_WARNING( FString::Printf(TEXT("Failed to open default device [%s]"), appFromAnsi(str_defdev) ));

	unguard;

	// get device info
	const ALCchar* str_dev = alcGetString(device,ALC_DEVICE_SPECIFIER);
	const ALCchar* str_cap = alcGetString(device,ALC_CAPTURE_DEVICE_SPECIFIER);
	const ALCchar* str_exts = alcGetString(device,ALC_EXTENSIONS);
	SW_LOG( SW_NAME, TEXT("ALC_DEVICE_SPECIFIER: %s"), appFromAnsi(str_dev) );
	SW_LOG( SW_NAME, TEXT("ALC_CAPTURE_DEVICE_SPECIFIER: %s"), appFromAnsi(str_cap) );
	SW_LOG( SW_NAME, TEXT("ALC_EXTENSIONS: %s"), appFromAnsi(str_exts) );

	// create context
	guard( USwOpenALAudio::Init::CreateContext );
	const ALCint ctx_params[] = { ALC_INVALID };
	context = alcCreateContext(device,ctx_params); 
	if( context == NULL ) 
	{
		alcCloseDevice(device);
		SW_RETURN_WARNING( FString::Printf(TEXT("Failed to open context for device [%s]"), appFromAnsi(str_dev) ));
	}

	// set active context
	if( !alcMakeContextCurrent(context) )
		SW_RETURN_WARNING( FString::Printf(TEXT("Failed to activate context for device [%s]"), appFromAnsi(str_dev) ));

	unguard;


	// Context info
	const ALchar* str_ver = alGetString(AL_VERSION);
	const ALchar* str_ven = alGetString(AL_VENDOR);
	const ALchar* str_ren = alGetString(AL_RENDERER);
	const ALchar* str_ext = alGetString(AL_EXTENSIONS);
	SW_LOG( SW_NAME, TEXT("AL_VERSION: %s"), appFromAnsi(str_ver) );
	SW_LOG( SW_NAME, TEXT("AL_VENDOR: %s"), appFromAnsi(str_ven) );
	SW_LOG( SW_NAME, TEXT("AL_RENDERER: %s"), appFromAnsi(str_ren) );
	SW_LOG( SW_NAME, TEXT("AL_EXTENSIONS: %s"), appFromAnsi(str_ext) );

	
	//const ALCchar* str_freq = alcGetString(device,ALC_FREQUENCY);
	//SW_LOG( SW_NAME, TEXT("ALC_FREQUENCY: %s"), appFromAnsi(str_freq) );


	ALCint att_size = 0;
	alcGetIntegerv( device, ALC_ATTRIBUTES_SIZE, 1, &att_size );
	if( att_size > 0 )
	{
		ALCint* att_all = new ALCint[att_size];
		alcGetIntegerv( device, ALC_ALL_ATTRIBUTES, att_size, att_all );

		for( int i=0; i<att_size; ++i )
		{
			if( ++i<att_size )
				SW_LOG( SW_NAME, TEXT("ALC_ALL_ATTRIBUTES[%d]: 0x%.04x = %d"), i-1, att_all[i-1], att_all[i] );
			else
				SW_LOG( SW_NAME, TEXT("ALC_ALL_ATTRIBUTES[%d]: 0x%.04x"), i-1, att_all[i-1] );
		}

		delete []att_all;
	}
	
	ALCint ctx_major;
	ALCint ctx_minor;
	alcGetIntegerv(device,ALC_MAJOR_VERSION,1,&ctx_major);
	alcGetIntegerv(device,ALC_MINOR_VERSION,1,&ctx_minor);
	SW_LOG( SW_NAME, TEXT("ALC_MAJOR_VERSION: %d"), ctx_major );
	SW_LOG( SW_NAME, TEXT("ALC_MINOR_VERSION: %d"), ctx_minor );


	// Clear AL Error Code
	alGetError();




	// Device specific tweaks
	FString str_devtest = FString(appFromAnsi(str_ven)).Caps();
	if( str_devtest.InStr(TEXT("NVIDIA")) != -1 )
	{
		// nvopenal.dll doesn't enforce sources limit - when exceeded goes boom
		MaxChannels = 64; 
	}


	// See how many sources you can create
	guard( USwOpenALAudio::Init::TestChannelCount );
	TArray<ALuint> sources;
	for( SupportedChannels=0; SupportedChannels<MaxChannels; ++SupportedChannels )
	{
		ALuint source;
		alGenSources(1, &source);
		if( alGetError() == AL_NO_ERROR )
			sources.AddItem(source);
		else
			break;
	}

	// Release the Sources
	for( int i=0; i<sources.Num(); ++i )
		alDeleteSources(1, &sources(i));

	// Update channel count
	Channels = Min(Channels,SupportedChannels);
	SW_LOG( SW_NAME, TEXT("Supported Channels: %d"), SupportedChannels );

	// Clear Error Code
	alGetError();
	unguard;


	// Create channels
	guard( USwOpenALAudio::Init::CreateChannels );
	Sounds.Empty();
	for( int i=0; i<Channels; ++i )
	{
		ALuint source;
		alGenSources(1, &source);
		if( alGetError() == AL_NO_ERROR )
		{
			Sounds.AddItem( FPlayingSound(source) );
		}
		else
		{
			Channels = Sounds.Num();
			break;
		}
	}
	
	if( Channels == 0 )
		SW_RETURN_WARNING( FString::Printf(TEXT("Could not create channels for device [%s]"), appFromAnsi(str_dev) ));

	SW_LOG( SW_NAME, TEXT("Active Channels: %d"), Channels );
	unguard;

	//alDistanceModel( AL_NONE );
	//alDistanceModel( AL_INVERSE_DISTANCE_CLAMPED );
	alDistanceModel( AL_LINEAR_DISTANCE );
	//alDistanceModel( AL_EXPONENT_DISTANCE );
	
	/*
#define AL_INVERSE_DISTANCE                       0xD001
#define AL_INVERSE_DISTANCE_CLAMPED               0xD002
#define AL_LINEAR_DISTANCE                        0xD003
#define AL_LINEAR_DISTANCE_CLAMPED                0xD004
#define AL_EXPONENT_DISTANCE                      0xD005
#define AL_EXPONENT_DISTANCE_CLAMPED              0xD006*/

	// Initialized!
	USound::Audio = this;
	UMusic::Audio = this;
	Initialized = 1;

	debugf( NAME_Init, TEXT("SwOpenAL audio subsystem initialized.") );
	return 1;
	unguard;
}

void USwOpenALAudio::StopAudio()
{
	swguard(USwOpenALAudio::StopAudio);

	SW_LOG( SW_NAME, TEXT("%s -- %s ::"), SW_LOGP );

	for( INT i=0; i<Sounds.Num(); ++i )
	{
		// Reset channels
		ALuint source = Sounds(i).Source;
		Sounds(i) = FPlayingSound(source);

		// Clear sources
		if( alIsSource(source) )
		{
			alSourceStop(source);
			alSourcei(source, AL_BUFFER, NULL);
		}
	}

	unguard;
}


void USwOpenALAudio::StopSoundId( INT Id )
{
	swguard(USwOpenALAudio::StopSoundId);

	SW_LOG( SW_NAME, TEXT("%s -- %s :: [%s]"), SW_LOGP, *ToStr(Id) );

	unguard;
}

void USwOpenALAudio::StopSound( INT Index )
{
	swguard(USwOpenALAudio::StopSound);

	SW_LOG( SW_NAME, TEXT("%s -- %s :: [%s]"), SW_LOGP, *ToStr(Index) );

	if( Index < Sounds.Num() )
	{
		// Reset channel
		ALuint source = Sounds(Index).Source;
		Sounds(Index) = FPlayingSound(source);

		// Clear source
		if( alIsSource(source) )
		{
			alSourceStop(source);
			alSourcei(source, AL_BUFFER, NULL);
		}
	}

	unguard;
}



void USwOpenALAudio::SetViewport( UViewport* InViewport )
{
	swguard(USwOpenALAudio::SetViewport);

	SW_LOG( SW_NAME, TEXT("%s -- %s :: [%s]"), SW_LOGP, *ToStr(InViewport) );

	// Stop playing sounds.
	StopAudio();

	// Remember the viewport.
	if( Viewport != InViewport )
	{
		if( Viewport )
		{
			// Unregister music.
			for( TObjectIterator<UMusic> MusicIt; MusicIt; ++MusicIt )
				if( MusicIt->Handle )
					UnregisterMusic( *MusicIt );

			// Shut down.
			// TODO: destroy context?
		}

		Viewport = InViewport;
		if( Viewport )
		{
			// Start music
			if( Viewport->Actor->Song && Viewport->Actor->Transition == MTRAN_None )
				Viewport->Actor->Transition = MTRAN_Instant;

			// Start sound output.
			// TODO: create context?

			// Update volumes
			SetVolumes();
		}
	}

	unguard;
}

UViewport* USwOpenALAudio::GetViewport()
{
	swguard(USwOpenALAudio::GetViewport);
	return Viewport;
	unguard;
}

void USwOpenALAudio::RegisterSound( USound* Sound )
{
	swguard(USwOpenALAudio::RegisterSound);

	SW_LOG( SW_NAME, TEXT("%s -- %s :: [%s]"), SW_LOGP, *ToStr(Sound) );

	if( Sound && !Sound->Handle )
	{
		// Load sound
		FSoundLoader loader;
		if( loader.LoadSound(Sound) )
		{
			// Verify buffer & store it
			ALuint buffer = reinterpret_cast<ALuint>(Sound->Handle);
			if( buffer && alIsBuffer(buffer) )
			{
				Buffers.AddItem( buffer );
			}
		}
	}

	unguard;
}

void USwOpenALAudio::UnregisterSound( USound* Sound )
{
	swguard(USwOpenALAudio::UnregisterSound);

	SW_LOG( SW_NAME, TEXT("%s -- %s :: [%s]"), SW_LOGP, *ToStr(Sound) );


	if( Sound && Sound->Handle )
	{
		debugf( NAME_DevSound, TEXT("Unregister sound: %s"), Sound->GetFullName() );

		// Get buffer
		ALuint buffer = GetBuffer(Sound);
		if( !buffer )
		{
			SW_LOG( NAME_Warning, TEXT("%s -- %s :: Sound wasn't registered [%s]"), SW_LOGP, *ToStr(Sound) );
			return;
		}

		// Clear sound from channels
		for( INT i=0; i<Sounds.Num(); ++i )
		{
			FPlayingSound& playing = Sounds(i);

			if( playing.Sound == Sound )
			{
				ALuint src = playing.Source;

				// Stop playing
				if( alIsSource(src) )
				{
					alSourceStop(src);
					alSourcei(src, AL_BUFFER, NULL);
				}

				// Reset channel variables
				playing = FPlayingSound();
			}
		}

		// TODO: Unload this buffer
	}

	unguard;
}


void USwOpenALAudio::RegisterMusic( UMusic* Music )
{
	swguard(USwOpenALAudio::RegisterMusic);

	SW_LOG( SW_NAME, TEXT("%s -- %s :: [%s]"), SW_LOGP, *ToStr(Music) );

	unguard;
}

void USwOpenALAudio::UnregisterMusic( UMusic* Music )
{
	swguard(USwOpenALAudio::UnregisterMusic);

	SW_LOG( SW_NAME, TEXT("%s -- %s :: [%s]"), SW_LOGP, *ToStr(Music) );

	unguard;
}

UBOOL USwOpenALAudio::PlaySound
(
	AActor*	Actor,
	INT		Id,
	USound*	Sound,
	FVector	Location,
	FLOAT	Volume,
	FLOAT	Radius,
	FLOAT	Pitch
)
{
	swguard(USwOpenALAudio::PlaySound);

FVector v;
_asm int 3;
_asm nop;
USound::Audio->PlaySound( 0, 0, 0, v, 0, 0, 0 );
_asm nop;

	ALenum result;

	SW_LOG( SW_NAME, TEXT("%s -- %s :: [%s],[%s],[%s],[%s],[%s],[%s],[%s]")
		, SW_LOGP, *ToStr(Actor,true), *ToStr(Id), *ToStr(Sound), *ToStr(Location), *ToStr(Volume), *ToStr(Radius), *ToStr(Pitch) );

	if( !Viewport || !Sound || Radius <= 0 )
		return 0;

	// Get buffer
	ALuint buffer = LoadBuffer(Sound);
	if( !buffer )
		SW_RETURN_WARNING( FString::Printf(TEXT("Invalid buffer [%p] for sound [%s]"), buffer, *ToStr(Sound) ));

	// Buffer info
	/*ALint b_freq;
	ALint b_size;
	ALint b_bits;
	ALint b_channels;
	alGetBufferi( buffer,	AL_FREQUENCY,	&b_freq );
	alGetBufferi( buffer,	AL_SIZE,		&b_size );
	alGetBufferi( buffer,	AL_BITS,		&b_bits );
	alGetBufferi( buffer,	AL_CHANNELS,	&b_channels );*/

	
	// Allocate a new slot if requested.
	if( (Id&14) == 2*SLOT_None )
		Id = 16 * --FreeSlot;

	// Compute priority.
	FLOAT Priority = SoundPriority( Viewport, Location, Volume, Radius );

	// If already playing, stop it.
	INT Index = -1;
	FLOAT BestPriority = Priority;
	for( INT i=0; i<Sounds.Num(); i++ )
	{
		FPlayingSound& playing = Sounds(i);

		if( (playing.Id & ~1) == (Id & ~1) )
		{
			// Skip if not interruptable.
			if( Id & 1 )
				return 0;

			// Stop the sound.
			Index = i;
			break;
		}
		else if( playing.Priority <= BestPriority )
		{
			Index = i;
			BestPriority = playing.Priority;
		}
	}
	
	// If no channels, or its priority is overruled, stop it.
	if( Index == -1 )
	{
		SW_RETURN_WARNING( FString::Printf(TEXT("Failed to find channel for sound [%s]"), *ToStr(Sound) ));
		//return 0;
	}


    // Reset channel
	StopSound(Index);
	FPlayingSound& psound = Sounds(Index);
	ALuint src = psound.Source;
	if( !alIsSource(src) )
		SW_RETURN_WARNING( FString::Printf(TEXT("Missing sound source [%s]"), *ToStr(Sound) ));

	// Clear AL errors
	alGetError();

	// Setup channel
	psound = FPlayingSound( Actor, Id, Sound, Location, Volume, Radius, Pitch, Priority, src );

	
	// Init source

	ALboolean src_looping = ((Id & 14) == SLOT_Ambient*2);

    alSourcei (src, AL_BUFFER,		buffer );
    alSourcef (src, AL_PITCH,		1.0f );
    alSourcef (src, AL_GAIN,		1.0f );
    alSourcei (src, AL_LOOPING,		src_looping );

	// Update the sound.
	if( Actor )
	{
		ALfloat SourcePos[3];
		ALfloat SourceVel[3];

		FromFVector( Actor->Location, SourcePos );
		FromFVector( Actor->Velocity, SourceVel );


		alSourcefv(src, AL_POSITION,	SourcePos );
		alSourcefv(src, AL_VELOCITY,	SourceVel );
	}
	else
	{
		alSourcefv(src, AL_POSITION,	ListenerPos );
		alSourcefv(src, AL_VELOCITY,	ListenerVel );
	}

	if( SW_GET_AL_ERROR )
		SW_RETURN_WARNING( FString::Printf(TEXT("Failed to init source [%s] for sound [%s]"), *GetALErrorString(result), *ToStr(Sound) ));



	// Play source
	alSourcePlay(src);
	if( SW_GET_AL_ERROR )
		SW_RETURN_WARNING( FString::Printf(TEXT("Failed to play source [%s] for sound [%s]"), *GetALErrorString(result), *ToStr(Sound) ));


	return 1;
	unguard;
}

void USwOpenALAudio::NoteDestroy( AActor* Actor )
{
	swguard(USwOpenALAudio::NoteDestroy);

	SW_LOG( SW_NAME, TEXT("%s -- %s :: [%s]"), SW_LOGP, *ToStr(Actor) );

	if( Actor && Actor->IsValid() )
	{
		for( INT i=0; i<Sounds.Num(); i++ )
		{
			// Stop this actor's sounds
			if( Sounds(i).Actor == Actor )
			{
				StopSound(i);
			}
			
			/*if( (PlayingSounds[i].Id&14)==SLOT_Ambient*2 )
			{
				// Stop ambient sound when actor dies.
				StopSound( i );
			}
			else
			{
				// Unbind regular sounds from actors.
				PlayingSounds[i].Actor = NULL;
			}*/
		}
	}

	unguard;
}

void USwOpenALAudio::RenderAudioGeometry( FSceneNode* Frame )
{
	swguard(USwOpenALAudio::RenderAudioGeometry);

	//SW_LOG( SW_NAME, TEXT("%s -- %s :: "), SW_LOGP );

	unguard;
}

void USwOpenALAudio::Update( FPointRegion Region, FCoords& Coords )
{
	swguard(USwOpenALAudio::Update);

	//SW_LOG( SW_NAME, TEXT("%s -- %s :: [%s],[%s]"), SW_LOGP, *ToStr(Region), *ToStr(Coords) );

	if( !Viewport )
		return;

	// Clear AL errors
	alGetError();
//	ALenum result;

	
	// Lock
	alcSuspendContext(context);

	// Time passes...
	/*FTime DeltaTime = appSeconds() - LastTime;
	LastTime += DeltaTime;
	DeltaTime = Clamp( DeltaTime, 0.0, 1.0 );*/

	AActor *ViewActor = Viewport->Actor->ViewTarget ? Viewport->Actor->ViewTarget : Viewport->Actor;
	UBOOL Realtime = Viewport->IsRealtime() && Viewport->Actor->Level->Pauser == TEXT("");



	// Update listener

	// Position of the listener.
	// Velocity of the listener.
	// Orientation of the listener. (first 3 elements are "at", second 3 are "up")
	ALfloat ListenerOri[6];

	FromFVector( ViewActor->Location, ListenerPos );
	FromFVector( ViewActor->Velocity, ListenerVel );
	FromFRotator( ViewActor->Rotation, ListenerOri );

    alListenerf(AL_GAIN,    1.0f);
    alListenerfv(AL_POSITION,    ListenerPos);
    alListenerfv(AL_VELOCITY,    ListenerVel);
    alListenerfv(AL_ORIENTATION, ListenerOri);


//	if( SW_GET_AL_ERROR )
//		SW_RETURN_WARNING( FString::Printf(TEXT("Failed to init listener [%s] for sound [%s]"), *GetALErrorString(result), *ToStr(Sound) ));


	// See if any new ambient sounds need to be started.
	if( Realtime )
	{
		swguard(USwOpenALAudio::Update::StartAmbience);
		for( INT i=0; i<Viewport->Actor->GetLevel()->Actors.Num(); ++i )
		{
			AActor* Actor = Viewport->Actor->GetLevel()->Actors(i);
			if(	Actor
			&&	Actor->AmbientSound
			&&	FDistSquared( ViewActor->Location, Actor->Location) <= Square(Actor->WorldSoundRadius()) )
			{
				INT Id = Actor->GetIndex() * 16 + SLOT_Ambient * 2;
				for( INT j=0; j<Sounds.Num(); ++j )
					if( Sounds(j).Id == Id )
						break;

				if( j == Sounds.Num() )
					PlaySound( Actor, Id, Actor->AmbientSound, Actor->Location, AmbientFactor*Actor->SoundVolume/255.0, Actor->WorldSoundRadius(), Actor->SoundPitch/64.0 );
			}
		}
		unguard;
	}


	// Update all playing ambient sounds.
	swguard(UpdateAmbience);
	for( INT i=0; i<Sounds.Num(); i++ )
	{
		FPlayingSound& Playing = Sounds(i);
		if( (Playing.Id&14)==SLOT_Ambient*2 )
		{
			check(Playing.Actor);
			if
			(	FDistSquared(ViewActor->Location,Playing.Actor->Location)>Square(Playing.Actor->WorldSoundRadius())
			||	Playing.Actor->AmbientSound!=Playing.Sound 
			||  !Realtime )
			{
				// Ambient sound went out of range.
				StopSound( i );
			}
			else
			{

				// Update basic sound properties.
				FLOAT Brightness = 2.0 * (AmbientFactor*Playing.Actor->SoundVolume/255.0);
				if( Playing.Actor->LightType!=LT_None )
				{
					FPlane Color;
					Brightness *= Playing.Actor->LightBrightness/255.0;
//					Viewport->GetOuterUClient()->Engine->Render->GlobalLighting( (Viewport->Actor->ShowFlags & SHOW_PlayerCtrl)!=0, Playing.Actor, Brightness, Color );
				}
				Playing.Volume = Brightness;
				Playing.Radius = Playing.Actor->WorldSoundRadius();
				Playing.Pitch  = Playing.Actor->SoundPitch/64.0;

			}
		}
	}
	unguard;


	// Update all active sounds.
	swguard(UpdateSounds);
	for( INT Index=0; Index<Sounds.Num(); Index++ )
	{
		FPlayingSound& Playing = Sounds(Index);
		if( Playing.Actor )
			check(Playing.Actor->IsValid());

		// Get source
		ALuint src = Playing.Source;
		ALint srcstate;
		alGetSourcei( src, AL_SOURCE_STATE, &srcstate );

		if( Playing.Id == 0 )
		{
			// Empty slot
			continue;
		}
		else if( srcstate == AL_STOPPED )
		{
			// Sound is finished.
			StopSound( Index );
		}
		else
		{
			// Update positioning from actor, if available.
			if( Playing.Actor )
				Playing.Location = Playing.Actor->Location;

			// Update the priority.
			Playing.Priority = SoundPriority( Viewport, Playing.Location, Playing.Volume, Playing.Radius );

			FLOAT Dist = FVector( Playing.Location - ViewActor->Location).Size();
			FLOAT Attenuation = Clamp(1.0-Dist/Playing.Radius,0.0,1.0);

#if 0
			// Compute the spatialization.
			FVector Location = Playing.Location.TransformPointBy( Coords );
			FLOAT   PanAngle = appAtan2(Location.X, Abs(Location.Z));

			// Despatialize sounds when you get real close to them.
			FLOAT CenterDist  = 0.1*Playing.Radius;
			FLOAT Size        = Location.Size();
			if( Location.SizeSquared() < Square(CenterDist) )
				PanAngle *= Size / CenterDist;

			// Compute panning and volume.
			INT     SoundPan      = Clamp( (INT)(AUDIO_MAXPAN/2 + PanAngle*AUDIO_MAXPAN*7/8/PI), 0, AUDIO_MAXPAN );
			FLOAT   Attenuation = Clamp(1.0-Size/Playing.Radius,0.0,1.0);
			INT     SoundVolume   = Clamp( (INT)(AUDIO_MAXVOLUME * Playing.Volume * Attenuation * EFFECT_FACTOR), 0, AUDIO_MAXVOLUME );
			if( ReverseStereo )
				SoundPan = AUDIO_MAXPAN - SoundPan;
			if( Location.Z<0.0 && UseSurround )
				SoundPan = AUDIO_MIDPAN | AUDIO_SURPAN;

			// Compute doppler shifting (doesn't account for player's velocity).
			FLOAT Doppler=1.0;
			if( Playing.Actor )
			{
				FLOAT V = (Playing.Actor->Velocity/*-ViewActor->Velocity*/) | (Playing.Actor->Location - ViewActor->Location).SafeNormal();
				Doppler = Clamp( 1.0 - V/DopplerSpeed, 0.5, 2.0 );
			}
#endif
			// Update the sound.
			if( Playing.Actor )
			{
				ALfloat SourcePos[3];
				ALfloat SourceVel[3];

				FromFVector( Playing.Actor->Location, SourcePos );
				FromFVector( Playing.Actor->Velocity, SourceVel );

				alSourcefv(src, AL_POSITION,	SourcePos );
				alSourcefv(src, AL_VELOCITY,	SourceVel );
			}
			else
			{
				alSourcefv(src, AL_POSITION,	ListenerPos );
				alSourcefv(src, AL_VELOCITY,	ListenerVel );
			}

			//ALfloat SourceOr[6];
			//FromFRotator( FRotator(0,0,0), SourceOr );

			alSourcef(src, AL_GAIN, Playing.Volume /* * Attenuation */ );			
			alSourcef(src, AL_MAX_DISTANCE, Playing.Radius*0.02f);
			alSourcef(src, AL_REFERENCE_DISTANCE, 0.0f);
			alSourcef(src, AL_ROLLOFF_FACTOR, 1.0f);
			//alSourcefv(src, AL_DIRECTION, SourceOr);
			//alSourcef(src, AL_CONE_INNER_ANGLE, 30.0f);
			//alSourcef(src, AL_CONE_OUTER_ANGLE, 360.0f);
			//alSourcef(src, AL_CONE_INNER_GAIN, 1.0f);
			//alSourcef(src, AL_CONE_OUTER_GAIN, 0.25);
			//alSourcef(src, AL_PITCH, 0.50f);
			//alSourcef(src, AL_MIN_GAIN, 0.0f);
			//alSourcef(src, AL_MAX_GAIN, 1.0f);

#if 0

			// Update the sound.
			Sample* Sample = GetSound(Playing.Sound);
			FVector Z(0,0,0);
			FVector L(Location.X/400.0,Location.Y/400.0,Location.Z/400.0);

			if( Playing.Channel )
			{
				// Update an existing sound.
				swguard(UpdateSample);
				UpdateSample
				( 
					Playing.Channel,
					(INT) (Sample->SamplesPerSec * Playing.Pitch * Doppler),
					SoundVolume,
					SoundPan
				);
				Playing.Channel->BasePanning = SoundPan;
				unguard;
			}
			else
			{
				// Start this new sound.
				swguard(StartSample);
				if( !Playing.Channel ) 
					Playing.Channel = StartSample
						( Index+1, Sample, 
						  (INT) (Sample->SamplesPerSec * Playing.Pitch * Doppler), 
						  SoundVolume, SoundPan );
				check(Playing.Channel);
				unguard;
			}
#endif
		}
	}
	unguard;


	// Unlock.
	alcProcessContext(context);


	unguard;
}

UBOOL USwOpenALAudio::GetLowQualitySetting()
{
	swguard(USwOpenALAudio::GetLowQualitySetting);

	//SW_LOG( SW_NAME, TEXT("%s -- %s :: "), SW_LOGP );
	return 0;

	unguard;
}







void USwOpenALAudio::SetInstantSoundVolume( BYTE newSoundVolume )
{
	swguard(USwOpenALAudio::SetInstantSoundVolume);

	SW_LOG( SW_NAME, TEXT("%s -- %s :: [%s]"), SW_LOGP, *ToStr(newSoundVolume) );

	unguard;
}
void USwOpenALAudio::SetInstantSpeechVolume( BYTE newSpeechVolume )
{
	swguard(USwOpenALAudio::SetInstantSpeechVolume);

	SW_LOG( SW_NAME, TEXT("%s -- %s :: [%s]"), SW_LOGP, *ToStr(newSpeechVolume) );

	unguard;
}
void USwOpenALAudio::SetInstantMusicVolume( BYTE newMusicVolume )
{
	swguard(USwOpenALAudio::SetInstantMusicVolume);

	SW_LOG( SW_NAME, TEXT("%s -- %s :: [%s]"), SW_LOGP, *ToStr(newMusicVolume) );

	unguard;
}

void USwOpenALAudio::SetVolumes()
{
	swguard(USwOpenALAudio::SetVolumes);

	/*// Normalize the volumes.
	FLOAT NormSoundVolume = SoundVolume/255.0;
	FLOAT NormMusicVolume = Clamp(MusicVolume/255.0,0.0,1.0);

	// Set music and effects volumes.
	verify( SetSampleVolume( 127*NormSoundVolume ) );
	if( UseDigitalMusic )
		verify( SetMusicVolume( 127*NormMusicVolume*Max(MusicFade,0.f) ) );
	if( UseCDMusic )
		SetCDAudioVolume( 127*NormMusicVolume*Max(MusicFade,0.f) );*/

	unguard;
}



void USwOpenALAudio::PostRender( FSceneNode* Frame )
{
	swguard(USwOpenALAudio::PostRender);

	//SW_LOG( SW_NAME, TEXT("%s -- %s :: "), SW_LOGP );

#if 0
	Frame->Viewport->Canvas->Color = FColor(255,255,255);
	if( AudioStats )
	{
		Frame->Viewport->Canvas->CurX=0;
		Frame->Viewport->Canvas->CurY=16;
		Frame->Viewport->Canvas->WrappedPrintf
		(
			Frame->Viewport->Canvas->SmallFont,
			0, TEXT("GenericAudioSubsystem Statistics")
		);
		for (INT i=0; i<Channels; i++)
		{
			if (PlayingSounds[i].Channel)
			{
				INT Factor;
				if (DetailStats)
					Factor = 16;
				else
					Factor = 8;
					
				// Current Sound.
				Frame->Viewport->Canvas->CurX=10;
				Frame->Viewport->Canvas->CurY=24 + Factor*i;
				Frame->Viewport->Canvas->WrappedPrintf
				( Frame->Viewport->Canvas->SmallFont, 0, TEXT("Channel %2i: %s"),
					i, PlayingSounds[i].Sound->GetFullName() );

				if (DetailStats)
				{
					// Play meter.
					VoiceStats CurrentStats;
					GetVoiceStats( &CurrentStats, PlayingSounds[i].Channel );
					Frame->Viewport->Canvas->CurX=10;
					Frame->Viewport->Canvas->CurY=32 + Factor*i;
					Frame->Viewport->Canvas->WrappedPrintf
					( Frame->Viewport->Canvas->SmallFont, 0, TEXT("  [%s] %05.1f\% Vol: %05.2f"),
						*CurrentStats.CompletionString, CurrentStats.Completion*100, PlayingSounds[i].Volume );
				}
			} else {
				INT Factor;
				if (DetailStats)
					Factor = 16;
				else
					Factor = 8;
					
				Frame->Viewport->Canvas->CurX=10;
				Frame->Viewport->Canvas->CurY=24 + Factor*i;
				if (i >= 10)
					Frame->Viewport->Canvas->WrappedPrintf
					( Frame->Viewport->Canvas->SmallFont, 0, TEXT("Channel %i:  None"),
						i );
				else
					Frame->Viewport->Canvas->WrappedPrintf
					( Frame->Viewport->Canvas->SmallFont, 0, TEXT("Channel %i: None"),
						i );

				if (DetailStats)
				{
					// Play meter.
					Frame->Viewport->Canvas->CurX=10;
					Frame->Viewport->Canvas->CurY=32 + Factor*i;
					Frame->Viewport->Canvas->WrappedPrintf
					( Frame->Viewport->Canvas->SmallFont, 0, TEXT("  [----------]") );
				}
			}
		}
	}

#endif

	unguard;
}


UBOOL USwOpenALAudio::Exec( const TCHAR* Cmd, FOutputDevice& Ar )
{
	swguard(USwOpenALAudio::Exec);

	//SW_LOG( SW_NAME, TEXT("%s -- %s :: "), SW_LOGP );

#if 0
	const TCHAR* Str = Cmd;
	if( ParseCommand(&Str,TEXT("ASTAT")) )
	{
		if( ParseCommand(&Str,TEXT("Audio")) )
		{
			AudioStats ^= 1;
			return 1;
		}
		if( ParseCommand(&Str,TEXT("Detail")) )
		{
			DetailStats ^= 1;
			return 1;
		}
	}
#endif

	return 0;
	
	unguard;
}


/* ----------------------------------------------------------------------------
	The End.
---------------------------------------------------------------------------- */
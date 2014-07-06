/* ============================================================================
	SwSoundLoader.h:
	Copyright 2007 Roman Switch` Dzieciol. All Rights Reserved.
============================================================================ */
#ifndef SWSOUNDLOADER_H
#define SWSOUNDLOADER_H


// Includes.
#include "SwOpenAL.h"
#include "SwOpenALAudio.h"


/* ----------------------------------------------------------------------------
	Helpers
---------------------------------------------------------------------------- */


extern FString SwTwoCCToStr( _WORD code );

#define SW_RETURN_WARNING( msg ) { SW_LOG( NAME_Warning, TEXT("%s !! %s :: %s !"), SW_LOGP, *FString(msg) ); return false; }




/* ----------------------------------------------------------------------------
	FWaveHeader
---------------------------------------------------------------------------- */

struct FWaveHeader
{ 
	// Riff header
	DWORD	rID;				// Contains 'RIFF'
	DWORD	rLen;				// Remaining length of the entire riff chunk (= file).
	DWORD	wID;				// Form type. Contains 'WAVE' for .wav files.
	
	// Format chunk
	DWORD	FormatID;			// General data chunk ID like 'data', or 'fmt ' 
	DWORD	FormatSize;			// Length of the rest of this chunk in bytes
	
	// Format
    _WORD   wFormatTag;			// Format type: 1 = PCM
    _WORD   nChannels;			// Number of channels (i.e. mono, stereo...).
    DWORD   nSamplesPerSec;		// Sample rate. 44100 or 22050 or 11025  Hz.
    DWORD   nAvgBytesPerSec;	// For buffer estimation  = sample rate * BlockAlign.
    _WORD   nBlockAlign;		// Block size of data = Channels times BYTES per sample.
    _WORD   wBitsPerSample;		// Number of bits per sample of mono data.
	
	// Data chunk
	DWORD	DataID;				// General data chunk ID like 'data', or 'fmt ' 
	DWORD	DataSize;			// Length of the rest of this chunk in bytes

	// Data
	BYTE	FirstByte;			// First data byte
};






/* ----------------------------------------------------------------------------
	FSoundLoader
---------------------------------------------------------------------------- */

class FSoundLoader
{
public:
	UBOOL LoadSound( USound* Sound );

protected:
	UBOOL ParseWAV( USound* Sound );
	UBOOL ParseMP3( USound* Sound );
};


/*
0         4   ChunkID          Contains the letters "RIFF" in ASCII form
                               (0x52494646 big-endian form).
4         4   ChunkSize        36 + SubChunk2Size, or more precisely:
                               4 + (8 + SubChunk1Size) + (8 + SubChunk2Size)
                               This is the size of the rest of the chunk 
                               following this number.  This is the size of the 
                               entire file in bytes minus 8 bytes for the
                               two fields not included in this count:
                               ChunkID and ChunkSize.
8         4   Format           Contains the letters "WAVE"
                               (0x57415645 big-endian form).

The "WAVE" format consists of two subchunks: "fmt " and "data":
The "fmt " subchunk describes the sound data's format:

12        4   Subchunk1ID      Contains the letters "fmt "
                               (0x666d7420 big-endian form).
16        4   Subchunk1Size    16 for PCM.  This is the size of the
                               rest of the Subchunk which follows this number.
20        2   AudioFormat      PCM = 1 (i.e. Linear quantization)
                               Values other than 1 indicate some 
                               form of compression.
22        2   NumChannels      Mono = 1, Stereo = 2, etc.
24        4   SampleRate       8000, 44100, etc.
28        4   ByteRate         == SampleRate * NumChannels * BitsPerSample/8
32        2   BlockAlign       == NumChannels * BitsPerSample/8
                               The number of bytes for one sample including
                               all channels. I wonder what happens when
                               this number isn't an integer?
34        2   BitsPerSample    8 bits = 8, 16 bits = 16, etc.
          2   ExtraParamSize   if PCM, then doesn't exist
          X   ExtraParams      space for extra parameters

The "data" subchunk contains the size of the data and the actual sound:

36        4   Subchunk2ID      Contains the letters "data"
                               (0x64617461 big-endian form).
40        4   Subchunk2Size    == NumSamples * NumChannels * BitsPerSample/8
                               This is the number of bytes in the data.
                               You can also think of this as the size
                               of the read of the subchunk following this 
                               number.
44        *   Data             The actual sound data.
*/


#endif
/* ----------------------------------------------------------------------------
	The End.
---------------------------------------------------------------------------- */
/* ============================================================================
	SwSoundLoader.cpp:
	Copyright 2007 Roman Switch` Dzieciol. All Rights Reserved.
============================================================================ */

// Includes.
#include "SwSoundLoader.h"


UBOOL FSoundLoader::LoadSound( USound* Sound )
{
	swguard(FSoundLoader::LoadSound);
	
	SW_LOG( SW_NAME, TEXT("%s -- %s :: [%s]"), SW_LOGP, *ToStr(Sound) );

	if( Sound && !Sound->Handle )
	{
		// Set the handle to avoid reentrance.
		//Sound->Handle = SW_INVALID_BUFFER;

		// Identify and load
		if( appStricmp( TEXT("wav"), *Sound->FileType ) == 0 )
		{
			// Load the data.
			Sound->Data.LoadSpecial();
			debugf( NAME_DevSound, TEXT("Register sound: %s (%i)"), Sound->GetPathName(), Sound->Data.Num() );
			check(Sound->Data.Num()>0);

			ParseWAV(Sound);

			// Unload the data.
			Sound->Data.Unload();

		}
		/*else if( appStricmp( TEXT("mp3"), *Sound->FileType ) == 0 )
		{
			// Load the data.
			Sound->Data.Load();
			debugf( NAME_DevSound, TEXT("Register sound: %s (%i)"), Sound->GetPathName(), Sound->Data.Num() );
			check(Sound->Data.Num()>0);

			ParseMP3(Sound);

			// Unload the data.
			Sound->Data.Unload();
		}*/
		else
		{
			SW_LOG( NAME_Warning, TEXT("%s :: Unsupported sound format [%s] in sound [%s] !"), *SwTimeStr(), *Sound->FileType, Sound->GetFullName() );
			return 0;
		}
	}

	return 1;
	
	unguard;
}

UBOOL FSoundLoader::ParseWAV( USound* Sound )
{
	swguard(FSoundLoader::ParseWAV);

	SW_LOG( SW_NAME, TEXT("%s -- %s :: [%s]"), SW_LOGP, *ToStr(Sound) );

    ALenum result;
	alGetError();

	// Parse WAV data

	// Must have at least the header
	if( Sound->Data.Num() < sizeof(FWaveHeader) )
		SW_RETURN_WARNING( FString::Printf(TEXT("Malformed data in sound [%s]"), Sound->GetFullName() ));

	// Parse header
	FWaveHeader* hdr;
	hdr = reinterpret_cast<FWaveHeader*>( Sound->Data.GetData() );
	
	// Format chunk must come before data chunk. other chunks unsupported
	if( hdr->FormatID != 0x20746D66 )
		SW_RETURN_WARNING( FString::Printf(TEXT("Missing fmt chunk in sound [%s]"), Sound->GetFullName() ));

	// Only standard PCM
	if( hdr->wFormatTag != 0x0001 )
		SW_RETURN_WARNING( FString::Printf(TEXT("Unsupported WAV format [%s] in sound [%s]"), *SwTwoCCToStr(hdr->wFormatTag), Sound->GetFullName() ));

	// Only 1 (and sometimes 2) channel
	if( hdr->nChannels != 1 
	&&  hdr->nChannels != 2 )
		SW_RETURN_WARNING( FString::Printf(TEXT("Unsupported channel count [%s] in sound [%s]"), *ToStr(hdr->nChannels), Sound->GetFullName() ));

	// Rate is 44100, 22050 or 11025
	if( hdr->nSamplesPerSec != 44100 
	&&  hdr->nSamplesPerSec != 22050
	&&  hdr->nSamplesPerSec != 11025 )
		SW_RETURN_WARNING( FString::Printf(TEXT("Unsupported rate [%s] in sound [%s]"), *ToStr(hdr->nSamplesPerSec), Sound->GetFullName() ));

	// 16 or 8 bps
	if( hdr->wBitsPerSample != 16 
	&&  hdr->wBitsPerSample != 8 )
		SW_RETURN_WARNING( FString::Printf(TEXT("Unsupported bps [%s] in sound [%s]"), *ToStr(hdr->wBitsPerSample), Sound->GetFullName() ));

	// Data chunk
	if( hdr->DataID != 0x61746164 )
		SW_RETURN_WARNING( FString::Printf(TEXT("Missing data chunk in sound [%s]"), Sound->GetFullName() ));

	// Dump data
	/*SW_LOG( SW_NAME, TEXT("wFormatTag: %s"), *ToStr(hdr->wFormatTag) );
	SW_LOG( SW_NAME, TEXT("nChannels: %s"), *ToStr(hdr->nChannels) );
	SW_LOG( SW_NAME, TEXT("nSamplesPerSec: %s"), *ToStr(hdr->nSamplesPerSec) );
	SW_LOG( SW_NAME, TEXT("nAvgBytesPerSec: %s"), *ToStr(hdr->nAvgBytesPerSec) );
	SW_LOG( SW_NAME, TEXT("nBlockAlign: %s"), *ToStr(hdr->nBlockAlign) );
	SW_LOG( SW_NAME, TEXT("wBitsPerSample: %s"), *ToStr(hdr->wBitsPerSample) );*/

	// Generate buffer
	ALuint buffer;
	alGenBuffers( 1, &buffer );
	if( SW_GET_AL_ERROR )
		SW_RETURN_WARNING( FString::Printf(TEXT("alGenBuffers failed [%s] in sound [%s]"), *GetALErrorString(result), Sound->GetFullName() ));

	// Init buffer params
	ALenum format = hdr->nChannels == 1 
				? (hdr->wBitsPerSample == 8 ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16) 
				: (hdr->wBitsPerSample == 8 ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16);
	ALvoid* data = &(hdr->FirstByte);
	ALsizei size = hdr->DataSize;
	ALsizei freq = hdr->nSamplesPerSec;


	// Init buffer
	alBufferData( buffer, format, data, size, freq );
	if( SW_GET_AL_ERROR )
		SW_RETURN_WARNING( FString::Printf(TEXT("alBufferData failed [%s] in sound [%s]"), *GetALErrorString(result), Sound->GetFullName() ));

	// store buffer
	Sound->Handle = (void*)buffer;

	return true;
	unguard;
}

UBOOL FSoundLoader::ParseMP3( USound* Sound )
{
	swguard(FSoundLoader::ParseMP3);


	return 1;
	unguard;
}


FString SwTwoCCToStr( _WORD code )
{
	switch( code )
	{
		case 0x0000: return FString(TEXT("Microsoft Unknown Wave Format"));
		case 0x0001: return FString(TEXT("Microsoft PCM Format"));
		case 0x0002: return FString(TEXT("Microsoft ADPCM Format"));
		case 0x0003: return FString(TEXT("IEEE Float"));
		case 0x0004: return FString(TEXT("Compaq Computer VSELP (codec for Windows CE 2.0 devices)"));
		case 0x0005: return FString(TEXT("IBM CVSD"));
		case 0x0006: return FString(TEXT("Microsoft ALAW (CCITT A-Law)"));
		case 0x0007: return FString(TEXT("Microsoft MULAW (CCITT u-Law)"));
		case 0x0008: return FString(TEXT("Microsoft DTS"));
		case 0x0009: return FString(TEXT("Microsoft DRM"));
		case 0x0010: return FString(TEXT("OKI ADPCM"));
		case 0x0011: return FString(TEXT("Intel DVI ADPCM (IMA ADPCM)"));
		case 0x0012: return FString(TEXT("Videologic MediaSpace ADPCM"));
		case 0x0013: return FString(TEXT("Sierra Semiconductor ADPCM"));
		case 0x0014: return FString(TEXT("Antex Electronics G.723 ADPCM"));
		case 0x0015: return FString(TEXT("DSP Solutions DigiSTD"));
		case 0x0016: return FString(TEXT("DSP Solutions DigiFIX"));
		case 0x0017: return FString(TEXT("Dialogic OKI ADPCM"));
		case 0x0018: return FString(TEXT("MediaVision ADPCM"));
		case 0x0019: return FString(TEXT("Hewlett-Packard CU codec"));
		case 0x0020: return FString(TEXT("Yamaha ADPCM"));
		case 0x0021: return FString(TEXT("Speech Compression SONARC"));
		case 0x0022: return FString(TEXT("DSP Group True Speech"));
		case 0x0023: return FString(TEXT("Echo Speech EchoSC1"));
		case 0x0024: return FString(TEXT("Audiofile AF36"));
		case 0x0025: return FString(TEXT("Audio Processing Technology APTX"));
		case 0x0026: return FString(TEXT("AudioFile AF10"));
		case 0x0027: return FString(TEXT("Prosody 1612 codec for CTI Speech Card"));
		case 0x0028: return FString(TEXT("Merging Technologies S.A. LRC"));
		case 0x0030: return FString(TEXT("Dolby Labs AC2"));
		case 0x0031: return FString(TEXT("Microsoft GSM 6.10"));
		case 0x0032: return FString(TEXT("MSNAudio"));
		case 0x0033: return FString(TEXT("Antex Electronics ADPCME"));
		case 0x0034: return FString(TEXT("Control Resources VQLPC"));
		case 0x0035: return FString(TEXT("DSP Solutions DigiREAL"));
		case 0x0036: return FString(TEXT("DSP Solutions DigiADPCM"));
		case 0x0037: return FString(TEXT("Control Resources CR10"));
		case 0x0038: return FString(TEXT("Natural MicroSystems VBXADPCM"));
		case 0x0039: return FString(TEXT("Roland RDAC (Crystal Semiconductor IMA ADPCM)"));
		case 0x003A: return FString(TEXT("Echo Speech EchoSC3"));
		case 0x003B: return FString(TEXT("Rockwell ADPCM"));
		case 0x003C: return FString(TEXT("Rockwell Digit LK"));
		case 0x003D: return FString(TEXT("Xebec Multimedia Solutions"));
		case 0x0040: return FString(TEXT("Antex Electronics G.721 ADPCM"));
		case 0x0041: return FString(TEXT("Antex Electronics G.728 CELP"));
		case 0x0042: return FString(TEXT("Microsoft MSG723"));
		case 0x0043: return FString(TEXT("IBM AVC ADPCM"));
		case 0x0045: return FString(TEXT("ITU-T G.726 ADPCM"));
		case 0x0050: return FString(TEXT("MPEG-1 layer 1, 2"));
		case 0x0052: return FString(TEXT("InSoft RT24 (ACM codec is an alternative codec)"));
		case 0x0053: return FString(TEXT("InSoft PAC"));
		case 0x0055: return FString(TEXT("MPEG-1 Layer 3 (MP3)"));
		case 0x0059: return FString(TEXT("Lucent G.723"));
		case 0x0060: return FString(TEXT("Cirrus Logic"));
		case 0x0061: return FString(TEXT("ESS Technology ESPCM / Duck DK4 ADPCM"));
		case 0x0062: return FString(TEXT("Voxware file-mode codec / Duck DK3 ADPCM"));
		case 0x0063: return FString(TEXT("Canopus Atrac"));
		case 0x0064: return FString(TEXT("APICOM G.726 ADPCM"));
		case 0x0065: return FString(TEXT("APICOM G.722 ADPCM"));
		case 0x0066: return FString(TEXT("Microsoft DSAT"));
		case 0x0067: return FString(TEXT("Microsoft DSAT Display"));
		case 0x0069: return FString(TEXT("Voxware Byte Aligned (bitstream-mode codec)"));
		case 0x0070: return FString(TEXT("Voxware AC8 (Lernout & Hauspie CELP 4.8 kbps)"));
		case 0x0071: return FString(TEXT("Voxware AC10 (Lernout & Hauspie CBS 8kbps)"));
		case 0x0072: return FString(TEXT("Voxware AC16 (Lernout & Hauspie CBS 12kbps)"));
		case 0x0073: return FString(TEXT("Voxware AC20 (Lernout & Hauspie CBS 16kbps)"));
		case 0x0074: return FString(TEXT("Voxware MetaVoice (file and stream oriented)"));
		case 0x0075: return FString(TEXT("Voxware MetaSound (file and stream oriented)"));
		case 0x0076: return FString(TEXT("Voxware RT29HW"));
		case 0x0077: return FString(TEXT("Voxware VR12"));
		case 0x0078: return FString(TEXT("Voxware VR18"));
		case 0x0079: return FString(TEXT("Voxware TQ40"));
		case 0x0080: return FString(TEXT("Softsound"));
		case 0x0081: return FString(TEXT("Voxware TQ60"));
		case 0x0082: return FString(TEXT("Microsoft MSRT24 (ACM codec is an alternative codec)"));
		case 0x0083: return FString(TEXT("AT&T Labs G.729A"));
		case 0x0084: return FString(TEXT("Motion Pixels MVI MV12"));
		case 0x0085: return FString(TEXT("DataFusion Systems G.726"));
		case 0x0086: return FString(TEXT("DataFusion Systems GSM610"));
		case 0x0088: return FString(TEXT("Iterated Systems ISIAudio"));
		case 0x0089: return FString(TEXT("Onlive"));
		case 0x0091: return FString(TEXT("Siemens Business Communications SBC24"));
		case 0x0092: return FString(TEXT("Sonic Foundry Dolby AC3 SPDIF"));
		case 0x0093: return FString(TEXT("MediaSonic G.723"));
		case 0x0094: return FString(TEXT("Aculab PLC Prosody 8KBPS"));
		case 0x0097: return FString(TEXT("ZyXEL ADPCM"));
		case 0x0098: return FString(TEXT("Philips LPCBB"));
		case 0x0099: return FString(TEXT("Studer Professional Audio AG Packed"));
		case 0x00A0: return FString(TEXT("Malden Electronics PHONYTALK"));
		case 0x00FF: return FString(TEXT("AAC"));
		case 0x0100: return FString(TEXT("Rhetorex ADPCM"));
		case 0x0101: return FString(TEXT("IBM mu-law / BeCubed Software IRAT"));
		case 0x0102: return FString(TEXT("IBM A-law"));
		case 0x0103: return FString(TEXT("IBM AVC ADPCM"));
		case 0x0111: return FString(TEXT("Vivo G.723"));
		case 0x0112: return FString(TEXT("Vivo Siren"));
		case 0x0123: return FString(TEXT("Digital G.723"));
		case 0x0125: return FString(TEXT("Sanyo LD ADPCM"));
		case 0x0130: return FString(TEXT("Sipro Lab Telecom ACELP.net"));
		case 0x0131: return FString(TEXT("Sipro Lab Telecom ACELP.4800"));
		case 0x0132: return FString(TEXT("Sipro Lab Telecom ACELP.8V3"));
		case 0x0133: return FString(TEXT("Sipro Lab Telecom ACELP.G.729"));
		case 0x0134: return FString(TEXT("Sipro Lab Telecom ACELP.G.729A"));
		case 0x0135: return FString(TEXT("Sipro Lab Telecom ACELP.KELVIN"));
		case 0x0140: return FString(TEXT("Dictaphone G.726 ADPCM"));
		case 0x0150: return FString(TEXT("Qualcomm PureVoice"));
		case 0x0151: return FString(TEXT("Qualcomm HalfRate"));
		case 0x0155: return FString(TEXT("Ring Zero Systems TUB GSM"));
		case 0x0160: return FString(TEXT("Windows Media Audio V1 / DivX audio (WMA)"));
		case 0x0161: return FString(TEXT("Windows Media Audio V2 V7 V8 V9 / DivX audio (WMA) / Alex AC3 Audio"));
		case 0x0162: return FString(TEXT("Windows Media Audio Professional V9"));
		case 0x0163: return FString(TEXT("Windows Media Audio Lossless V9"));
		case 0x0170: return FString(TEXT("UNISYS NAP ADPCM"));
		case 0x0171: return FString(TEXT("UNISYS NAP ULAW"));
		case 0x0172: return FString(TEXT("UNISYS NAP ALAW"));
		case 0x0173: return FString(TEXT("UNISYS NAP 16K"));
		case 0x0200: return FString(TEXT("Creative Labs ADPCM"));
		case 0x0202: return FString(TEXT("Creative Labs FastSpeech8"));
		case 0x0203: return FString(TEXT("Creative Labs FastSpeech10"));
		case 0x0210: return FString(TEXT("UHER Informatic ADPCM"));
		case 0x0215: return FString(TEXT("Ulead DV ACM"));
		case 0x0216: return FString(TEXT("Ulead DV ACM"));
		case 0x0220: return FString(TEXT("Quarterdeck"));
		case 0x0230: return FString(TEXT("I-link Worldwide ILINK VC"));
		case 0x0240: return FString(TEXT("Aureal Semiconductor RAW SPORT"));
		case 0x0241: return FString(TEXT("ESST AC3"));
		case 0x0250: return FString(TEXT("Interactive Products HSX"));
		case 0x0251: return FString(TEXT("Interactive Products RPELP"));
		case 0x0260: return FString(TEXT("Consistent Software CS2"));
		case 0x0270: return FString(TEXT("Sony ATRAC3 (SCX, same as MiniDisk LP2)"));
		case 0x0300: return FString(TEXT("Fujitsu FM TOWNS SND"));
		case 0x0400: return FString(TEXT("BTV Digital (Brooktree digital audio format)"));
		case 0x0401: return FString(TEXT("Intel Music Coder (IMC)"));
		case 0x0402: return FString(TEXT("Ligos Indeo Audio"));
		case 0x0450: return FString(TEXT("QDesign Music"));
		case 0x0680: return FString(TEXT("AT&T Labs VME VMPCM"));
		case 0x0681: return FString(TEXT("AT&T Labs TPC"));
		case 0x0700: return FString(TEXT("YMPEG Alpha (dummy for MPEG-2 compressor)"));
		case 0x08AE: return FString(TEXT("ClearJump LiteWave"));
		case 0x1000: return FString(TEXT("Olivetti GSM"));
		case 0x1001: return FString(TEXT("Olivetti ADPCM"));
		case 0x1002: return FString(TEXT("Olivetti CELP"));
		case 0x1003: return FString(TEXT("Olivetti SBC"));
		case 0x1004: return FString(TEXT("Olivetti OPR"));
		case 0x1100: return FString(TEXT("Lernout & Hauspie codec"));
		case 0x1101: return FString(TEXT("Lernout & Hauspie CELP codec"));
		case 0x1102: return FString(TEXT("Lernout & Hauspie SBC codec"));
		case 0x1103: return FString(TEXT("Lernout & Hauspie SBC codec"));
		case 0x1104: return FString(TEXT("Lernout & Hauspie SBC codec"));
		case 0x1400: return FString(TEXT("Norris Communication"));
		case 0x1401: return FString(TEXT("AT&T Labs ISIAudio"));
		case 0x1500: return FString(TEXT("AT&T Labs Soundspace Music Compression"));
		case 0x181C: return FString(TEXT("VoxWare RT24 speech codec"));
		case 0x181E: return FString(TEXT("Lucent elemedia AX24000P Music codec"));
		case 0x1C07: return FString(TEXT("Lucent SX8300P speech codec"));
		case 0x1C0C: return FString(TEXT("Lucent SX5363S G.723 compliant codec"));
		case 0x1F03: return FString(TEXT("CUseeMe DigiTalk (ex-Rocwell)"));
		case 0x1FC4: return FString(TEXT("NCT Soft ALF2CD ACM"));
		case 0x2000: return FString(TEXT("Dolby AC3 / FAST Multimedia AG DVM"));
		case 0x2001: return FString(TEXT("Dolby DTS (Digital Theater System)"));
		case 0x2002: return FString(TEXT("RealAudio 1 / 2 14.4"));
		case 0x2003: return FString(TEXT("RealAudio 1 / 2 28.8"));
		case 0x2004: return FString(TEXT("RealAudio G2 / 8 Cook (low bitrate)"));
		case 0x2005: return FString(TEXT("RealAudio 3 / 4 / 5 Music (DNET)"));
		case 0x2006: return FString(TEXT("RealAudio 10 AAC (RAAC)"));
		case 0x2007: return FString(TEXT("RealAudio 10 AAC+ (RACP)"));
		case 0x3313: return FString(TEXT("makeAVIS (ffvfw fake AVI sound from AviSynth scripts)"));
		case 0x4143: return FString(TEXT("Divio MPEG-4 AAC audio"));
		case 0x434C: return FString(TEXT("LEAD Speech"));
		case 0x564C: return FString(TEXT("LEAD Vorbis"));
		case 0x674f: return FString(TEXT("Ogg Vorbis (mode 1)"));
		case 0x6750: return FString(TEXT("Ogg Vorbis (mode 2)"));
		case 0x6751: return FString(TEXT("Ogg Vorbis (mode 3)"));
		case 0x676f: return FString(TEXT("Ogg Vorbis (mode 1+)"));
		case 0x6770: return FString(TEXT("Ogg Vorbis (mode 2+)"));
		case 0x6771: return FString(TEXT("Ogg Vorbis (mode 3+)"));
		case 0x7A21: return FString(TEXT("GSM-AMR (CBR, no SID)"));
		case 0x7A22: return FString(TEXT("GSM-AMR (VBR, including SID)"));
		case 0xDFAC: return FString(TEXT("DebugMode SonicFoundry Vegas FrameServer ACM Codec"));
		case 0xF1AC: return FString(TEXT("Free Lossless Audio Codec FLAC"));
		case 0xFFFE: return FString(TEXT("Extensible wave format"));
		case 0xFFFF: return FString(TEXT("In Development / Unregistered"));
		default: return FString::Printf(TEXT("Unknown Format #%.04x"), code); 
	}
}


/* ----------------------------------------------------------------------------
	The End.
---------------------------------------------------------------------------- */

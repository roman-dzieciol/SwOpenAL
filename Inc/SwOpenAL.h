/* ============================================================================
	SwOpenAL.h:
	Copyright 2007 Roman Switch` Dzieciol. All Rights Reserved.
============================================================================ */
#ifndef SWOPENAL_H
#define SWOPENAL_H


// Includes.
#include <al.h>
#include <alc.h>
#include "Core.h"
#include "Engine.h"



// Libs.
#pragma comment(lib, "Core")
#pragma comment(lib, "Engine")
#pragma comment(lib, "OpenAL32")


// Macros
#define SWOPENAL_API DLL_EXPORT
#define SW_NAME (EName)SWOPENAL_SwOpenAL.GetIndex()
#define SW_SEP FString(TEXT(" | "))
#define swguard(text) guard(text##())
#define SW_LOG GLog->Logf
#define SW_ERR GError->Logf
#define SW_LOGP *SwTimeStr(), __FUNC_NAME__
//#define SW_LOGPD *SwTimeStr(), __FUNC_NAME__, __LINE__
//#define SW_INVALID_BUFFER ((void*)-1)
#define SW_GET_AL_ERROR (result = alGetError()) != AL_NO_ERROR


// Defs
extern SWOPENAL_API FName SWOPENAL_SwOpenAL;


// Utils
#include "SwToStr.h"


#endif
/* ----------------------------------------------------------------------------
	The End.
---------------------------------------------------------------------------- */
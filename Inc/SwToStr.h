/* ============================================================================
	SwToStr.h:
	Copyright 2007 Roman Switch` Dzieciol. All Rights Reserved.
============================================================================ */
#ifndef SWTOSTR_H
#define SWTOSTR_H



/*-----------------------------------------------------------------------------
	SwTimeStr.
-----------------------------------------------------------------------------*/

inline FString SwTimeStr( bool bShort=false )
{	
	INT tYear, tMonth, tDayOfWeek, tDay, tHour, tMin, tSec, tMSec;
	appSystemTime( tYear, tMonth, tDayOfWeek, tDay, tHour, tMin, tSec, tMSec );
	FString Result = FString::Printf( TEXT("[%02d:%02d:%03d]"), tMin, tSec, tMSec );
	return Result;
}

/*-----------------------------------------------------------------------------
	SwGetName
-----------------------------------------------------------------------------*/
inline FString SwGetName( UObject* p, bool bShort=false )
{
	return p ? p->GetName() : TEXT("None");
}

/*-----------------------------------------------------------------------------
	SwGetPath
-----------------------------------------------------------------------------*/
inline FString SwGetPathName( UObject* p, bool bShort=false )
{
	return p ? p->GetPathName() : TEXT("None");
}


/* ============================================================================
	To String functions
============================================================================ */

/*-----------------------------------------------------------------------------
	ToStr :: FName
-----------------------------------------------------------------------------*/
inline FString ToStr( const FName p, bool bShort=false )
{
	return *p;
}


/*-----------------------------------------------------------------------------
	ToStr :: FVector
-----------------------------------------------------------------------------*/
inline FString ToStr( const FVector p, bool bShort=false )
{
	return FString::Printf(TEXT("%.02f,%.02f,%.02f"), p.X, p.Y, p.Z); 
}


/*-----------------------------------------------------------------------------
	ToStr :: FRotator
-----------------------------------------------------------------------------*/
inline FString ToStr( const FRotator p, bool bShort=false )
{
	return FString::Printf(TEXT("%d,%d,%d"), p.Pitch, p.Yaw, p.Roll); 
}


/*-----------------------------------------------------------------------------
	ToStr :: UBOOL
-----------------------------------------------------------------------------*/
inline FString ToStr( const UBOOL p, bool bShort=false )
{
	return FString::Printf(TEXT("%d"), p); 
}


/*-----------------------------------------------------------------------------
	ToStr :: UObject
-----------------------------------------------------------------------------*/
inline FString ToStr( UObject* p, bool bShort=false )
{
	FString s = FString::Printf(TEXT("ON:%p"), p); 
	if( p == NULL || bShort )
		return s;
	
	s += SW_SEP + FString::Printf(TEXT("%s"),p->GetFullName());
	s += SW_SEP + FString::Printf(TEXT("%.08x"),p->GetFlags());
	return s;
}


/*-----------------------------------------------------------------------------
	ToStr :: AActor
-----------------------------------------------------------------------------*/
inline FString ToStr( AActor* p, bool bShort=false )
{
	FString s = FString::Printf(TEXT("AC:%p"), p); 
	if( p == NULL )
		return s;

	s += SW_SEP + FString::Printf(TEXT("%s"),p->GetName());
	if( bShort )
		return s;
	
	s += SW_SEP + FString::Printf(TEXT("%.08x"),p->GetFlags());
	s += SW_SEP + FString::Printf(TEXT("%d%d%d"),p->Role,p->RemoteRole,p->Level->NetMode);
	
	return s;
}

/*-----------------------------------------------------------------------------
	ToStr :: ETravelType
-----------------------------------------------------------------------------*/
inline FString ToStr( ETravelType a, bool bShort=false )
{
	switch( a )
	{
	case TRAVEL_Absolute: return FString(TEXT("ABSOLUTE"));
	case TRAVEL_Partial: return FString(TEXT("PARTIAL"));
	case TRAVEL_Relative: return FString(TEXT("RELATIVE"));
	default: return FString(TEXT("UNKNOWN"));
	}
}


/*-----------------------------------------------------------------------------
	ToStr :: ENetRole
-----------------------------------------------------------------------------*/
inline FString ToStr( ENetRole a, bool bShort=false )
{
	switch( a )
	{
	case ROLE_None: return FString(TEXT("ROLE_None"));
	case ROLE_DumbProxy: return FString(TEXT("ROLE_DumbProxy"));
	case ROLE_SimulatedProxy: return FString(TEXT("ROLE_SimulatedProxy"));
	case ROLE_AutonomousProxy: return FString(TEXT("ROLE_AutonomousProxy"));
	case ROLE_Authority: return FString(TEXT("ROLE_Authority"));
	default: return FString(TEXT("UNKNOWN"));
	}
}

/*-----------------------------------------------------------------------------
	ToStr :: FMemStack
-----------------------------------------------------------------------------*/
inline FString ToStr( FMemStack* p, bool bShort=false )
{
	return FString::Printf(TEXT("MS:%p %d"), p, p->GetByteCount()); 
}

/*-----------------------------------------------------------------------------
	ToStr :: FCheckResult
-----------------------------------------------------------------------------*/
inline FString ToStr( FCheckResult* c, bool bShort=false )
{
	FString S;
	int i = 0;

	while( c )
	{
		FString aname;
		if( c->Actor )
			aname = c->Actor->GetFullName();

		FString pname;
		if( c->Primitive )
			pname = c->Primitive->GetFullName();

		S += FString::Printf( TEXT("FCR[%d]< P[%s] L[%s] N[%s] T[%f] I[%d] A[%s] N[%p] > ")
		, i
		, *pname
		, *ToStr(c->Location)
		, *ToStr(c->Normal)
		, c->Time
		, c->Item
		, *aname
		, c->Next
		);


		i++;
		c = c->GetNext();
	}

	return S;
}

/*-----------------------------------------------------------------------------
	ToStr :: ULevel
-----------------------------------------------------------------------------*/
inline FString ToStr( ULevel* p, bool bShort=false )
{
	FString s = FString::Printf(TEXT("LV:%p"), p); 
	if( p == NULL || bShort )
		return s;
	
	s += SW_SEP + FString::Printf(TEXT("%s"),p->GetFullName());
	return s;
}


/*-----------------------------------------------------------------------------
	ToStr :: FFileManager
-----------------------------------------------------------------------------*/
inline FString ToStr( FFileManager* p, bool bShort=false )
{
	FString s = FString::Printf(TEXT("FM:%p"), p); 
	if( p == NULL || bShort )
		return s;
	return s;
}

/*-----------------------------------------------------------------------------
	ToStr :: USound
-----------------------------------------------------------------------------*/
inline FString ToStr( FSoundData p, bool bShort=false )
{
	FString s = FString::Printf(TEXT("%d"), p.Num()); 
	return s;
}


/*-----------------------------------------------------------------------------
	ToStr :: USound
-----------------------------------------------------------------------------*/
inline FString ToStr( USound* p, bool bShort=false )
{
	FString s = FString::Printf(TEXT("SO:%p"), p); 
	if( p == NULL || bShort )
		return s;

	s += SW_SEP + p->GetName();
	s += SW_SEP + *ToStr(p->FileType);
	s += SW_SEP + *ToStr(p->OriginalSize);
	s += SW_SEP + *ToStr(p->Duration);
	s += SW_SEP + FString::Printf(TEXT("%p"), p->Handle); 
	//if( p->Handle )
	//	s += SW_SEP + *ToStr(p->Data);

	return s;
}

/*-----------------------------------------------------------------------------
	ToStr :: UMusic
-----------------------------------------------------------------------------*/
inline FString ToStr( UMusic* p, bool bShort=false )
{
	FString s = FString::Printf(TEXT("MU:%p"), p); 
	if( p == NULL || bShort )
		return s;

	s += SW_SEP + p->GetName();
	//s += SW_SEP + *ToStr(p->FileType);

	return s;
}

/*-----------------------------------------------------------------------------
	ToStr :: FCoords
-----------------------------------------------------------------------------*/
inline FString ToStr( const FCoords p, bool bShort=false )
{
	FString s = *ToStr(p.Origin);
	if( bShort )
		return s;

	s += SW_SEP + *ToStr(p.XAxis);
	s += SW_SEP + *ToStr(p.YAxis);
	s += SW_SEP + *ToStr(p.ZAxis);
	return s;
}


/*-----------------------------------------------------------------------------
	ToStr :: FPointRegion
-----------------------------------------------------------------------------*/
inline FString ToStr( const FPointRegion p, bool bShort=false )
{
	FString s = *ToStr(p.Zone);
	if( bShort )
		return s;

	s += SW_SEP + *ToStr(p.iLeaf);
	s += SW_SEP + *ToStr(p.ZoneNumber);
	return s;
}


#endif
/* ----------------------------------------------------------------------------
	The End.
---------------------------------------------------------------------------- */
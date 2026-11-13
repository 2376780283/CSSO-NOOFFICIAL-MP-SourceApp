//===== Copyright � 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose:
//
// $NoKeywords: $
//===========================================================================//

#ifndef TE_EFFECT_DISPATCH_H
#define TE_EFFECT_DISPATCH_H
#ifdef _WIN32
#pragma once
#endif


#include "effect_dispatch_data.h"

void DispatchEffect( const char *pName, const CEffectData &data );
void DispatchEffect( IRecipientFilter& filter, float flDelay, const char *pName, const CEffectData &data );
inline void DispatchEffect( const char *pName, const CEffectData &data, IRecipientFilter &filter )
{
	DispatchEffect( filter, 0.0f, pName, data );
}

#endif // TE_EFFECT_DISPATCH_H

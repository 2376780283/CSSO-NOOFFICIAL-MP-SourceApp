//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "precache_register.h"
#include "datacache/iprecachesystem.h"
#include "tier1/KeyValues.h"
#include "filesystem.h"
#include "igamesystem.h"

namespace
{
class CResourcePrecacheHandler : public IPrecacheHandler
{
public:
	void CacheResource( PrecacheResourceType_t type, const char *name, bool precache, int *index = NULL ) override
	{
		if ( index )
		{
			if ( type == PARTICLE_MATERIAL )
				*reinterpret_cast<void **>( index ) = NULL;
			else
				*index = 0;
		}
		if ( !precache )
			return;

		int result = 0;
		switch ( type )
		{
		case MATERIAL:
			PrecacheMaterial( name );
			result = GetMaterialIndex( name );
			break;
		case MODEL: result = CBaseEntity::PrecacheModel( name ); break;
		case GAMESOUND: result = CBaseEntity::PrecacheScriptSound( name ); break;
		case PARTICLE_SYSTEM:
			PrecacheParticleSystem( name );
			result = GetParticleSystemIndex( name );
			break;
		case ENTITY: UTIL_PrecacheOther( name ); break;
		case DECAL: result = UTIL_PrecacheDecal( name, true ); break;
		case SHARED:
			if ( g_pPrecacheSystem )
				g_pPrecacheSystem->Cache( this, SHARED_SYSTEM, name, true );
			break;
		case KV_DEP_FILE: CacheFile( name ); break;
		case PARTICLE_MATERIAL:
#ifdef CLIENT_DLL
			// Legacy particle material indices are pointer-sized on Android 64-bit.
			if ( index )
				*reinterpret_cast<void **>( index ) = ParticleMgr()->GetPMaterial( name );
			else
				ParticleMgr()->GetPMaterial( name );
#endif
			return;
		case VGUI_RESOURCE:
		case GAME_MATERIAL_DECALS:
			break;
		default: Warning( "Unknown precache resource type %d\n", type ); break;
		}
		if ( index )
			*index = result;
	}

private:
	void CacheFile( const char *name )
	{
		static const char *types[] = { "vgui_resource", "material", "model", "scriptsound", "particle_system", "entity", "decal", "pmaterial", "dependency_file", "game_material_decals", "shared" };
		COMPILE_TIME_ASSERT( ARRAYSIZE( types ) == PRECACHE_RESOURCE_TYPE_COUNT );
		KeyValues *values = new KeyValues( "ResourceFile" );
		if ( values->LoadFromFile( g_pFullFileSystem, name, "GAME" ) )
		{
			for ( KeyValues *entry = values->GetFirstSubKey(); entry; entry = entry->GetNextKey() )
			{
				for ( int i = 0; i < ARRAYSIZE( types ); ++i )
				{
					if ( !Q_stricmp( entry->GetName(), types[i] ) )
					{
						CacheResource( static_cast<PrecacheResourceType_t>( i ), entry->GetString(), true );
						break;
					}
				}
			}
		}
		else
			Warning( "Can't open precache resource file %s\n", name );
		values->deleteThis();
	}
};

CResourcePrecacheHandler s_ResourcePrecacheHandler;

class CResourcePrecacheLifecycle : public CAutoGameSystem
{
public:
	void LevelShutdownPostEntity() override
	{
		if ( g_pPrecacheSystem )
			g_pPrecacheSystem->UncacheAll( &s_ResourcePrecacheHandler );
	}
} s_ResourcePrecacheLifecycle;
}

IPrecacheHandler *g_pPrecacheHandler = &s_ResourcePrecacheHandler;

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static CPrecacheRegister	*g_pPrecacheRegisters = 0;

CPrecacheRegister::CPrecacheRegister(PrecacheFn fn, const void *pUser)
{
	m_Fn = fn;
	m_pUser = const_cast<void *>(pUser);

	m_pNext = g_pPrecacheRegisters;
	g_pPrecacheRegisters = this;
}


void CPrecacheRegister::Precache()
{
	if ( g_pPrecacheSystem )
		g_pPrecacheSystem->Cache( g_pPrecacheHandler, GLOBAL, NULL, true );
	else
		Warning( "Resource precache system is unavailable\n" );

	for(CPrecacheRegister *pCur=g_pPrecacheRegisters; pCur; pCur=pCur->m_pNext)
	{
		pCur->m_Fn(pCur->m_pUser);
	}
}


void CPrecacheRegister::PrecacheFn_Other(void *pUser)
{
	UTIL_PrecacheOther((const char*)pUser);
}



//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 3D inventory item preview panel
//
// $NoKeywords: $
//=============================================================================//

#include "vgui_inventory_preview.h"

#include <vgui/IInput.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/Label.h>

#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "materialsystem/imesh.h"
#include "matsys_controls/matsyscontrols.h"
#include "VGuiMatSurface/IMatSystemSurface.h"
#include "istudiorender.h"
#include "renderparm.h"
#include "bone_setup.h"
#include "animation.h"
#include "ai_activity.h"
#include "engine/ivmodelrender.h"
#include "cs_skin_database.h"

// Defined in the client module.
extern IVModelRender *modelrender;

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

struct InventoryPreviewModel_t
{
	CSWeaponID	weaponID;
	const char	*pszModel;
	const char	*pszIdleSequence;
};

static const InventoryPreviewModel_t s_InventoryPreviewModels[] =
{
	{ WEAPON_P250,			"models/weapons/w_pist_p250.mdl",				"t_loadout_pistol_idle" },
	{ WEAPON_GLOCK,			"models/weapons/w_pist_glock18.mdl",			"t_loadout_pistol_idle" },
	{ WEAPON_SSG08,			"models/weapons/w_snip_ssg08.mdl",				"t_loadout_rifle02_idle_sniper" },
	{ WEAPON_XM1014,		"models/weapons/w_shot_xm1014.mdl",				"t_loadout_shotgun_xm_idle" },
	{ WEAPON_C4,			"models/weapons/w_c4.mdl",						"t_loadout_c4_idle" },
	{ WEAPON_MAC10,			"models/weapons/w_smg_mac10.mdl",				"t_loadout_shotgun_xm_idle" },
	{ WEAPON_AUG,			"models/weapons/w_rif_aug.mdl",					"ct_loadout_rifle_idle_handrepo_aug" },
	{ WEAPON_ELITE,			"models/weapons/w_pist_elite.mdl",				"t_loadout_dual_idle" },
	{ WEAPON_FIVESEVEN,		"models/weapons/w_pist_fiveseven.mdl",			"ct_loadout_pistol01_idle" },
	{ WEAPON_UMP45,			"models/weapons/w_smg_ump45.mdl",				"t_loadout_ump45_idle" },
	{ WEAPON_SCAR20,		"models/weapons/w_snip_scar20.mdl",				"ct_loadout_rifle_scar_idle" },
	{ WEAPON_GALILAR,		"models/weapons/w_rif_galilar.mdl",				"t_loadout_rifle02_idle_galil" },
	{ WEAPON_FAMAS,			"models/weapons/w_rif_famas.mdl",				"ct_loadout_famas_idle" },
	{ WEAPON_USP,			"models/weapons/w_pist_usp.mdl",					"ct_loadout_pistol01_idle" },
	{ WEAPON_AWP,			"models/weapons/w_snip_awp.mdl",					"t_loadout_rifle02_idle_awp" },
	{ WEAPON_MP5SD,			"models/weapons/w_smg_mp5sd.mdl",				"t_loadout_shotgun_xm_idle" },
	{ WEAPON_M249,			"models/weapons/w_mach_m249.mdl",				"t_loadout_heavy_m249_idle" },
	{ WEAPON_NOVA,			"models/weapons/w_shot_nova.mdl",				"t_loadout_shotgun_idle" },
	{ WEAPON_M4A1,			"models/weapons/w_rif_m4a1_silencer.mdl",		"ct_loadout_rifle_idle_handrepo_m4" },
	{ WEAPON_MP9,			"models/weapons/w_smg_mp9.mdl",					"ct_loadout_p90_idle" },
	{ WEAPON_G3SG1,			"models/weapons/w_snip_g3sg1.mdl",				"t_loadout_rifle02_idle_g3sg" },
	{ WEAPON_DEAGLE,		"models/weapons/w_pist_deagle.mdl",				"t_loadout_pistol_idle" },
	{ WEAPON_SG556,			"models/weapons/w_rif_sg556.mdl",				"t_loadout_rifle02_idle" },
	{ WEAPON_AK47,			"models/weapons/w_rif_ak47.mdl",					"t_loadout_rifle02_idle" },
	{ WEAPON_P90,			"models/weapons/w_smg_p90.mdl",					"t_loadout_p90_idle" },
	{ WEAPON_HKP2000,		"models/weapons/w_pist_hkp2000.mdl",				"ct_loadout_pistol01_idle" },
	{ WEAPON_TEC9,			"models/weapons/w_pist_tec9.mdl",				"t_loadout_pistol_idle" },
	{ WEAPON_M4A4,			"models/weapons/w_rif_m4a4.mdl",					"ct_loadout_rifle_idle_handrepo_m4" },
	{ WEAPON_REVOLVER,		"models/weapons/w_pist_revolver.mdl",			"t_loadout_pistol_idle" },
	{ WEAPON_CZ75A,			"models/weapons/w_pist_cz_75.mdl",				"t_loadout_pistol_idle" },
	{ WEAPON_MAG7,			"models/weapons/w_shot_mag7.mdl",				"ct_loadout_mag7_idle" },
	{ WEAPON_SAWEDOFF,		"models/weapons/w_shot_sawedoff.mdl",			"t_loadout_shotgun_idle" },
	{ WEAPON_NEGEV,			"models/weapons/w_mach_negev.mdl",				"t_loadout_heavy_idle" },
	{ WEAPON_MP7,			"models/weapons/w_smg_mp7.mdl",					"t_loadout_mp7_idle" },
	{ WEAPON_BIZON,			"models/weapons/w_smg_bizon.mdl",				"t_loadout_bizon_idle" },
	{ WEAPON_TASER,			"models/weapons/w_eq_taser.mdl",					"t_loadout_pistol_idle" },

	{ WEAPON_KNIFE,					"models/weapons/w_knife_default_t.mdl",		"t_loadout_knife_idle1" },
	{ WEAPON_KNIFE_T,				"models/weapons/w_knife_default_t.mdl",		"t_loadout_knife_idle1" },
	{ WEAPON_KNIFE_GG,				"models/weapons/w_knife_default_t.mdl",		"t_loadout_knife_idle1" },
	{ WEAPON_KNIFE_CSS,				"models/weapons/w_knife_css.mdl",				"t_loadout_knife_idle1" },
	{ WEAPON_KNIFE_KARAMBIT,		"models/weapons/w_knife_karam.mdl",			"t_loadout_knife_idle1" },
	{ WEAPON_KNIFE_FLIP,			"models/weapons/w_knife_flip.mdl",				"t_loadout_knife_idle1" },
	{ WEAPON_KNIFE_BAYONET,			"models/weapons/w_knife_bayonet.mdl",			"t_loadout_knife_idle1" },
	{ WEAPON_KNIFE_M9_BAYONET,		"models/weapons/w_knife_m9_bay.mdl",			"t_loadout_knife_idle1" },
	{ WEAPON_KNIFE_BUTTERFLY,		"models/weapons/w_knife_butterfly.mdl",		"t_loadout_knife_idle1" },
	{ WEAPON_KNIFE_GUT,				"models/weapons/w_knife_gut.mdl",				"t_loadout_knife_idle1" },
	{ WEAPON_KNIFE_TACTICAL,		"models/weapons/w_knife_tactical.mdl",			"t_loadout_knife_idle1" },
	{ WEAPON_KNIFE_FALCHION,		"models/weapons/w_knife_falchion_advanced.mdl","t_loadout_knife_idle1" },
	{ WEAPON_KNIFE_SURVIVAL_BOWIE,	"models/weapons/w_knife_survival_bowie.mdl",	"t_loadout_knife_idle1" },
	{ WEAPON_KNIFE_CANIS,			"models/weapons/w_knife_canis.mdl",			"t_loadout_knife_idle1" },
	{ WEAPON_KNIFE_CORD,			"models/weapons/w_knife_cord.mdl",				"t_loadout_knife_idle1" },
	{ WEAPON_KNIFE_GYPSY,			"models/weapons/w_knife_gypsy_jackknife.mdl",	"t_loadout_knife_idle1" },
	{ WEAPON_KNIFE_OUTDOOR,			"models/weapons/w_knife_outdoor.mdl",			"t_loadout_knife_idle1" },
	{ WEAPON_KNIFE_SKELETON,		"models/weapons/w_knife_skeleton.mdl",			"t_loadout_knife_idle1" },
	{ WEAPON_KNIFE_STILETTO,		"models/weapons/w_knife_stiletto.mdl",			"t_loadout_knife_idle1" },
	{ WEAPON_KNIFE_URSUS,			"models/weapons/w_knife_ursus.mdl",			"t_loadout_knife_idle1" },
	{ WEAPON_KNIFE_WIDOWMAKER,		"models/weapons/w_knife_widowmaker.mdl",		"t_loadout_knife_idle1" },
	{ WEAPON_KNIFE_PUSH,			"models/weapons/w_knife_push.mdl",				"t_loadout_push_idle" },
};

static const InventoryPreviewModel_t *FindInventoryPreviewModel( CSWeaponID weaponID )
{
	for ( int i = 0; i < ARRAYSIZE( s_InventoryPreviewModels ); ++i )
	{
		if ( s_InventoryPreviewModels[i].weaponID == weaponID )
			return &s_InventoryPreviewModels[i];
	}

	return NULL;
}

CInventoryPreviewPanel::CInventoryPreviewPanel( Panel *parent, const char *panelName )
	: BaseClass( parent, panelName )
{
	m_DefaultEnvCubemap.Init( vgui::MaterialSystem()->FindTexture( "editor/cube_vertigo", NULL, true ) );
	m_MaterialOverride.Init( (IMaterial*)NULL );

	m_Camera.m_origin.Init( 60.0f, 0.0f, 0.0f );
	m_Camera.m_angles.Init( 0.0f, 180.0f, 0.0f );
	m_Camera.m_flZNear = 3.0f;
	m_Camera.m_flZFar = 16384.0f * 1.73205080757f;
	m_Camera.m_flFOV = 40.0f;

	m_nNumLightDescs = 0;
	m_bMousePressed = false;
	m_flLastFrameTime = (float)vgui::system()->GetFrameTime();
	m_flRotationAngleLeft = 0.0f;
	m_flRotationTimeLeft = 0.0f;
	m_angModel.Init();
	SetIdentityMatrix( m_MDLToWorld );

	memset( &m_pLightDesc[0], 0, sizeof( LightDesc_t ) );
	m_pLightDesc[0].m_Type = MATERIAL_LIGHT_DIRECTIONAL;
	m_pLightDesc[0].m_Color.Init( 1.0f, 1.0f, 1.0f );
	m_pLightDesc[0].m_Direction.Init( 0.0f, 0.0f, -1.0f );
	m_pLightDesc[0].m_Range = 0.0;
	m_pLightDesc[0].m_Attenuation0 = 1.0;
	m_pLightDesc[0].m_Attenuation1 = 0;
	m_pLightDesc[0].m_Attenuation2 = 0;
	m_pLightDesc[0].RecalculateDerivedValues();
	m_nNumLightDescs = 1;

	for ( int i = 0; i < 6; ++i )
	{
		m_vecAmbientCube[i].Init( 0.4f, 0.4f, 0.4f, 1.0f );
	}

	SetBgColor( Color( 20, 22, 26, 255 ) );
}

CInventoryPreviewPanel::~CInventoryPreviewPanel()
{
	m_MaterialOverride.Shutdown();
	m_DefaultEnvCubemap.Shutdown();
}

void CInventoryPreviewPanel::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
	SetBgColor( pScheme->GetColor( "Frame.BgColor", Color( 20, 22, 26, 255 ) ) );
}

void CInventoryPreviewPanel::ClearPreview()
{
	m_MaterialOverride.Shutdown();
	m_MDL.SetMDL( MDLHANDLE_INVALID );
	m_MDL.m_flTime = 0.0f;
}

void CInventoryPreviewPanel::SetPreviewItem( CSWeaponID weaponID, int iPaintKit )
{
	const InventoryPreviewModel_t *pInfo = FindInventoryPreviewModel( weaponID );
	if ( !pInfo )
	{
		ClearPreview();
		return;
	}

	SetMDL( pInfo->pszModel );
	PlaySequence( pInfo->pszIdleSequence );

	m_angModel.Init( 0.0f, 0.0f, 0.0f );
	UpdateModelTransform();

	m_MDL.m_flTime = 0.0f;

	m_MaterialOverride.Shutdown();
	if ( iPaintKit > 0 )
	{
		g_SkinDatabase.Initialize();
		m_MaterialOverride.Init( g_SkinDatabase.GetSkinMaterial( iPaintKit ) );
	}
}

void CInventoryPreviewPanel::SetMDL( const char *pMDLName )
{
	MDLHandle_t hMDLFindResult = vgui::MDLCache()->FindMDL( pMDLName );
	MDLHandle_t hMDL = pMDLName ? hMDLFindResult : MDLHANDLE_INVALID;
	if ( vgui::MDLCache()->IsErrorModel( hMDL ) )
	{
		hMDL = MDLHANDLE_INVALID;
	}

	m_MDL.SetMDL( hMDL );
	m_MDL.m_pProxyData = NULL;

	Vector vecMins, vecMaxs;
	GetMDLBoundingBox( &vecMins, &vecMaxs, hMDL, m_MDL.m_nSequence );

	m_MDL.m_bWorldSpaceViewTarget = false;
	m_MDL.m_vecViewTarget.Init( 100.0f, 0.0f, vecMaxs.z );

	int nRef = vgui::MDLCache()->Release( hMDLFindResult );
	(void)nRef;
	AssertMsg( hMDL == MDLHANDLE_INVALID || nRef > 0, "Inventory preview referenced a model that has a zero ref count." );
}

void CInventoryPreviewPanel::PlaySequence( const char *pszSequenceName )
{
	CStudioHdr studioHDR( m_MDL.GetStudioHdr(), g_pMDLCache );
	int iSeq = ::LookupSequence( &studioHDR, pszSequenceName );
	if ( iSeq != ACT_INVALID )
	{
		m_MDL.m_nSequence = iSeq;
		m_MDL.m_flTime = 0.0f;
	}
}

void CInventoryPreviewPanel::UpdateModelTransform()
{
	AngleMatrix( m_angModel, m_MDLToWorld );
}

void CInventoryPreviewPanel::SetupRenderState( int nDisplayWidth, int nDisplayHeight )
{
	CMatRenderContextPtr pRenderContext( g_pMaterialSystem );

	VMatrix view, projection;
	ComputeViewMatrix( &view, m_Camera );
	ComputeProjectionMatrix( &projection, m_Camera, nDisplayWidth, nDisplayHeight );

	pRenderContext->MatrixMode( MATERIAL_MODEL );
	pRenderContext->LoadIdentity();

	pRenderContext->MatrixMode( MATERIAL_VIEW );
	pRenderContext->LoadMatrix( view );

	pRenderContext->MatrixMode( MATERIAL_PROJECTION );
	pRenderContext->LoadMatrix( projection );

	for ( int i = 0; i < m_nNumLightDescs; ++i )
	{
		pRenderContext->SetLight( i, m_pLightDesc[i] );
	}

	LightDesc_t desc;
	desc.m_Type = MATERIAL_LIGHT_DISABLE;
	int nMaxLightCount = g_pMaterialSystemHardwareConfig->MaxNumLights();
	for ( int i = m_nNumLightDescs; i < nMaxLightCount; ++i )
	{
		pRenderContext->SetLight( i, desc );
	}

	pRenderContext->SetAmbientLightCube( m_vecAmbientCube );

	if ( !g_pStudioRender )
		return;

	VMatrix worldToCamera;
	MatrixInverseTR( view, worldToCamera );
	Vector vecOrigin, vecRight, vecUp, vecForward;
	MatrixGetColumn( worldToCamera, 0, &vecRight );
	MatrixGetColumn( worldToCamera, 1, &vecUp );
	MatrixGetColumn( worldToCamera, 2, &vecForward );
	MatrixGetColumn( worldToCamera, 3, &vecOrigin );
	g_pStudioRender->SetViewState( vecOrigin, vecRight, vecUp, vecForward );
	g_pStudioRender->SetLocalLights( m_nNumLightDescs, m_pLightDesc );
	g_pStudioRender->SetAmbientLightColors( m_vecAmbientCube );
}

void CInventoryPreviewPanel::Paint()
{
	int wide, tall;
	GetSize( wide, tall );

	vgui::MatSystemSurface()->Begin3DPaint( 0, 0, wide, tall, false );

	// The dialog is small compared to the main menu player preview, so use the
	// panel size directly instead of clipping against the full screen.
	SetupRenderState( wide, tall );

	CMatRenderContextPtr pRenderContext( vgui::MaterialSystem() );
	Color clrBg = GetBgColor();
	pRenderContext->ClearColor4ub( clrBg.r(), clrBg.g(), clrBg.b(), clrBg.a() );
	pRenderContext->ClearBuffers( false, true );

	pRenderContext->CullMode( MATERIAL_CULLMODE_CCW );
	pRenderContext->SetIntRenderingParameter( INT_RENDERPARM_WRITE_DEPTH_TO_DESTALPHA, false );

	OnPaint3D();

	pRenderContext->CullMode( MATERIAL_CULLMODE_CW );

	vgui::MatSystemSurface()->End3DPaint();
}

void CInventoryPreviewPanel::OnPaint3D()
{
	if ( m_MDL.GetMDL() == MDLHANDLE_INVALID )
		return;

	MDLCACHE_CRITICAL_SECTION();

	CMatRenderContextPtr pRenderContext( vgui::MaterialSystem() );
	pRenderContext->BindLocalCubemap( m_DefaultEnvCubemap );

	if ( m_MaterialOverride.IsValid() && modelrender )
		modelrender->ForcedMaterialOverride( m_MaterialOverride );

	CStudioHdr studioHdr( g_pMDLCache->GetStudioHdr( m_MDL.GetMDL() ), g_pMDLCache );
	matrix3x4_t *pBoneToWorld = g_pStudioRender->LockBoneMatrices( studioHdr.numbones() );
	m_MDL.SetUpBones( m_MDLToWorld, studioHdr.numbones(), pBoneToWorld );
	g_pStudioRender->UnlockBoneMatrices();
	m_MDL.Draw( m_MDLToWorld, pBoneToWorld );

	if ( m_MaterialOverride.IsValid() && modelrender )
		modelrender->ForcedMaterialOverride( NULL );

	pRenderContext->Flush();
}

void CInventoryPreviewPanel::OnThink()
{
	BaseClass::OnThink();

	float flNow = (float)vgui::system()->GetFrameTime();
	float flDelta = flNow - m_flLastFrameTime;
	m_flLastFrameTime = flNow;

	if ( flDelta < 0.0f || flDelta > 0.25f )
		flDelta = 0.0f;

	if ( m_MDL.GetMDL() != MDLHANDLE_INVALID )
		m_MDL.m_flTime += flDelta;

	bool bTransformChanged = false;

	if ( m_flRotationTimeLeft > 0.0f )
	{
		const float flRotationTime = 1.0f;
		float flPercentage = m_flRotationTimeLeft / flRotationTime;
		float flAngleDelta = m_flRotationAngleLeft * flPercentage * flDelta;

		m_angModel.y += flAngleDelta;
		m_flRotationAngleLeft -= flAngleDelta;
		m_flRotationTimeLeft -= flDelta;
		bTransformChanged = true;
	}

	if ( m_angModel.y > 360.0f )
		m_angModel.y -= 360.0f;
	else if ( m_angModel.y < -360.0f )
		m_angModel.y += 360.0f;

	if ( bTransformChanged )
		UpdateModelTransform();
}

void CInventoryPreviewPanel::OnMousePressed( vgui::MouseCode code )
{
	RequestFocus();
	vgui::input()->GetCursorPosition( m_nLastMouseX, m_nLastMouseY );
	m_bMousePressed = true;
}

void CInventoryPreviewPanel::OnMouseReleased( vgui::MouseCode code )
{
	m_bMousePressed = false;
}

void CInventoryPreviewPanel::OnCursorMoved( int x, int y )
{
	if ( !m_bMousePressed )
		return;

	int xpos, ypos;
	vgui::input()->GetCursorPos( xpos, ypos );

	float flDelta = (float)( xpos - m_nLastMouseX );
	m_flRotationAngleLeft += flDelta;
	if ( m_flRotationAngleLeft != 0.0f )
		m_flRotationTimeLeft = 1.0f;

	m_nLastMouseX = xpos;
	m_nLastMouseY = ypos;
}

void CInventoryPreviewPanel::OnCursorExited()
{
	m_bMousePressed = false;
}

//-----------------------------------------------------------------------------
// CInventoryPreviewDialog
//-----------------------------------------------------------------------------
CInventoryPreviewDialog::CInventoryPreviewDialog( vgui::Panel *parent, const char *panelName )
	: BaseClass( parent, panelName )
{
	SetTitle( "Item Preview", true );
	SetSize( 420, 360 );
	SetMinimumSize( 300, 240 );
	SetSizeable( true );
	SetDeleteSelfOnClose( false );

	m_pPreviewPanel = new CInventoryPreviewPanel( this, "PreviewPanel" );
	m_pNameLabel = new vgui::Label( this, "ItemNameLabel", "" );
	m_pNameLabel->SetContentAlignment( vgui::Label::a_center );

	MoveToCenterOfScreen();
}

CInventoryPreviewDialog::~CInventoryPreviewDialog()
{
}

void CInventoryPreviewDialog::PreviewItem( CSWeaponID weaponID, int iPaintKit, const char *pszName )
{
	if ( m_pPreviewPanel )
		m_pPreviewPanel->SetPreviewItem( weaponID, iPaintKit );

	if ( m_pNameLabel )
		m_pNameLabel->SetText( pszName ? pszName : "" );

	if ( pszName && pszName[0] )
		SetTitle( pszName, true );

	Activate();
	MoveToFront();
}

void CInventoryPreviewDialog::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetBgColor( pScheme->GetColor( "Frame.BgColor", Color( 20, 22, 26, 255 ) ) );

	if ( m_pNameLabel )
	{
		m_pNameLabel->SetFont( pScheme->GetFont( "Default", true ) );
		m_pNameLabel->SetFgColor( pScheme->GetColor( "Label.TextColor", Color( 255, 255, 255, 255 ) ) );
	}
}

void CInventoryPreviewDialog::PerformLayout()
{
	BaseClass::PerformLayout();

	int px, py, pw, ph;
	GetClientArea( px, py, pw, ph );

	const int padding = 8;
	const int nameHeight = 24;
	int previewHeight = ph - padding * 2 - nameHeight;

	if ( m_pPreviewPanel )
	{
		m_pPreviewPanel->SetBounds( px + padding, py + padding,
			pw - padding * 2, previewHeight );
	}

	if ( m_pNameLabel )
	{
		m_pNameLabel->SetBounds( px + padding, py + ph - padding - nameHeight,
			pw - padding * 2, nameHeight );
	}
}

void CInventoryPreviewDialog::OnClose()
{
	SetVisible( false );
}

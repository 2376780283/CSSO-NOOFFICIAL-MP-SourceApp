//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 3D inventory item preview panel
//
// $NoKeywords: $
//=============================================================================//

#ifndef VGUI_INVENTORY_PREVIEW_H
#define VGUI_INVENTORY_PREVIEW_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Panel.h>
#include <vgui_controls/Frame.h>
#include "cs_weapon_parse.h"
#include "tier2/camerautils.h"
#include "tier3/mdlutils.h"
#include "materialsystem/MaterialSystemUtil.h"

#define INVENTORY_PREVIEW_MAX_LIGHTS 4

namespace vgui
{
	class Label;
}

class CInventoryPreviewPanel : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CInventoryPreviewPanel, vgui::Panel );

public:
	CInventoryPreviewPanel( vgui::Panel *parent, const char *panelName );
	virtual ~CInventoryPreviewPanel();

	void SetPreviewItem( CSWeaponID weaponID, int iPaintKit );
	void ClearPreview();

	virtual void Paint();
	virtual void OnThink();
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void OnMousePressed( vgui::MouseCode code );
	virtual void OnMouseReleased( vgui::MouseCode code );
	virtual void OnCursorMoved( int x, int y );
	virtual void OnCursorExited();

private:
	void SetupRenderState( int nDisplayWidth, int nDisplayHeight );
	void OnPaint3D();
	void SetMDL( const char *pMDLName );
	void PlaySequence( const char *pszSequenceName );
	void UpdateModelTransform();

	Camera_t			m_Camera;
	Vector4D			m_vecAmbientCube[6];
	LightDesc_t			m_pLightDesc[INVENTORY_PREVIEW_MAX_LIGHTS];
	CMDL				m_MDL;
	matrix3x4_t			m_MDLToWorld;
	CTextureReference	m_DefaultEnvCubemap;
	CMaterialReference	m_MaterialOverride;
	QAngle				m_angModel;

	int					m_nNumLightDescs;
	int					m_nLastMouseX;
	int					m_nLastMouseY;
	float				m_flRotationAngleLeft;
	float				m_flRotationTimeLeft;
	float				m_flLastFrameTime;
	bool				m_bMousePressed;
};

//-----------------------------------------------------------------------------
// Preview dialog shown from the inventory item context menu
//-----------------------------------------------------------------------------
class CInventoryPreviewDialog : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE( CInventoryPreviewDialog, vgui::Frame );

public:
	CInventoryPreviewDialog( vgui::Panel *parent, const char *panelName );
	virtual ~CInventoryPreviewDialog();

	void PreviewItem( CSWeaponID weaponID, int iPaintKit, const char *pszName );

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void PerformLayout();
	virtual void OnClose();

private:
	CInventoryPreviewPanel	*m_pPreviewPanel;
	vgui::Label				*m_pNameLabel;
};

#endif // VGUI_INVENTORY_PREVIEW_H

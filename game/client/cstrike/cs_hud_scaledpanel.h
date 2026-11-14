#ifndef CS_HUD_SCALEDPANEL_H
#define CS_HUD_SCALEDPANEL_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/EditablePanel.h>
#include <vgui/ISurface.h>
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/MaterialSystemUtil.h"

extern ConVar cl_hud_scale;

// Transform the whole subtree, including fonts and dynamically created icons.
// Keep resource coordinates intact so HUD animation targets remain valid.
class CCSHudScaledPanel : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CCSHudScaledPanel, EditablePanel );

public:
	CCSHudScaledPanel( vgui::Panel *parent, const char *name )
		: vgui::EditablePanel( parent, name ), m_flAnchorX( 0.5f ), m_flAnchorY( 0.0f ) {}

	void SetHudScaleAnchor( float x, float y ) { m_flAnchorX = x; m_flAnchorY = y; }

	virtual void PaintTraverse( bool repaint, bool allowForce = true )
	{
		const float scale = cl_hud_scale.GetFloat();
		if ( scale == 1.0f )
		{
			vgui::EditablePanel::PaintTraverse( repaint, allowForce );
			return;
		}

		int wide, tall;
		vgui::surface()->GetScreenSize( wide, tall );
		const float x = wide * m_flAnchorX;
		const float y = tall * m_flAnchorY;
		// Surface batches glyphs; flush on both sides of the matrix change.
		vgui::surface()->DrawFlushText();
		CMatRenderContextPtr context( materials );
		context->MatrixMode( MATERIAL_MODEL );
		context->PushMatrix();
		context->Translate( x, y, 0 );
		context->Scale( scale, scale, 1 );
		context->Translate( -x, -y, 0 );
		vgui::EditablePanel::PaintTraverse( repaint, allowForce );
		vgui::surface()->DrawFlushText();
		context->MatrixMode( MATERIAL_MODEL );
		context->PopMatrix();
	}

private:
	float m_flAnchorX;
	float m_flAnchorY;
};

#endif

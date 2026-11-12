//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Skin inventory options property page
//
// $NoKeywords: $
//=============================================================================//

#ifndef MODOPTIONSSUBINVENTORY_H
#define MODOPTIONSSUBINVENTORY_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/PropertyPage.h>

namespace vgui
{
	class Button;
	class ComboBox;
	class CInventoryPanel;
}

//-----------------------------------------------------------------------------
// Purpose: skin inventory options property page
//-----------------------------------------------------------------------------
class CModOptionsSubInventory : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE( CModOptionsSubInventory, vgui::PropertyPage );

public:
	CModOptionsSubInventory( vgui::Panel *parent );
	~CModOptionsSubInventory();

	MESSAGE_FUNC_PTR( OnTextChanged, "TextChanged", panel );

protected:
	// Called when page is loaded.  Data should be reloaded from document into controls.
	virtual void OnResetData();
	// Called when the OK / Apply button is pressed.  Changed data should be written into document.
	virtual void OnApplyChanges();

	virtual void PerformLayout();
	virtual void OnCommand( const char *command );

private:
	void CreateControls();

	vgui::CInventoryPanel	*m_pInventoryPanel;
	vgui::Button			*m_pShowAllButton;
	vgui::ComboBox			*m_pWeaponFilter;
	vgui::ComboBox			*m_pRarityFilter;
};

#endif // MODOPTIONSSUBINVENTORY_H

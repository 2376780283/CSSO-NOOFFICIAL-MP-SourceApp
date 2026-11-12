//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Context menu used by the skin inventory item panel
//
// $NoKeywords: $
//=============================================================================//

#ifndef VGUI_ITEM_CONTEXT_MENU_H
#define VGUI_ITEM_CONTEXT_MENU_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Menu.h>
#include "cs_weapon_parse.h"

enum InventoryItemType_t
{
	INVENTORY_ITEM_WEAPON_SKIN = 0,
	INVENTORY_ITEM_KNIFE,
	INVENTORY_ITEM_GLOVES,
	INVENTORY_ITEM_AGENT_CT,
	INVENTORY_ITEM_AGENT_T,
	INVENTORY_ITEM_MUSIC
};

class CItemContextMenu : public vgui::Menu
{
	DECLARE_CLASS_SIMPLE( CItemContextMenu, vgui::Menu );

public:
	CItemContextMenu( vgui::Panel *parent, const char *panelName );
	virtual ~CItemContextMenu();

	void ShowForItem( int iPaintKit, CSWeaponID weaponID, InventoryItemType_t itemType,
		int iSpecialIndex, const char *pszWeaponName, const char *pszSkinName, int x, int y );
	virtual void PerformLayout();

	int GetCurrentPaintKit() const { return m_iCurrentPaintKit; }
	CSWeaponID GetCurrentWeaponID() const { return m_CurrentWeaponID; }
	InventoryItemType_t GetItemType() const { return m_ItemType; }
	int GetSpecialIndex() const { return m_iSpecialIndex; }

private:
	void BuildMenuForItem();

	int					m_iCurrentPaintKit;
	CSWeaponID			m_CurrentWeaponID;
	InventoryItemType_t	m_ItemType;
	int					m_iSpecialIndex;
	char				m_szWeaponName[64];
	char				m_szSkinName[128];
};

#endif // VGUI_ITEM_CONTEXT_MENU_H

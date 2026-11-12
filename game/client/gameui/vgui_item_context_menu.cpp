//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Context menu used by the skin inventory item panel
//
// $NoKeywords: $
//=============================================================================//

#include "vgui_item_context_menu.h"

#include <vgui/ISurface.h>
#include "tier0/dbg.h"
#include "tier1/KeyValues.h"
#include "tier1/strtools.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

CItemContextMenu::CItemContextMenu( vgui::Panel *parent, const char *panelName )
	: BaseClass( parent, panelName )
{
	m_iCurrentPaintKit = 0;
	m_CurrentWeaponID = WEAPON_NONE;
	m_ItemType = INVENTORY_ITEM_WEAPON_SKIN;
	m_iSpecialIndex = 0;
	m_szWeaponName[0] = '\0';
	m_szSkinName[0] = '\0';
}

CItemContextMenu::~CItemContextMenu()
{
}

void CItemContextMenu::PerformLayout()
{
	BaseClass::PerformLayout();

	int itemHeight = GetMenuItemHeight();
	if ( itemHeight <= 0 )
		itemHeight = 20;

	int menuHeight = GetItemCount() * itemHeight + 4;
	SetSize( 190, menuHeight );
}

void CItemContextMenu::BuildMenuForItem()
{
	DeleteAllItems();

	if ( m_ItemType == INVENTORY_ITEM_KNIFE || m_ItemType == INVENTORY_ITEM_GLOVES )
	{
		AddMenuItem( "Equip for all", new KeyValues( "MenuCommand", "command", "equip_all" ), GetParent() );
		AddMenuItem( "Equip for CT", new KeyValues( "MenuCommand", "command", "equip_ct" ), GetParent() );
		AddMenuItem( "Equip for T", new KeyValues( "MenuCommand", "command", "equip_t" ), GetParent() );
		AddMenuItem( "Unequip", new KeyValues( "MenuCommand", "command", "unequip" ), GetParent() );
	}
	else
	{
		AddMenuItem( "Equip", new KeyValues( "MenuCommand", "command", "equip" ), GetParent() );
		AddMenuItem( "Unequip", new KeyValues( "MenuCommand", "command", "unequip" ), GetParent() );
	}
}

void CItemContextMenu::ShowForItem( int iPaintKit, CSWeaponID weaponID, InventoryItemType_t itemType,
	int iSpecialIndex, const char *pszWeaponName, const char *pszSkinName, int x, int y )
{
	m_iCurrentPaintKit = iPaintKit;
	m_CurrentWeaponID = weaponID;
	m_ItemType = itemType;
	m_iSpecialIndex = iSpecialIndex;

	Q_strncpy( m_szWeaponName, pszWeaponName ? pszWeaponName : "", sizeof( m_szWeaponName ) );
	Q_strncpy( m_szSkinName, pszSkinName ? pszSkinName : "", sizeof( m_szSkinName ) );

	BuildMenuForItem();
	PerformLayout();

	int menuWidth, menuHeight;
	GetSize( menuWidth, menuHeight );

	int screenWidth, screenHeight;
	vgui::surface()->GetScreenSize( screenWidth, screenHeight );

	if ( x + menuWidth > screenWidth )
		x = screenWidth - menuWidth;
	if ( y + menuHeight > screenHeight )
		y = screenHeight - menuHeight;
	if ( x < 0 )
		x = 0;
	if ( y < 0 )
		y = 0;

	SetPos( x, y );
	SetVisible( true );
	MoveToFront();
	RequestFocus();
}

//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Skin inventory options property page
//
// $NoKeywords: $
//=============================================================================//

#include "ModOptionsSubInventory.h"

#include <vgui_controls/Button.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/ScrollableEditablePanel.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/ScrollBar.h>
#include <vgui/IImage.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/IInput.h>
#include <vgui/IVGui.h>

#include "tier0/dbg.h"
#include "tier0/platform.h"
#include "tier1/KeyValues.h"
#include "tier1/convar.h"
#include "tier1/strtools.h"
#include "cs_skin_database.h"
#include "cs_weapon_parse.h"
#include "vgui_item_context_menu.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

namespace vgui
{

#define EQUIP_TEAM_NONE  0
#define EQUIP_TEAM_CT    ( 1 << 0 )
#define EQUIP_TEAM_T     ( 1 << 1 )
#define EQUIP_TEAM_BOTH  ( EQUIP_TEAM_CT | EQUIP_TEAM_T )

//-----------------------------------------------------------------------------
// Purpose: ConVar / slot helpers shared by item indicators and menu commands
//-----------------------------------------------------------------------------
static const char *GetWeaponSkinConVarName( CSWeaponID weaponID )
{
	switch ( weaponID )
	{
		case WEAPON_AK47:       return "loadout_skin_ak47";
		case WEAPON_M4A4:       return "loadout_skin_m4a4";
		case WEAPON_M4A1:       return "loadout_skin_m4a1s";
		case WEAPON_AWP:        return "loadout_skin_awp";
		case WEAPON_DEAGLE:     return "loadout_skin_deagle";
		case WEAPON_REVOLVER:   return "loadout_skin_revolver";
		case WEAPON_GLOCK:      return "loadout_skin_glock";
		case WEAPON_USP:        return "loadout_skin_usp";
		case WEAPON_HKP2000:    return "loadout_skin_hkp2000";
		case WEAPON_P250:       return "loadout_skin_p250";
		case WEAPON_FIVESEVEN:  return "loadout_skin_fiveseven";
		case WEAPON_TEC9:       return "loadout_skin_tec9";
		case WEAPON_CZ75A:      return "loadout_skin_cz75a";
		case WEAPON_ELITE:      return "loadout_skin_dualberettas";
		case WEAPON_MAG7:       return "loadout_skin_mag7";
		case WEAPON_NOVA:       return "loadout_skin_nova";
		case WEAPON_SAWEDOFF:   return "loadout_skin_sawedoff";
		case WEAPON_XM1014:     return "loadout_skin_xm1014";
		case WEAPON_M249:       return "loadout_skin_m249";
		case WEAPON_NEGEV:      return "loadout_skin_negev";
		case WEAPON_MAC10:      return "loadout_skin_mac10";
		case WEAPON_MP9:        return "loadout_skin_mp9";
		case WEAPON_MP7:        return "loadout_skin_mp7";
		case WEAPON_MP5SD:      return "loadout_skin_mp5sd";
		case WEAPON_UMP45:      return "loadout_skin_ump45";
		case WEAPON_P90:        return "loadout_skin_p90";
		case WEAPON_BIZON:      return "loadout_skin_bizon";
		case WEAPON_GALILAR:    return "loadout_skin_galilar";
		case WEAPON_FAMAS:      return "loadout_skin_famas";
		case WEAPON_AUG:        return "loadout_skin_aug";
		case WEAPON_SG556:      return "loadout_skin_sg556";
		case WEAPON_SSG08:      return "loadout_skin_ssg08";
		case WEAPON_SCAR20:     return "loadout_skin_scar20";
		case WEAPON_G3SG1:      return "loadout_skin_g3sg1";
		default:                return NULL;
	}
}

static int GetKnifeSlotIndex( CSWeaponID weaponID )
{
	switch ( weaponID )
	{
		case WEAPON_KNIFE_CSS:           return 1;
		case WEAPON_KNIFE_KARAMBIT:      return 2;
		case WEAPON_KNIFE_FLIP:          return 3;
		case WEAPON_KNIFE_BAYONET:       return 4;
		case WEAPON_KNIFE_M9_BAYONET:    return 5;
		case WEAPON_KNIFE_BUTTERFLY:     return 6;
		case WEAPON_KNIFE_GUT:           return 7;
		case WEAPON_KNIFE_TACTICAL:      return 8;
		case WEAPON_KNIFE_FALCHION:      return 9;
		case WEAPON_KNIFE_SURVIVAL_BOWIE:return 10;
		case WEAPON_KNIFE_CANIS:         return 11;
		case WEAPON_KNIFE_CORD:          return 12;
		case WEAPON_KNIFE_GYPSY:         return 13;
		case WEAPON_KNIFE_OUTDOOR:       return 14;
		case WEAPON_KNIFE_SKELETON:      return 15;
		case WEAPON_KNIFE_STILETTO:      return 16;
		case WEAPON_KNIFE_URSUS:         return 17;
		case WEAPON_KNIFE_WIDOWMAKER:    return 18;
		case WEAPON_KNIFE_PUSH:          return 19;
		default:                         return 0;
	}
}

//-----------------------------------------------------------------------------
// CInventoryItemPanel - panel for a single skin
//-----------------------------------------------------------------------------
class CInventoryItemPanel : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CInventoryItemPanel, vgui::EditablePanel );

public:
	CInventoryItemPanel( Panel *parent, const char *panelName, const SkinDefinition_t *pSkinDef,
		InventoryItemType_t itemType = INVENTORY_ITEM_WEAPON_SKIN );

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void PerformLayout();
	virtual void Paint();
	virtual void OnMousePressed( vgui::MouseCode code );
	virtual void OnCursorEntered();
	virtual void OnCursorExited();

	const SkinDefinition_t *GetSkinDefinition() const { return m_pSkinDef; }
	int GetPaintKit() const { return m_pSkinDef ? m_pSkinDef->iPaintKit : 0; }
	InventoryItemType_t GetItemType() const { return m_ItemType; }
	void UpdateEquipIndicators();

private:
	void UpdateDisplay();
	void SetIconImage( const char *pszIconPath );
	Color GetRarityColor() const;
	int GetEquipFlags() const;

	const SkinDefinition_t	*m_pSkinDef;
	InventoryItemType_t		m_ItemType;
	vgui::Label				*m_pNameLabel;
	vgui::Label				*m_pWeaponLabel;
	vgui::ImagePanel		*m_pIconImage;

	bool					m_bMouseOver;
	int						m_iEquipFlags;
};

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CInventoryItemPanel::CInventoryItemPanel( Panel *parent, const char *panelName, const SkinDefinition_t *pSkinDef,
	InventoryItemType_t itemType )
	: BaseClass( parent, panelName )
{
	m_pSkinDef = pSkinDef;
	m_ItemType = itemType;
	m_bMouseOver = false;
	m_iEquipFlags = EQUIP_TEAM_NONE;

	m_pNameLabel = new vgui::Label( this, "NameLabel", "" );
	m_pNameLabel->SetMouseInputEnabled( false );
	m_pWeaponLabel = new vgui::Label( this, "WeaponLabel", "" );
	m_pWeaponLabel->SetMouseInputEnabled( false );
	m_pIconImage = new vgui::ImagePanel( this, "IconImage" );
	m_pIconImage->SetMouseInputEnabled( false );

	SetMouseInputEnabled( true );

	UpdateDisplay();

	InvalidateLayout();
}

void CInventoryItemPanel::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetBorder( pScheme->GetBorder( "ButtonBorder" ) );
	SetBgColor( GetRarityColor() );

	m_pNameLabel->SetFont( pScheme->GetFont( "DefaultSmall", true ) );
	m_pNameLabel->SetFgColor( pScheme->GetColor( "Label.TextColor", Color( 255, 255, 255, 255 ) ) );

	m_pWeaponLabel->SetFont( pScheme->GetFont( "DefaultVerySmall", true ) );
	m_pWeaponLabel->SetFgColor( pScheme->GetColor( "Label.DisabledTextColor", Color( 180, 180, 180, 255 ) ) );
}

void CInventoryItemPanel::PerformLayout()
{
	BaseClass::PerformLayout();

	int wide, tall;
	GetSize( wide, tall );

	const int margin = 6;
	const int weaponLabelHeight = 15;
	const int nameLabelHeight = 22;
	const int labelSpacing = 3;

	int textBlock = weaponLabelHeight + nameLabelHeight + labelSpacing;
	int iconAreaWidth = wide - margin * 2;
	int iconAreaHeight = tall - margin * 2 - textBlock;
	if ( iconAreaWidth < 0 )
		iconAreaWidth = 0;
	if ( iconAreaHeight < 0 )
		iconAreaHeight = 0;

	int drawWide = iconAreaWidth;
	int drawTall = iconAreaHeight;

	IImage *pImage = m_pIconImage->GetImage();
	if ( pImage )
	{
		int textureWide = 0, textureTall = 0;
		pImage->GetSize( textureWide, textureTall );

		if ( textureWide > 0 && textureTall > 0 )
		{
			float scaleX = (float)iconAreaWidth / textureWide;
			float scaleY = (float)iconAreaHeight / textureTall;
			float scale = MIN( scaleX, scaleY );
			if ( scale > 1.0f )
				scale = 1.0f;

			drawWide = (int)( textureWide * scale + 0.5f );
			drawTall = (int)( textureTall * scale + 0.5f );
		}
	}

	int iconX = ( wide - drawWide ) / 2;
	int iconY = margin + ( iconAreaHeight - drawTall ) / 2;
	m_pIconImage->SetBounds( iconX, iconY, drawWide, drawTall );

	int labelTop = margin + iconAreaHeight + labelSpacing;
	m_pWeaponLabel->SetBounds( margin, labelTop, wide - margin * 2, weaponLabelHeight );
	m_pWeaponLabel->SetContentAlignment( vgui::Label::a_center );

	labelTop += weaponLabelHeight;
	m_pNameLabel->SetBounds( margin, labelTop, wide - margin * 2, nameLabelHeight );
	m_pNameLabel->SetContentAlignment( vgui::Label::a_center );
}

void CInventoryItemPanel::Paint()
{
	BaseClass::Paint();

	if ( m_iEquipFlags == EQUIP_TEAM_NONE )
		return;

	int wide, tall;
	GetSize( wide, tall );

	const int indicatorSize = 10;
	const int indicatorSpacing = 4;
	int indicatorY = 7;
	int indicatorX = wide - indicatorSize - 7;

	if ( m_iEquipFlags & EQUIP_TEAM_CT )
	{
		vgui::surface()->DrawSetColor( Color( 90, 140, 200, 255 ) );
		vgui::surface()->DrawFilledRect( indicatorX, indicatorY, indicatorX + indicatorSize, indicatorY + indicatorSize );
		vgui::surface()->DrawSetColor( Color( 255, 255, 255, 200 ) );
		vgui::surface()->DrawOutlinedRect( indicatorX, indicatorY, indicatorX + indicatorSize, indicatorY + indicatorSize );
		indicatorX -= ( indicatorSize + indicatorSpacing );
	}

	if ( m_iEquipFlags & EQUIP_TEAM_T )
	{
		vgui::surface()->DrawSetColor( Color( 200, 150, 100, 255 ) );
		vgui::surface()->DrawFilledRect( indicatorX, indicatorY, indicatorX + indicatorSize, indicatorY + indicatorSize );
		vgui::surface()->DrawSetColor( Color( 255, 255, 255, 200 ) );
		vgui::surface()->DrawOutlinedRect( indicatorX, indicatorY, indicatorX + indicatorSize, indicatorY + indicatorSize );
	}
}

int CInventoryItemPanel::GetEquipFlags() const
{
	if ( !m_pSkinDef )
		return EQUIP_TEAM_NONE;

	if ( m_ItemType == INVENTORY_ITEM_KNIFE )
	{
		int knifeIndex = GetKnifeSlotIndex( m_pSkinDef->weaponID );
		if ( knifeIndex <= 0 )
			return EQUIP_TEAM_NONE;

		int flags = EQUIP_TEAM_NONE;

		ConVar *pKnifeSkinCT = cvar->FindVar( "loadout_slot_knife_weapon_skin_ct" );
		ConVar *pKnifeSlotCT = cvar->FindVar( "loadout_slot_knife_weapon_ct" );
		if ( pKnifeSkinCT && pKnifeSlotCT &&
			pKnifeSkinCT->GetInt() == m_pSkinDef->iPaintKit &&
			pKnifeSlotCT->GetInt() == knifeIndex )
		{
			flags |= EQUIP_TEAM_CT;
		}

		ConVar *pKnifeSkinT = cvar->FindVar( "loadout_slot_knife_weapon_skin_t" );
		ConVar *pKnifeSlotT = cvar->FindVar( "loadout_slot_knife_weapon_t" );
		if ( pKnifeSkinT && pKnifeSlotT &&
			pKnifeSkinT->GetInt() == m_pSkinDef->iPaintKit &&
			pKnifeSlotT->GetInt() == knifeIndex )
		{
			flags |= EQUIP_TEAM_T;
		}

		return flags;
	}

	const char *pszConVarName = GetWeaponSkinConVarName( m_pSkinDef->weaponID );
	if ( !pszConVarName )
		return EQUIP_TEAM_NONE;

	ConVar *pConVar = cvar->FindVar( pszConVarName );
	if ( pConVar && pConVar->GetInt() == m_pSkinDef->iPaintKit )
		return EQUIP_TEAM_BOTH;

	return EQUIP_TEAM_NONE;
}

void CInventoryItemPanel::UpdateEquipIndicators()
{
	m_iEquipFlags = GetEquipFlags();
	Repaint();
}

void CInventoryItemPanel::OnMousePressed( vgui::MouseCode code )
{
	if ( ( code == MOUSE_LEFT || code == MOUSE_RIGHT ) && m_pSkinDef )
	{
		int cursorX = 0, cursorY = 0;
		vgui::input()->GetCursorPos( cursorX, cursorY );

		KeyValues *pKV = new KeyValues( "ItemSelected" );
		pKV->SetInt( "paintkit", m_pSkinDef->iPaintKit );
		pKV->SetInt( "weaponid", m_pSkinDef->weaponID );
		pKV->SetInt( "itemtype", m_ItemType );
		pKV->SetInt( "specialindex", 0 );
		pKV->SetInt( "cursorx", cursorX );
		pKV->SetInt( "cursory", cursorY );
		PostActionSignal( pKV );
	}

	BaseClass::OnMousePressed( code );
}

void CInventoryItemPanel::OnCursorEntered()
{
	m_bMouseOver = true;
	Color rarityColor = GetRarityColor();

	SetBgColor( Color(
		MIN( rarityColor.r() + 30, 255 ),
		MIN( rarityColor.g() + 30, 255 ),
		MIN( rarityColor.b() + 30, 255 ),
		255
	) );
	BaseClass::OnCursorEntered();
}

void CInventoryItemPanel::OnCursorExited()
{
	m_bMouseOver = false;
	SetBgColor( GetRarityColor() );
	BaseClass::OnCursorExited();
}

void CInventoryItemPanel::UpdateDisplay()
{
	if ( !m_pSkinDef )
		return;

	m_pNameLabel->SetText( m_pSkinDef->szName );

	const char *pszWeaponName = WeaponIDToAlias( m_pSkinDef->weaponID );
	if ( pszWeaponName && Q_strnicmp( pszWeaponName, "weapon_", 7 ) == 0 )
	{
		pszWeaponName += 7;
	}

	char szWeaponDisplay[64];
	Q_snprintf( szWeaponDisplay, sizeof( szWeaponDisplay ), "%s", pszWeaponName ? pszWeaponName : "" );
	if ( szWeaponDisplay[0] >= 'a' && szWeaponDisplay[0] <= 'z' )
		szWeaponDisplay[0] -= 32;

	m_pWeaponLabel->SetText( szWeaponDisplay );

	if ( m_pSkinDef->szIconPath[0] )
	{
		m_pIconImage->SetShouldScaleImage( true );
		m_pIconImage->SetVisible( true );
		SetIconImage( m_pSkinDef->szIconPath );
	}
	else
	{
		m_pIconImage->SetVisible( false );
	}
}

void CInventoryItemPanel::SetIconImage( const char *pszIconPath )
{
	if ( !pszIconPath || !pszIconPath[0] )
		return;

	// ImagePanel / scheme image names are relative to materials/vgui and the
	// scheme manager already prepends "vgui/". Skin packs commonly store
	// either "materials/vgui/...", "vgui/..." or a plain relative path, so
	// try all common forms instead of silently ending up with a blank panel.
	char szNormalized[MAX_PATH];
	Q_strncpy( szNormalized, pszIconPath, sizeof( szNormalized ) );

	for ( char *p = szNormalized; *p; ++p )
	{
		if ( *p == '\\' )
			*p = '/';
	}

	char *pszRaw = szNormalized;
	while ( *pszRaw == '/' )
		++pszRaw;

	const char *pszNoMaterials = pszRaw;
	if ( Q_strnicmp( pszNoMaterials, "materials/", 10 ) == 0 )
		pszNoMaterials += 10;

	const char *pszNoVgui = pszRaw;
	if ( Q_strnicmp( pszNoVgui, "vgui/", 5 ) == 0 )
		pszNoVgui += 5;

	const char *pszRelative = pszNoMaterials;
	if ( Q_strnicmp( pszRelative, "vgui/", 5 ) == 0 )
		pszRelative += 5;

	char szEscaped[MAX_PATH];
	Q_snprintf( szEscaped, sizeof( szEscaped ), "../%s", pszNoMaterials );

	const char *pCandidates[] = { pszRaw, pszNoMaterials, pszNoVgui, pszRelative, szEscaped };
	for ( int i = 0; i < ARRAYSIZE( pCandidates ); ++i )
	{
		if ( !pCandidates[i] || !pCandidates[i][0] )
			continue;

		m_pIconImage->SetImage( pCandidates[i] );

		IImage *pImage = m_pIconImage->GetImage();
		if ( pImage )
		{
			int wide = 0, tall = 0;
			pImage->GetSize( wide, tall );
			if ( wide > 0 && tall > 0 )
				return;
		}
	}

	DevMsg( "[Inventory] Could not load icon '%s' for skin '%s'\n",
		pszIconPath, m_pSkinDef ? m_pSkinDef->szName : "" );
}

Color CInventoryItemPanel::GetRarityColor() const
{
	if ( !m_pSkinDef )
		return Color( 40, 40, 40, 255 );

	switch ( m_pSkinDef->rarity )
	{
		case SKIN_RARITY_COMMON:      return Color( 60, 70, 80, 255 );
		case SKIN_RARITY_UNCOMMON:    return Color( 40, 60, 90, 255 );
		case SKIN_RARITY_RARE:        return Color( 30, 50, 120, 255 );
		case SKIN_RARITY_MYTHICAL:    return Color( 60, 30, 120, 255 );
		case SKIN_RARITY_LEGENDARY:   return Color( 100, 20, 110, 255 );
		case SKIN_RARITY_ANCIENT:     return Color( 120, 30, 30, 255 );
		case SKIN_RARITY_CONTRABAND:  return Color( 110, 80, 20, 255 );
		default:                      return Color( 40, 40, 40, 255 );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Sort skins by weapon, then rarity (best first), then name
//-----------------------------------------------------------------------------
static int __cdecl CompareInventorySkins( const SkinDefinition_t * const *ppLeft, const SkinDefinition_t * const *ppRight )
{
	const SkinDefinition_t *pLeft = *ppLeft;
	const SkinDefinition_t *pRight = *ppRight;

	const char *pszLeftWeapon = WeaponIDToAlias( pLeft->weaponID );
	const char *pszRightWeapon = WeaponIDToAlias( pRight->weaponID );
	if ( !pszLeftWeapon )
		pszLeftWeapon = "";
	if ( !pszRightWeapon )
		pszRightWeapon = "";

	int result = Q_stricmp( pszLeftWeapon, pszRightWeapon );
	if ( result != 0 )
		return result;

	// Higher rarity first within the same weapon.
	if ( pLeft->rarity != pRight->rarity )
		return (int)pRight->rarity - (int)pLeft->rarity;

	result = Q_stricmp( pLeft->szName, pRight->szName );
	if ( result != 0 )
		return result;

	return pLeft->iPaintKit - pRight->iPaintKit;
}

//-----------------------------------------------------------------------------
// CInventoryItemContainer
//-----------------------------------------------------------------------------
class CInventoryItemContainer : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CInventoryItemContainer, vgui::EditablePanel );

public:
	CInventoryItemContainer( Panel *parent, const char *panelName )
		: BaseClass( parent, panelName )
	{
	}

	virtual void PerformLayout() {}

	// EditablePanel::OnSizeChanged() repositions every child to its pin offset.
	// That destroys the manually computed item grid whenever the scrollable
	// container is resized, so deliberately skip the auto-layout here.
	virtual void OnSizeChanged( int wide, int tall )
	{
		InvalidateLayout();
	}
};

//-----------------------------------------------------------------------------
// CInventoryPanel
//-----------------------------------------------------------------------------
class CInventoryPanel : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CInventoryPanel, vgui::EditablePanel );

public:
	CInventoryPanel( Panel *parent, const char *panelName );
	virtual ~CInventoryPanel();

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void PerformLayout();
	virtual void OnCommand( const char *command );
	virtual void OnThink();

	void LoadAllSkins();
	void LoadSkinsForWeapon( CSWeaponID weaponID );
	void ClearInventory();

	void SetWeaponFilter( CSWeaponID weaponID );
	void SetRarityFilter( ESkinRarity rarity );
	void ClearFilters();

	MESSAGE_FUNC_PARAMS( OnItemSelected, "ItemSelected", data );
	MESSAGE_FUNC_PARAMS( OnMenuCommand, "MenuCommand", data );

private:
	void RebuildInventory();
	void LayoutItems();
	void AddSkinToInventory( const SkinDefinition_t *pSkinDef );
	void SaveItemToLoadout( int iPaintKit, CSWeaponID weaponID, InventoryItemType_t itemType, int teamFlags );
	void UnequipItem( CSWeaponID weaponID, InventoryItemType_t itemType );

	vgui::ScrollableEditablePanel	*m_pScrollablePanel;
	vgui::EditablePanel				*m_pItemContainer;
	CUtlVector<CInventoryItemPanel *> m_Items;
	CItemContextMenu				*m_pContextMenu;

	int m_iItemWidth;
	int m_iItemHeight;
	int m_iItemSpacing;
	int m_iItemsPerRow;

	CSWeaponID	m_FilterWeaponID;
	ESkinRarity	m_FilterRarity;
	bool		m_bUseWeaponFilter;
	bool		m_bUseRarityFilter;

	bool m_bFirstLayout;
};

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CInventoryPanel::CInventoryPanel( Panel *parent, const char *panelName )
	: BaseClass( parent, panelName )
{
	m_iItemWidth = 140;
	m_iItemHeight = 132;
	m_iItemSpacing = 8;
	m_iItemsPerRow = 3;

	m_FilterWeaponID = WEAPON_NONE;
	m_FilterRarity = SKIN_RARITY_COMMON;
	m_bUseWeaponFilter = false;
	m_bUseRarityFilter = false;
	m_bFirstLayout = true;

	m_pItemContainer = new CInventoryItemContainer( NULL, "ItemContainer" );

	m_pScrollablePanel = new vgui::ScrollableEditablePanel( this, m_pItemContainer, "InventoryScroll" );

	m_pContextMenu = new CItemContextMenu( this, "ItemContextMenu" );
	m_pContextMenu->SetVisible( false );

	vgui::ivgui()->AddTickSignal( GetVPanel(), 100 );
}

CInventoryPanel::~CInventoryPanel()
{
	ClearInventory();

	if ( m_pContextMenu )
	{
		m_pContextMenu->MarkForDeletion();
		m_pContextMenu = NULL;
	}
}

void CInventoryPanel::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
	SetBgColor( pScheme->GetColor( "Frame.BgColor", Color( 30, 30, 30, 255 ) ) );
}

void CInventoryPanel::OnThink()
{
	BaseClass::OnThink();

	if ( m_bFirstLayout && GetWide() > 0 && GetTall() > 0 )
	{
		m_bFirstLayout = false;
		PostMessage( this, new KeyValues( "DelayedLoad" ), 0.1f );
	}
}

void CInventoryPanel::PerformLayout()
{
	BaseClass::PerformLayout();
	LayoutItems();
}

void CInventoryPanel::LayoutItems()
{
	int wide, tall;
	GetSize( wide, tall );

	if ( wide <= 0 || tall <= 0 )
		return;

	m_pScrollablePanel->SetBounds( 0, 0, wide, tall );

	const int scrollBarWidth = 16;
	const int spacing = m_iItemSpacing;
	const int usableWidth = wide - scrollBarWidth;
	const int minItemWidth = 128;
	const int maxItemWidth = 180;

	m_iItemsPerRow = ( usableWidth + spacing ) / ( minItemWidth + spacing );
	if ( m_iItemsPerRow < 1 )
		m_iItemsPerRow = 1;
	if ( m_iItemsPerRow > 6 )
		m_iItemsPerRow = 6;

	int itemWidth = ( usableWidth - ( m_iItemsPerRow + 1 ) * spacing ) / m_iItemsPerRow;
	if ( itemWidth < 1 )
		itemWidth = 1;
	if ( itemWidth > maxItemWidth )
		itemWidth = maxItemWidth;
	m_iItemWidth = itemWidth;

	int gridWidth = m_iItemsPerRow * m_iItemWidth + ( m_iItemsPerRow + 1 ) * spacing;
	int gridLeft = ( usableWidth - gridWidth ) / 2;
	if ( gridLeft < 0 )
		gridLeft = 0;

	int numItems = m_Items.Count();
	int numRows = ( numItems + m_iItemsPerRow - 1 ) / m_iItemsPerRow;

	int contentHeight = spacing + numRows * ( m_iItemHeight + spacing );
	int containerHeight = max( tall, contentHeight );

	// Resize the container before positioning children. EditablePanel-derived
	// containers may reposition their children from OnSizeChanged(), which
	// would otherwise reset the whole grid to (0,0).
	m_pItemContainer->SetSize( usableWidth, containerHeight );

	for ( int i = 0; i < numItems; i++ )
	{
		int row = i / m_iItemsPerRow;
		int col = i % m_iItemsPerRow;

		int x = gridLeft + spacing + col * ( m_iItemWidth + spacing );
		int y = spacing + row * ( m_iItemHeight + spacing );

		m_Items[i]->SetBounds( x, y, m_iItemWidth, m_iItemHeight );

		// SetBounds() does not invalidate the child's own layout, so make sure
		// its icon and labels get positioned using the new item size.
		m_Items[i]->InvalidateLayout( true );
	}

	// ScrollableEditablePanel only recalculates its scrollbar when it is laid out.
	// The item container changes size after loading/filtering, so keep the bar in sync here.
	vgui::ScrollBar *pScrollBar = m_pScrollablePanel->GetScrollbar();
	if ( pScrollBar )
	{
		pScrollBar->SetRange( 0, containerHeight );
		pScrollBar->SetRangeWindow( tall );
		int maxScroll = max( 0, containerHeight - tall );
		if ( pScrollBar->GetValue() > maxScroll )
			pScrollBar->SetValue( maxScroll );
	}

	m_pScrollablePanel->InvalidateLayout( true );
}

void CInventoryPanel::OnCommand( const char *command )
{
	if ( Q_stricmp( command, "DelayedLoad" ) == 0 )
	{
		LoadAllSkins();
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

void CInventoryPanel::LoadAllSkins()
{
	ClearInventory();

	if ( !g_SkinDatabase.Initialize() )
	{
		Warning( "[Inventory] Failed to initialize skin database\n" );
		return;
	}

	CUtlVector<const SkinDefinition_t *> allSkins;
	g_SkinDatabase.GetAllSkins( allSkins );
	allSkins.Sort( CompareInventorySkins );

	DevMsg( "[Inventory] Loading %d skins into inventory\n", allSkins.Count() );

	FOR_EACH_VEC( allSkins, i )
	{
		const SkinDefinition_t *pSkinDef = allSkins[i];

		if ( m_bUseWeaponFilter && pSkinDef->weaponID != m_FilterWeaponID )
			continue;

		if ( m_bUseRarityFilter && pSkinDef->rarity != m_FilterRarity )
			continue;

		AddSkinToInventory( pSkinDef );
	}

	m_pItemContainer->SetPos( 0, 0 );
	vgui::ScrollBar *pScrollBar = m_pScrollablePanel->GetScrollbar();
	if ( pScrollBar )
		pScrollBar->SetValue( 0 );

	LayoutItems();
	InvalidateLayout();
}

void CInventoryPanel::LoadSkinsForWeapon( CSWeaponID weaponID )
{
	ClearInventory();

	if ( !g_SkinDatabase.Initialize() )
	{
		Warning( "[Inventory] Failed to initialize skin database\n" );
		return;
	}

	CUtlVector<const SkinDefinition_t *> weaponSkins;
	g_SkinDatabase.GetSkinsForWeapon( weaponID, weaponSkins );
	weaponSkins.Sort( CompareInventorySkins );

	DevMsg( "[Inventory] Loading %d skins for weapon %d\n", weaponSkins.Count(), weaponID );

	FOR_EACH_VEC( weaponSkins, i )
	{
		AddSkinToInventory( weaponSkins[i] );
	}

	m_pItemContainer->SetPos( 0, 0 );
	vgui::ScrollBar *pScrollBar = m_pScrollablePanel->GetScrollbar();
	if ( pScrollBar )
		pScrollBar->SetValue( 0 );

	LayoutItems();
	InvalidateLayout();
}

void CInventoryPanel::AddSkinToInventory( const SkinDefinition_t *pSkinDef )
{
	if ( !pSkinDef )
		return;

	char panelName[64];
	Q_snprintf( panelName, sizeof( panelName ), "Item_%d", m_Items.Count() );

	InventoryItemType_t itemType = INVENTORY_ITEM_WEAPON_SKIN;
	if ( pSkinDef->weaponID >= WEAPON_KNIFE && pSkinDef->weaponID <= WEAPON_KNIFE_LAST )
		itemType = INVENTORY_ITEM_KNIFE;

	CInventoryItemPanel *pItem = new CInventoryItemPanel( m_pItemContainer, panelName, pSkinDef, itemType );
	pItem->AddActionSignalTarget( this );
	pItem->UpdateEquipIndicators();
	m_Items.AddToTail( pItem );
}

void CInventoryPanel::ClearInventory()
{
	FOR_EACH_VEC( m_Items, i )
	{
		m_Items[i]->MarkForDeletion();
	}
	m_Items.RemoveAll();
}

void CInventoryPanel::SetWeaponFilter( CSWeaponID weaponID )
{
	m_FilterWeaponID = weaponID;
	m_bUseWeaponFilter = true;
	RebuildInventory();
}

void CInventoryPanel::SetRarityFilter( ESkinRarity rarity )
{
	m_FilterRarity = rarity;
	m_bUseRarityFilter = true;
	RebuildInventory();
}

void CInventoryPanel::ClearFilters()
{
	m_bUseWeaponFilter = false;
	m_bUseRarityFilter = false;
	RebuildInventory();
}

void CInventoryPanel::RebuildInventory()
{
	LoadAllSkins();
}

void CInventoryPanel::OnItemSelected( KeyValues *data )
{
	int iPaintKit = data->GetInt( "paintkit", 0 );
	int iWeaponID = data->GetInt( "weaponid", WEAPON_NONE );
	int iItemType = data->GetInt( "itemtype", INVENTORY_ITEM_WEAPON_SKIN );
	int iSpecialIndex = data->GetInt( "specialindex", 0 );
	int cursorX = data->GetInt( "cursorx", 0 );
	int cursorY = data->GetInt( "cursory", 0 );

	if ( !m_pContextMenu || iPaintKit <= 0 || iWeaponID == WEAPON_NONE )
		return;

	const SkinDefinition_t *pSkinDef = g_SkinDatabase.FindSkinByPaintKit( iPaintKit );
	if ( !pSkinDef )
		return;

	const char *pszWeaponName = WeaponIDToAlias( pSkinDef->weaponID );
	m_pContextMenu->ShowForItem( iPaintKit, (CSWeaponID)iWeaponID, (InventoryItemType_t)iItemType,
		iSpecialIndex, pszWeaponName ? pszWeaponName : "Item", pSkinDef->szName, cursorX, cursorY );
}

void CInventoryPanel::OnMenuCommand( KeyValues *data )
{
	if ( !m_pContextMenu )
		return;

	const char *pszCommand = data->GetString( "command", "" );
	int iPaintKit = m_pContextMenu->GetCurrentPaintKit();
	CSWeaponID weaponID = m_pContextMenu->GetCurrentWeaponID();
	InventoryItemType_t itemType = m_pContextMenu->GetItemType();

	if ( Q_stricmp( pszCommand, "unequip" ) == 0 )
	{
		UnequipItem( weaponID, itemType );
	}
	else
	{
		int teamFlags = EQUIP_TEAM_BOTH;
		if ( Q_stricmp( pszCommand, "equip_ct" ) == 0 )
			teamFlags = EQUIP_TEAM_CT;
		else if ( Q_stricmp( pszCommand, "equip_t" ) == 0 )
			teamFlags = EQUIP_TEAM_T;

		SaveItemToLoadout( iPaintKit, weaponID, itemType, teamFlags );
	}

	m_pContextMenu->SetVisible( false );

	FOR_EACH_VEC( m_Items, i )
	{
		m_Items[i]->UpdateEquipIndicators();
	}
}

void CInventoryPanel::SaveItemToLoadout( int iPaintKit, CSWeaponID weaponID, InventoryItemType_t itemType, int teamFlags )
{
	if ( itemType == INVENTORY_ITEM_KNIFE )
	{
		int knifeIndex = GetKnifeSlotIndex( weaponID );
		if ( knifeIndex <= 0 )
			return;

		if ( teamFlags & EQUIP_TEAM_CT )
		{
			ConVar *pKnifeSkinCT = cvar->FindVar( "loadout_slot_knife_weapon_skin_ct" );
			if ( pKnifeSkinCT )
				pKnifeSkinCT->SetValue( iPaintKit );

			ConVar *pKnifeSlotCT = cvar->FindVar( "loadout_slot_knife_weapon_ct" );
			if ( pKnifeSlotCT )
				pKnifeSlotCT->SetValue( knifeIndex );
		}

		if ( teamFlags & EQUIP_TEAM_T )
		{
			ConVar *pKnifeSkinT = cvar->FindVar( "loadout_slot_knife_weapon_skin_t" );
			if ( pKnifeSkinT )
				pKnifeSkinT->SetValue( iPaintKit );

			ConVar *pKnifeSlotT = cvar->FindVar( "loadout_slot_knife_weapon_t" );
			if ( pKnifeSlotT )
				pKnifeSlotT->SetValue( knifeIndex );
		}

		return;
	}

	const char *pszConVarName = GetWeaponSkinConVarName( weaponID );
	if ( !pszConVarName )
	{
		Warning( "[Inventory] No ConVar found for weapon ID: %d\n", weaponID );
		return;
	}

	ConVar *pConVar = cvar->FindVar( pszConVarName );
	if ( pConVar )
	{
		pConVar->SetValue( iPaintKit );
		Msg( "[Inventory] Equipped skin %d to %s\n", iPaintKit, pszConVarName );
	}
}

void CInventoryPanel::UnequipItem( CSWeaponID weaponID, InventoryItemType_t itemType )
{
	if ( itemType == INVENTORY_ITEM_KNIFE )
	{
		ConVar *pKnifeSkinCT = cvar->FindVar( "loadout_slot_knife_weapon_skin_ct" );
		if ( pKnifeSkinCT )
			pKnifeSkinCT->SetValue( 0 );

		ConVar *pKnifeSkinT = cvar->FindVar( "loadout_slot_knife_weapon_skin_t" );
		if ( pKnifeSkinT )
			pKnifeSkinT->SetValue( 0 );

		ConVar *pKnifeSlotCT = cvar->FindVar( "loadout_slot_knife_weapon_ct" );
		if ( pKnifeSlotCT )
			pKnifeSlotCT->SetValue( 0 );

		ConVar *pKnifeSlotT = cvar->FindVar( "loadout_slot_knife_weapon_t" );
		if ( pKnifeSlotT )
			pKnifeSlotT->SetValue( 0 );

		return;
	}

	const char *pszConVarName = GetWeaponSkinConVarName( weaponID );
	if ( pszConVarName )
	{
		ConVar *pConVar = cvar->FindVar( pszConVarName );
		if ( pConVar )
			pConVar->SetValue( 0 );
	}
}

} // namespace vgui

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CModOptionsSubInventory::CModOptionsSubInventory( vgui::Panel *parent )
	: BaseClass( parent, "ModOptionsSubInventory" )
{
	m_pInventoryPanel = NULL;
	m_pShowAllButton = NULL;
	m_pWeaponFilter = NULL;
	m_pRarityFilter = NULL;

	CreateControls();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CModOptionsSubInventory::~CModOptionsSubInventory()
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CModOptionsSubInventory::CreateControls()
{
	if ( !m_pInventoryPanel )
	{
		m_pInventoryPanel = new vgui::CInventoryPanel( this, "InventoryPanel" );
	}

	if ( !m_pShowAllButton )
	{
		m_pShowAllButton = new vgui::Button( this, "ShowAllButton", "#GameUI_Inventory_ShowAll", this, "ShowAll" );
	}

	if ( !m_pWeaponFilter )
	{
		m_pWeaponFilter = new vgui::ComboBox( this, "WeaponFilter", 10, false );
		m_pWeaponFilter->AddItem( "#GameUI_Inventory_AllWeapons", NULL );
		m_pWeaponFilter->AddItem( "AK-47", new KeyValues( "weapon", "id", WEAPON_AK47 ) );
		m_pWeaponFilter->AddItem( "M4A1", new KeyValues( "weapon", "id", WEAPON_M4A1 ) );
		m_pWeaponFilter->AddItem( "AWP", new KeyValues( "weapon", "id", WEAPON_AWP ) );
		m_pWeaponFilter->AddItem( "Desert Eagle", new KeyValues( "weapon", "id", WEAPON_DEAGLE ) );
		m_pWeaponFilter->AddItem( "Glock-18", new KeyValues( "weapon", "id", WEAPON_GLOCK ) );
		m_pWeaponFilter->AddItem( "USP-S", new KeyValues( "weapon", "id", WEAPON_USP ) );
		m_pWeaponFilter->AddItem( "P250", new KeyValues( "weapon", "id", WEAPON_P250 ) );
		m_pWeaponFilter->AddItem( "Five-SeveN", new KeyValues( "weapon", "id", WEAPON_FIVESEVEN ) );
		m_pWeaponFilter->AddActionSignalTarget( this );
	}

	if ( !m_pRarityFilter )
	{
		m_pRarityFilter = new vgui::ComboBox( this, "RarityFilter", 10, false );
		m_pRarityFilter->AddItem( "#GameUI_Inventory_AllRarities", NULL );
		m_pRarityFilter->AddItem( "Common", new KeyValues( "rarity", "id", SKIN_RARITY_COMMON ) );
		m_pRarityFilter->AddItem( "Uncommon", new KeyValues( "rarity", "id", SKIN_RARITY_UNCOMMON ) );
		m_pRarityFilter->AddItem( "Rare", new KeyValues( "rarity", "id", SKIN_RARITY_RARE ) );
		m_pRarityFilter->AddItem( "Mythical", new KeyValues( "rarity", "id", SKIN_RARITY_MYTHICAL ) );
		m_pRarityFilter->AddItem( "Legendary", new KeyValues( "rarity", "id", SKIN_RARITY_LEGENDARY ) );
		m_pRarityFilter->AddItem( "Ancient", new KeyValues( "rarity", "id", SKIN_RARITY_ANCIENT ) );
		m_pRarityFilter->AddItem( "Contraband", new KeyValues( "rarity", "id", SKIN_RARITY_CONTRABAND ) );
		m_pRarityFilter->AddActionSignalTarget( this );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CModOptionsSubInventory::PerformLayout()
{
	BaseClass::PerformLayout();

	int wide, tall;
	GetSize( wide, tall );

	if ( wide <= 0 || tall <= 0 )
		return;

	const int padding = 8;
	const int controlHeight = 25;
	int xPos = padding;

	if ( m_pShowAllButton )
	{
		m_pShowAllButton->SetBounds( xPos, padding, 110, controlHeight );
		xPos += 110 + padding;
	}

	if ( m_pWeaponFilter )
	{
		m_pWeaponFilter->SetBounds( xPos, padding, 150, controlHeight );
		xPos += 150 + padding;
	}

	if ( m_pRarityFilter )
	{
		m_pRarityFilter->SetBounds( xPos, padding, 140, controlHeight );
	}

	int contentTop = padding + controlHeight + padding;

	if ( m_pInventoryPanel )
	{
		m_pInventoryPanel->SetBounds( padding, contentTop,
			wide - padding * 2,
			tall - contentTop - padding );
		m_pInventoryPanel->InvalidateLayout( true );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CModOptionsSubInventory::OnCommand( const char *command )
{
	if ( Q_stricmp( command, "ShowAll" ) == 0 )
	{
		if ( m_pInventoryPanel )
		{
			m_pInventoryPanel->ClearFilters();
		}

		if ( m_pWeaponFilter )
			m_pWeaponFilter->ActivateItemByRow( 0 );

		if ( m_pRarityFilter )
			m_pRarityFilter->ActivateItemByRow( 0 );
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CModOptionsSubInventory::OnTextChanged( vgui::Panel *panel )
{
	if ( !m_pInventoryPanel )
		return;

	if ( panel == m_pWeaponFilter )
	{
		KeyValues *pUserData = m_pWeaponFilter->GetActiveItemUserData();

		if ( pUserData )
		{
			CSWeaponID weaponID = (CSWeaponID)pUserData->GetInt( "id", WEAPON_NONE );
			if ( weaponID != WEAPON_NONE )
			{
				m_pInventoryPanel->SetWeaponFilter( weaponID );
			}
			else
			{
				m_pInventoryPanel->ClearFilters();
			}
		}
		else
		{
			m_pInventoryPanel->ClearFilters();
		}
	}
	else if ( panel == m_pRarityFilter )
	{
		KeyValues *pUserData = m_pRarityFilter->GetActiveItemUserData();

		if ( pUserData )
		{
			ESkinRarity rarity = (ESkinRarity)pUserData->GetInt( "id", SKIN_RARITY_COMMON );
			m_pInventoryPanel->SetRarityFilter( rarity );
		}
		else
		{
			m_pInventoryPanel->ClearFilters();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CModOptionsSubInventory::OnResetData()
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CModOptionsSubInventory::OnApplyChanges()
{
}

// Dynamic CSSO 2-style buy wheel, reconstructed from resource and binary behavior.
#ifndef CSTRIKEBUYWHEEL_H
#define CSTRIKEBUYWHEEL_H
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/Button.h>
#include "tier1/utlvector.h"
#include "tier1/utlstring.h"
#include "cs_weapon_parse.h"
class CCSBuyMenu;
class BuyMenuItemIcon;
class CCSBuyWheel;
namespace vgui { class VectorImagePanel; }
class CCSBuyWheelButton : public vgui::Button
{
 DECLARE_CLASS_SIMPLE( CCSBuyWheelButton, vgui::Button );
public:
 CCSBuyWheelButton(CCSBuyWheel *owner, const char *text, const char *command, CSWeaponID weapon, int slot);
 ~CCSBuyWheelButton();
 void SetSector(float start, float end);
 void Configure(KeyValues *settings);
 void OnCursorExited() override;
 vgui::VPANEL IsWithinTraverse(int x, int y, bool traversePopups) override;
 void Paint() override;
 void PaintBackground() override {}
 void OnCursorEntered() override;
 void OnMousePressed(vgui::MouseCode code) override;
 void Activate();
 CSWeaponID Weapon() const { return m_weapon; }
 const char *Command() const { return m_command.String(); }
 void SetDescription(const char *text) { m_description = text; }
 const char *Description() const { return m_description.String(); }
 const char *ItemName() const { return m_text.String(); }
private:
 CCSBuyWheel *m_owner;
 CUtlString m_text, m_command, m_description;
 CSWeaponID m_weapon;
 int m_slot;
 int m_fillTexture;
 int m_texture;
 int m_hoverTexture;
 BuyMenuItemIcon *m_icon;
 float m_start, m_end;
 float m_numberRadius, m_textRadius, m_infoRadius;
 int m_textInsetX, m_textInsetY, m_priceInsetX, m_priceInsetY;
 CUtlString m_fontName, m_priceFontName;
 Color m_textColor, m_priceAvailable, m_priceUnavailable;
};
class CCSBuyWheel : public vgui::EditablePanel
{
 DECLARE_CLASS_SIMPLE( CCSBuyWheel, vgui::EditablePanel );
public:
 CCSBuyWheel(CCSBuyMenu *owner);
 ~CCSBuyWheel();
 void OnCommand(const char *command) override;
 void PerformLayout() override;
 void MainPage();
 void Refresh();
 bool InCategory() const { return m_category.Length() != 0; }
 void SelectSlot(int slot);
 void ShowInfo(CCSBuyWheelButton *button);
 void ClearInfoMessage();
 void ReloadLoadout();
private:
 void Build();
 void AddButton(const char *text, const char *command, CSWeaponID weapon, const char *description = "");
 CCSBuyMenu *m_owner;
 KeyValues *m_items;
 KeyValues *m_defaults;
 KeyValues *m_styles;
 CUtlString m_category;
 CUtlVector<CCSBuyWheelButton*> m_buttons;
 int m_team;
 vgui::VectorImagePanel *m_teamLogo;
};
#endif

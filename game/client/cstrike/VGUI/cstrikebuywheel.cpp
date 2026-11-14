// CSSO 2 buy menu: dynamic sectors with circular hit testing.
#include "cbase.h"
#include "cstrikebuymenu.h"
#include "cstrikebuywheel.h"
#include "cstrikebuydefaults.h"
#include "c_cs_player.h"
#include "cs_loadout.h"
#include "cs_gamerules.h"
#include "filesystem.h"
#include <vgui/ISurface.h>
#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui_controls/VectorImagePanel.h>
#include "tier0/memdbgon.h"

CCSBuyWheelButton::CCSBuyWheelButton(CCSBuyWheel *owner, const char *text, const char *command, CSWeaponID weapon, int slot)
 : BaseClass(owner,"BuySector",""), m_owner(owner), m_text(text), m_command(command), m_weapon(weapon), m_slot(slot), m_fillTexture(-1), m_texture(-1), m_hoverTexture(-1), m_icon(new BuyMenuItemIcon), m_start(0), m_end(360)
{
 SetPaintBorderEnabled(false);
 SetVisible(true);
 SetPaintEnabled(true);
 SetKeyBoardInputEnabled(false);
 SetText("");
 AddActionSignalTarget(owner);
 SetCommand(command);
 if(!command[0]) { SetEnabled(false); SetMouseInputEnabled(false); }
 if (weapon != WEAPON_NONE)
 {
  char path[MAX_PATH];
  const char *alias=WeaponIDToAlias(weapon);
  if (alias)
  {
   // CSSO2 uses its item names in the shared HUD SVG directory (0x1802e7064).
   if(weapon==WEAPON_GLOCK) alias="glock18";
   else if(weapon==WEAPON_SG556) alias="sg553";
   else if(weapon==WEAPON_INCGRENADE) alias="incendiarygrenade";
   Q_snprintf(path,sizeof(path),"materials/vgui/hud/svg/%s.svg",alias);
   m_icon->SetTexture(path,0,48,weapon);
  }
 }
}
CCSBuyWheelButton::~CCSBuyWheelButton() { m_icon->DestroyTexture(); delete m_icon; if(m_fillTexture!=-1) vgui::surface()->DestroyTextureID(m_fillTexture); if(m_texture!=-1) vgui::surface()->DestroyTextureID(m_texture); if(m_hoverTexture!=-1) vgui::surface()->DestroyTextureID(m_hoverTexture); }
void CCSBuyWheelButton::SetSector(float start,float end) { m_start=start; m_end=end; }
void CCSBuyWheelButton::Configure(KeyValues *settings)
{
 const bool item=m_weapon!=WEAPON_NONE;
 m_numberRadius=settings->GetFloat("number_inset_radius",0.2f);
 m_textRadius=settings->GetFloat("text_inset_radius",item?0.64f:0.5f);
 m_infoRadius=settings->GetFloat("info_inset_radius",0.64f);
 m_textInsetX=settings->GetInt("textinsetx",0); m_textInsetY=settings->GetInt("textinsety",item?-46:0);
 m_priceInsetX=settings->GetInt("price_inset_xpos",0); m_priceInsetY=settings->GetInt("price_inset_ypos",48);
 m_fontName=settings->GetString("font",item?"BuyMenuItemButtonFont":"BuyMenuCategoryButtonFont");
 m_priceFontName=settings->GetString("price_font","BuyMenuItemButtonPriceFont");
 m_textColor=settings->FindKey("fgcolor_override")?settings->GetColor("fgcolor_override"):Color(255,255,255,item?160:255);
 if(!Q_stricmp(settings->GetString("fgcolor_override"),"White")) m_textColor=Color(255,255,255,255);
 m_priceAvailable=settings->FindKey("price_fgcolor_available")?settings->GetColor("price_fgcolor_available"):Color(144,238,144,144);
 m_priceUnavailable=settings->FindKey("price_fgcolor_unavailable")?settings->GetColor("price_fgcolor_unavailable"):Color(255,0,0,255);
 // Button owns armed/depressed/released sounds and stay_armed_on_click.
 BaseClass::ApplySettings(settings);
 SetText(""); SetPaintBorderEnabled(false);
 SetArmedSound(settings->GetString("sound_armed","ui/buymenu_rollover.wav"));
 SetReleasedSound(settings->GetString("sound_released","ui/buymenu_submenu_click.wav"));
}
vgui::VPANEL CCSBuyWheelButton::IsWithinTraverse(int x,int y,bool popups)
{
 if(!IsVisible() || !IsMouseInputEnabled() || !m_command.Length()) return 0;
 int sx=x,sy=y; ScreenToLocal(sx,sy);
 if(GetWide()<=0 || GetTall()<=0) return 0;
 float nx=(sx-GetWide()*0.5f)/(GetWide()*0.5f), ny=(sy-GetTall()*0.5f)/(GetTall()*0.5f);
 if(nx*nx+ny*ny>1.0f) return 0;
 float angle=RAD2DEG(atan2f(ny,nx));
 while(angle<m_start) angle+=360.0f;
 if(angle>=m_end) return 0;
 return BaseClass::IsWithinTraverse(x,y,popups);
}
static void WheelText(vgui::HFont font,const char *text,int x,int y,Color color)
{
 wchar_t converted[512];
 const wchar_t *value=text[0]=='#' ? g_pVGuiLocalize->Find(text) : NULL;
 if(!value) { g_pVGuiLocalize->ConvertANSIToUnicode(text,converted,sizeof(converted)); value=converted; }
 int wide,tall; vgui::surface()->GetTextSize(font,value,wide,tall);
 vgui::surface()->DrawSetTextFont(font); vgui::surface()->DrawSetTextColor(color);
 vgui::surface()->DrawSetTextPos(x-wide/2,y-tall/2); vgui::surface()->DrawPrintText(value,wcslen(value));
}
// CSSO2 CircularButton, 0x18054cbf0 / 0x18054d570:
// triangle_uv maps each sector's outer arc to (0..1, 0), center to (0.5, 1).
// The source builds floor(degrees / 2) triangles, not a disk-space UV fan.
static void DrawBuySector(float cx,float cy,float start,float end)
{
 const int count=MAX(1,(int)(end-start)/2);
 for(int i=0;i<count;++i)
 {
  float a=DEG2RAD(start+2.0f*i);
  float b=DEG2RAD(i==count-1?end:start+2.0f*(i+1));
  vgui::Vertex_t triangle[3];
  triangle[0].Init(Vector2D(cx+cosf(a)*cx,cy+sinf(a)*cy),Vector2D((float)i/count,0));
  triangle[1].Init(Vector2D(cx+cosf(b)*cx,cy+sinf(b)*cy),Vector2D((float)(i+1)/count,0));
  triangle[2].Init(Vector2D(cx,cy),Vector2D(0.5f,1));
  vgui::surface()->DrawTexturedPolygon(3,triangle);
 }
}
void CCSBuyWheelButton::Paint()
{
 if(GetWide()<=0 || GetTall()<=0) return;
 const float cx=GetWide()*0.5f,cy=GetTall()*0.5f;
 // A white texture is used only as a fallback; the artwork keeps its alpha.
 if(m_fillTexture==-1)
 {
  m_fillTexture=vgui::surface()->CreateNewTextureID(true);
  const unsigned char white[4]={255,255,255,255};
  vgui::surface()->DrawSetTextureRGBA(m_fillTexture,white,1,1,1,false);
 }
 if(m_texture==-1)
 {
  m_texture=vgui::surface()->CreateNewTextureID(); m_hoverTexture=vgui::surface()->CreateNewTextureID();
  vgui::surface()->DrawSetTextureFile(m_texture,"vgui/ui/buymenu_button",1,false);
  vgui::surface()->DrawSetTextureFile(m_hoverTexture,"vgui/ui/buymenu_button_hover",1,false);
 }
 int artwork=(IsArmed() || IsDepressed() || IsSelected())?m_hoverTexture:m_texture;
 if(vgui::surface()->IsTextureIDValid(artwork))
 {
  vgui::surface()->DrawSetTexture(artwork);
  vgui::surface()->DrawSetColor(255,255,255,248);
 }
 else
 {
  vgui::surface()->DrawSetTexture(m_fillTexture);
  vgui::surface()->DrawSetColor(IsArmed()?Color(85,95,110,240):Color(28,32,39,235));
 }
 DrawBuySector(cx,cy,m_start,m_end);
 if(!m_command.Length()) return;
 float mid=DEG2RAD((m_start+m_end)*0.5f);
 const float scale=GetTall()/612.0f;
 int x=cx+cosf(mid)*cx*m_infoRadius,y=cy+sinf(mid)*cy*m_infoRadius;
 vgui::IScheme *scheme=vgui::scheme()->GetIScheme(GetScheme());
 vgui::HFont font=scheme->GetFont(m_fontName.String(),true);
 char number[12]; Q_snprintf(number,sizeof(number),"%d",m_slot);
 WheelText(font,number,cx+cosf(mid)*cx*m_numberRadius,cy+sinf(mid)*cy*m_numberRadius,m_textColor);
 WheelText(font,m_text.String(),cx+cosf(mid)*cx*m_textRadius+m_textInsetX*scale,
  cy+sinf(mid)*cy*m_textRadius+m_textInsetY*scale,m_textColor);
 if(m_weapon!=WEAPON_NONE)
 {
  C_CSPlayer *player=C_CSPlayer::GetLocalCSPlayer();
  CCSWeaponInfo *info=m_weapon<WEAPON_KNIFE ? GetWeaponInfo(m_weapon) : NULL;
  int price=info?info->GetWeaponPrice():m_weapon==ITEM_KEVLAR?ITEM_PRICE_KEVLAR:m_weapon==ITEM_ASSAULTSUIT?(player&&player->ArmorValue()>=100?ITEM_PRICE_HELMET:ITEM_PRICE_ASSAULTSUIT):m_weapon==ITEM_DEFUSER?ITEM_PRICE_DEFUSEKIT:0;
  bool allowed=player && player->CanAcquire(m_weapon,AcquireMethod::Buy)==AcquireResult::Allowed && (mp_maxmoney.GetInt()<=0 || player->GetAccount()>=price);
  char money[32]; Q_snprintf(money,sizeof(money),"%d",price);
  wchar_t amount[32],formatted[128];
  g_pVGuiLocalize->ConvertANSIToUnicode(money,amount,sizeof(amount));
  const wchar_t *format=g_pVGuiLocalize->Find("#CStrike_BuyMenu_PriceFormatting");
  g_pVGuiLocalize->ConstructString(formatted,sizeof(formatted),format?format:L"$%s1",1,amount);
  vgui::HFont priceFont=scheme->GetFont(m_priceFontName.String(),true);
  int tw,th; vgui::surface()->GetTextSize(priceFont,formatted,tw,th);
  vgui::surface()->DrawSetTextFont(priceFont);
  vgui::surface()->DrawSetTextColor(allowed?m_priceAvailable:m_priceUnavailable);
  vgui::surface()->DrawSetTextPos(x+m_priceInsetX*scale-tw/2,y+m_priceInsetY*scale-th/2);
  vgui::surface()->DrawPrintText(formatted,wcslen(formatted));
  if(m_icon->m_nTextureID!=-1)
  {
   int w=m_icon->m_iTextureSize[0]*scale,h=m_icon->m_iTextureSize[1]*scale;
   vgui::surface()->DrawSetTexture(m_icon->m_nTextureID); vgui::surface()->DrawSetColor(m_textColor);
   vgui::surface()->DrawTexturedSubRect(x-w/2,y-h/2,x+w/2,y+h/2,m_icon->m_flTextureCoords[0],m_icon->m_flTextureCoords[1],m_icon->m_flTextureCoords[2],m_icon->m_flTextureCoords[3]);
  }
 }
}
void CCSBuyWheelButton::OnCursorEntered() { BaseClass::OnCursorEntered(); m_owner->ShowInfo(this); }
void CCSBuyWheelButton::OnCursorExited() { BaseClass::OnCursorExited(); m_owner->ClearInfoMessage(); }
void CCSBuyWheelButton::OnMousePressed(vgui::MouseCode code) { m_owner->ShowInfo(this); BaseClass::OnMousePressed(code); }
void CCSBuyWheelButton::Activate() { if(m_command.Length()) m_owner->OnCommand(m_command.String()); }

CCSBuyWheel::CCSBuyWheel(CCSBuyMenu *owner) : BaseClass(owner,"BuyPanel"),m_owner(owner),m_items(new KeyValues("PlayerLoadout")),m_defaults(new KeyValues("LoadoutConfig")),m_styles(new KeyValues("BuyWheelStyles")),m_team(0)
{
 SetPaintBackgroundEnabled(false); SetProportional(true); SetZPos(5);
 m_styles->LoadFromFile(g_pFullFileSystem,"resource/ui/buymenu_csso2_buttons.res","GAME");
 if(!m_items->LoadFromFile(g_pFullFileSystem,"scripts/playerloadout.txt","GAME") || !m_items->GetFirstTrueSubKey())
 {
  Warning("Buy menu: scripts/playerloadout.txt missing or empty; using built-in categories.\n");
  m_items->Clear();
  m_items->LoadFromBuffer("BuiltInBuyMenu",kBuyMenuDefaultItems);
 }
 m_teamLogo=new vgui::VectorImagePanel(this,"TeamLogoImagePanel");
 m_teamLogo->SetMouseInputEnabled(false); m_teamLogo->SetKeyBoardInputEnabled(false); m_teamLogo->SetZPos(10);
 ReloadLoadout();
}
CCSBuyWheel::~CCSBuyWheel() { m_items->deleteThis(); m_defaults->deleteThis(); m_styles->deleteThis(); }
void CCSBuyWheel::AddButton(const char *text,const char *command,CSWeaponID weapon,const char *description)
{
 CCSBuyWheelButton *button=new CCSBuyWheelButton(this,text,command,weapon,m_buttons.Count()+1);
 button->Configure(m_styles->FindKey(weapon==WEAPON_NONE?"BuyCategoryButton":"BuyItemButton",true));
 button->SetDescription(description); m_buttons.AddToTail(button);
}
void CCSBuyWheel::ReloadLoadout()
{
 m_defaults->Clear();
 if(!m_defaults->LoadFromFile(g_pFullFileSystem,"cfg/loadout.txt","GAME") || !m_defaults->GetFirstTrueSubKey())
 {
  m_defaults->Clear();
  if(!m_defaults->LoadFromFile(g_pFullFileSystem,"cfg/loadout_default.txt","GAME") || !m_defaults->GetFirstTrueSubKey())
  {
   m_defaults->Clear(); m_defaults->LoadFromBuffer("BuiltInLoadout",kBuyMenuDefaultLoadout);
  }
 }
}
void CCSBuyWheel::MainPage() { m_category=""; Build(); }
void CCSBuyWheel::Refresh() { C_CSPlayer *p=C_CSPlayer::GetLocalCSPlayer(); if(p && p->GetTeamNumber()!=m_team) MainPage(); }
void CCSBuyWheel::Build()
{
 for(int i=0;i<m_buttons.Count();++i) { m_buttons[i]->SetVisible(false); m_buttons[i]->SetParent((vgui::Panel*)NULL); m_buttons[i]->MarkForDeletion(); }
 m_buttons.RemoveAll();
 C_CSPlayer *player=C_CSPlayer::GetLocalCSPlayer(); if(!player) return;
 m_team=player->GetTeamNumber();
 if(!InCategory())
 {
  for(KeyValues *cat=m_items->GetFirstTrueSubKey();cat;cat=cat->GetNextTrueSubKey())
  { char command[128]; Q_snprintf(command,sizeof(command),"category %s",cat->GetName()); AddButton(cat->GetString("name"),command,WEAPON_NONE); }
 }
 else
 {
  KeyValues *cat=m_items->FindKey(m_category.String());
  KeyValues *team=m_defaults->FindKey(m_team==TEAM_CT?"loadout_ct":"loadout_t");
  KeyValues *selected=team?team->FindKey(m_category.String()):NULL;
  // Non-editable categories always expose all team items; loadout flags only filter editable categories.
  if(cat && cat->GetInt("numitemsrequired",1)==0) selected=NULL;
  if(cat) for(KeyValues *item=cat->GetFirstTrueSubKey();item;item=item->GetNextTrueSubKey())
  {
   if(item->GetInt("teamnumber") && item->GetInt("teamnumber")!=m_team) continue;
   if(selected && !selected->GetInt(item->GetName()) &&
      !( !Q_stricmp(item->GetName(),"glock") && selected->GetInt("glock18") ) &&
      !( !Q_stricmp(item->GetName(),"sg556") && selected->GetInt("sg553") )) continue;
   const char *baseAlias=item->GetName();
   if(!Q_stricmp(baseAlias,"glock18")) baseAlias="glock";
   if(!Q_stricmp(baseAlias,"sg553")) baseAlias="sg556";
   if(!Q_stricmp(baseAlias,"incendiarygrenade")) baseAlias="incgrenade";
   const char *alias=CSLoadout()->GetLoadoutWeapon(player,baseAlias);
   CSWeaponID id=AliasToWeaponID(alias); if(id==WEAPON_NONE) continue;
   char command[128]; Q_snprintf(command,sizeof(command),"buy %s",alias);
   const char *name=item->GetString("name");
   const char *description=item->GetString("description");
   // A selected legacy loadout slot can resolve to a different weapon.
   CCSWeaponInfo *info=id<WEAPON_KNIFE?GetWeaponInfo(id):NULL;
   if(info) { name=info->szPrintName; description=info->m_szDescription; }
   AddButton(name,command,id,description);
  }
 }
 // Categories pad odd counts above one (0x1802e7325); items pad every odd count (0x1802e771a).
 if(m_buttons.Count()%2 && (InCategory() || m_buttons.Count()>1)) AddButton("","",WEAPON_NONE);
 m_teamLogo->SetTexture(m_team==TEAM_CT?"materials/vgui/hud/svg/ct_logo.svg":"materials/vgui/hud/svg/t_logo.svg");
 m_owner->InvalidateLayout(true);
 InvalidateLayout(true);
 DevMsg("Buy menu: team=%d category=%s sectors=%d bounds=%d,%d %dx%d\n",m_team,InCategory()?m_category.String():"main",m_buttons.Count(),GetXPos(),GetYPos(),GetWide(),GetTall());
}
void CCSBuyWheel::PerformLayout()
{
 BaseClass::PerformLayout();
 int logoSize=MAX(1,GetTall()*90/612);
 m_teamLogo->SetBounds((GetWide()-logoSize)/2,(GetTall()-logoSize)/2,logoSize,logoSize);
 int n=m_buttons.Count(); if(!n) return;
 int step=360/n,base=(n==4?315:270)-step;
 for(int i=0;i<n;++i) { m_buttons[i]->SetBounds(0,0,GetWide(),GetTall()); m_buttons[i]->SetSector(base+step*i,i==n-1?base+360:base+step*(i+1)); }
}
void CCSBuyWheel::SelectSlot(int slot) { if(slot>0 && slot<=m_buttons.Count()) m_buttons[slot-1]->Activate(); }
void CCSBuyWheel::OnCommand(const char *command)
{
 if(!Q_strncmp(command,"category ",9)) { m_category=command+9; Build(); return; }
 if(!Q_stricmp(command,"buymenu")) { MainPage(); return; }
 m_owner->OnCommand(command);
}
void CCSBuyWheel::ClearInfoMessage() { m_owner->HideSpecialMessage(PerWeaponMessage); }
void CCSBuyWheel::ShowInfo(CCSBuyWheelButton *button)
{
 C_CSPlayer *player=C_CSPlayer::GetLocalCSPlayer(); if(!player || button->Weapon()==WEAPON_NONE) return;
 CSWeaponID id=button->Weapon();
 CCSWeaponInfo *info=id<WEAPON_KNIFE?GetWeaponInfo(id):NULL;
 const char *model=NULL,*sequence="t_buymenu_nowep";
 if(info) { model=info->szWorldModel; sequence=m_team==TEAM_TERRORIST?info->m_szBuyMenuAnimT:info->m_szBuyMenuAnim; }
 else if(id==ITEM_DEFUSER) { model="models/weapons/w_defuser.mdl"; sequence="t_buymenu_defuser"; }
 else if(id==ITEM_KEVLAR || id==ITEM_ASSAULTSUIT) { model=id==ITEM_KEVLAR?"models/weapons/w_eq_armor.mdl":"models/weapons/w_eq_armor_helmet.mdl"; sequence="t_buymenu_armor_helmet"; }
 m_owner->SetWheelPreviewWeapon(model,sequence,id);
 m_owner->SetItemNameAndDescription(button->ItemName(),button->Description());
 m_owner->HideSpecialMessage(PerWeaponMessage);
 AcquireResult::Type result=player->CanAcquire(id,AcquireMethod::Buy);
 const char *message=NULL;
 switch(result)
 {
  case AcquireResult::AlreadyOwned: message="#CStrike_BuyMenu_AlreadyCarrying"; break;
  case AcquireResult::AlreadyPurchased: message="#CStrike_BuyMenu_AlreadyPurchased"; break;
  case AcquireResult::NotAllowedByTeam: message="#CStrike_BuyMenu_NotAvailable"; break;
  case AcquireResult::NotAllowedByMap: message="#CStrike_BuyMenu_NotAvailable"; break;
  case AcquireResult::NotAllowedForPurchase: message="#CStrike_BuyMenu_NotAvailable"; break;
  default: break;
 }
 if(message) m_owner->ShowSpecialMessage(message,PerWeaponMessage);
}

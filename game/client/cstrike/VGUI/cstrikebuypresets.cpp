// Team-separated favorites following the CS:GO 2017 port's BuyPresets files
// and console commands. Resolve aliases through CSSO loadout at purchase time.
#include "cbase.h"
#include "c_cs_player.h"
#include "weapon_csbase.h"
#include "cs_loadout.h"
#include "cs_gamerules.h"
#include "filesystem.h"
#include "tier1/KeyValues.h"
#include "tier0/memdbgon.h"

static const char *FavoriteFile( C_CSPlayer *player )
{
 return player->GetTeamNumber()==TEAM_CT ? "cfg/BuyWheelFavorites_CT.vdf" : "cfg/BuyWheelFavorites_TER.vdf";
}
static bool FavoritePlayer( C_CSPlayer *player )
{
 return player && (player->GetTeamNumber()==TEAM_CT || player->GetTeamNumber()==TEAM_TERRORIST);
}
CON_COMMAND( cl_buy_favorite_set, "Save current equipment to buy-wheel favorite 1-4." )
{
 C_CSPlayer *player=C_CSPlayer::GetLocalCSPlayer();
 if (!FavoritePlayer(player) || args.ArgC()!=2) return;
 int slot=atoi(args[1]); if(slot<1 || slot>4) return;
 KeyValues *data=new KeyValues("BuyWheelFavorites");
 data->LoadFromFile(g_pFullFileSystem,FavoriteFile(player),"GAME");
 char name[8];Q_snprintf(name,sizeof(name),"%d",slot);
 KeyValues *preset=data->FindKey(name,true);
 preset->Clear();
 for(int i=0;i<player->WeaponCount();++i)
 {
  C_WeaponCSBase *weapon=dynamic_cast<C_WeaponCSBase*>(player->GetWeapon(i));
  if(!weapon) continue;
  const CSWeaponID id=weapon->GetCSWeaponID();
  if (weapon->GetSlot()!=WEAPON_SLOT_RIFLE && weapon->GetSlot()!=WEAPON_SLOT_PISTOL && weapon->GetSlot()!=WEAPON_SLOT_GRENADES && id!=WEAPON_TASER) continue;
  int count=1;
  if(weapon->GetSlot()==WEAPON_SLOT_GRENADES) count=player->GetAmmoCount(weapon->GetPrimaryAmmoType());
  preset->SetInt(WeaponIDToAlias(id),clamp(count,0,2));
 }
 preset->SetInt("assaultsuit",player->HasHelmet() ? 1 : 0);
 preset->SetInt("kevlar",!player->HasHelmet() && player->ArmorValue()>0 ? 1 : 0);
 preset->SetInt("defuser",player->HasDefuser() ? 1 : 0);
 g_pFullFileSystem->CreateDirHierarchy("cfg","GAME");
 if(!data->SaveToFile(g_pFullFileSystem,FavoriteFile(player),"GAME")) Warning("Could not save buy-wheel favorite.\n");
 data->deleteThis();
}
CON_COMMAND( cl_buy_favorite, "Buy saved equipment from buy-wheel favorite 1-4." )
{
 C_CSPlayer *player=C_CSPlayer::GetLocalCSPlayer();
 if(!FavoritePlayer(player) || !player->IsAlive() || !player->IsInBuyZone() || CSGameRules()->IsBuyTimeElapsed() || args.ArgC()!=2) return;
 int slot=atoi(args[1]);if(slot<1 || slot>4)return;
 KeyValues *data=new KeyValues("BuyWheelFavorites");
 data->LoadFromFile(g_pFullFileSystem,FavoriteFile(player),"GAME");
 char name[8];Q_snprintf(name,sizeof(name),"%d",slot);
 KeyValues *preset=data->FindKey(name);
 if(!preset) { Msg("Favorite %d is empty. Use cl_buy_favorite_set %d to save equipment.\n",slot,slot);data->deleteThis();return; }
 CUtlString commands;
 for(KeyValues *item=preset->GetFirstSubKey();item;item=item->GetNextKey())
 {
  const char *alias=item->GetName();
  // Read favorites saved before the CSSO armor alias correction.
  if (!Q_stricmp(alias,"vesthelm")) alias="assaultsuit";
  else if (!Q_stricmp(alias,"vest")) alias="kevlar";
  // Persisted files are data: whitelist aliases before producing console commands.
  const CSWeaponID stored=AliasToWeaponID(alias);
  if(stored==WEAPON_NONE)continue;
  alias=CSLoadout()->GetLoadoutWeapon(player,alias);
  const CSWeaponID id=AliasToWeaponID(alias);
  if(player->CanAcquire(id,AcquireMethod::Buy)!=AcquireResult::Allowed)continue;
  int count=clamp(item->GetInt(),0,2);
  if(id==WEAPON_FLASHBANG)
  {
   CCSWeaponInfo *info=GetWeaponInfo(id);
   if(info)count=MAX(0,count-player->GetAmmoCount(info->iAmmoType));
  }
  else if(player->GetCSWeapon(id))count=0;
  for(int n=0;n<count;++n) { commands+="buy ";commands+=alias;commands+="\n"; }
 }
 if(commands.Length())engine->ClientCmd(commands.String());
 data->deleteThis();
}

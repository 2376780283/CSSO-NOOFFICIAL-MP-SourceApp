//========= Copyright Valve Corporation, All rights reserved. ============//
#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "c_cs_player.h"
#include "cs_client_gamestats.h"
#include "cs_gamerules.h"
#include "cs_hud_scaledpanel.h"
#include "filesystem.h"
#include "VGuiMatSurface/IMatSystemSurface.h"
#include "lunasvg/lunasvg.h"
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui/IScheme.h>
#include <vgui_controls/AnimationController.h>

ConVar cl_hud_scale("cl_hud_scale", "1.0", FCVAR_ARCHIVE, "Scale the main HUD panels (0.5 to 2.0).", true,
					0.5f, true, 2.0f);
extern ConVar cl_hud_healthammo_style;
extern ConVar cl_hud_background_alpha;
extern ConVar cl_hud_color;
extern ConVar cl_draw_only_deathnotices;
using namespace vgui;

// Local textures allow arbitrary animated bounds without changing global SVG panel behavior.
class CAmmoSVG
{
  public:
	CAmmoSVG() : texture(-1), width(0), height(0), rasterHeight(0)
	{
		path[0] = 0;
	}
	~CAmmoSVG()
	{
		if (texture != -1)
			surface()->DestroyTextureID(texture);
	}
	void Load(const char *file, int tall)
	{
		tall = MAX(1, tall);
		if (!Q_strcmp(path, file) && rasterHeight == tall)
			return;
		Q_strncpy(path, file, sizeof(path));
		rasterHeight = tall;
		width = height = 0;
		CUtlBuffer data;
		if (!filesystem->ReadFile(file, "GAME", data))
			return;
		data.PutChar(0);
		auto doc = lunasvg::Document::loadFromData((const char *)data.Base());
		if (!doc)
			return;
		auto bitmap = doc->renderToBitmap(0, tall);
		if (!bitmap.valid())
			return;
		width = bitmap.width();
		height = bitmap.height();
		if (texture == -1)
			texture = surface()->CreateNewTextureID(true);
		surface()->DrawSetTextureRGBA(texture, bitmap.data(), width, height, 1, true);
	}
	void Draw(int x, int y, int wide, int tall, Color color)
	{
		if (texture == -1 || width <= 0 || height <= 0 || color.a() == 0)
			return;
		int tw, th;
		surface()->DrawGetTextureSize(texture, tw, th);
		if (tw <= 0 || th <= 0)
			return;
		surface()->DrawSetTexture(texture);
		surface()->DrawSetColor(color);
		surface()->DrawTexturedSubRect(x, y, x + wide, y + tall, 0, 0, (float)width / tw, (float)height / th);
	}
	int width, height;

  private:
	int texture, rasterHeight;
	char path[128];
};

class CHudAmmo : public CHudElement, public CCSHudScaledPanel
{
	DECLARE_CLASS_SIMPLE(CHudAmmo, CCSHudScaledPanel);

  public:
	CHudAmmo(const char *name);
	~CHudAmmo()
	{
		delete m_animation;
		m_layout->deleteThis();
	}
	void Init() OVERRIDE;
	void Reset() OVERRIDE;
	void ApplySettings(KeyValues *settings) OVERRIDE;
	void OnScreenSizeChanged(int oldWide, int oldTall) OVERRIDE;
	void OnThink() OVERRIDE;
	void Paint() OVERRIDE;
	bool ShouldDraw() OVERRIDE;
	void FireGameEvent(IGameEvent *event) OVERRIDE;

  private:
	void ResetKills();
	void PlayAmmoAnimation(bool kill);
	void LoadImages();
	int V(const char *key, int fallback = 0) const;
	Color C(const char *key, Color fallback);
	int Text(HFont font, const wchar_t *text, int x, int y, Color color, bool right = false);
	void KillIcon(CHudTexture *icon, int x, int y, bool animate);
	KeyValues *m_layout;
	AnimationController *m_animation;
	CHandle<C_WeaponCSBase> m_weapon;
	int m_clip, m_reserve, m_exhaustibleCount, m_burst;
	int m_normalKills, m_headshotKills;
	bool m_lastHeadshot, m_usesClips, m_exhaustible, m_hasWeapon;
	int m_original[4], m_simple[4], m_screenTall;
	HFont m_clipFont, m_reserveFont, m_killFont, m_statFont;
	HFont m_defaultFonts[4];
	CAmmoSVG m_bullet, m_shell, m_exhaustibleImage, m_burstImage[2];
	CPanelAnimationVar(float, m_bulletOffset, "bullet_icon_xoffset", "0");
	CPanelAnimationVar(float, m_lastAlpha, "bullet_icon_last_alpha", "1");
	CPanelAnimationVar(float, m_shellX, "bullet_shell_icon_xoffset", "0");
	CPanelAnimationVar(float, m_shellY, "bullet_shell_icon_yoffset", "0");
	CPanelAnimationVar(float, m_shellAlpha, "bullet_shell_icon_alpha", "0");
	CPanelAnimationVar(float, m_killScale, "kill_icon_first_scale", "1");
	CPanelAnimationVar(float, m_killAlpha, "kill_icon_first_alpha", "1");
};
DECLARE_HUDELEMENT(CHudAmmo);

CHudAmmo::CHudAmmo(const char *name) : CHudElement(name), CCSHudScaledPanel(NULL, "HudAmmo")
{
	SetParent(g_pClientMode->GetViewport());
	SetScheme(scheme()->GetScheme("ClientScheme"));
	SetHudScaleAnchor(1, 1);
	SetHiddenBits(HIDEHUD_WEAPONSELECTION | HIDEHUD_NOT_OBSERVING_PLAYERS);
	SetMouseInputEnabled(false);
	SetKeyBoardInputEnabled(false);
	m_layout = new KeyValues("HudAmmo");
	// Viewport scheme reloads clear its animation scripts; Ammo owns its sequences.
	m_animation = new AnimationController(this);
	Q_memset(m_original, 0, sizeof(m_original));
	Q_memset(m_simple, 0, sizeof(m_simple));
	m_screenTall = 0;
	m_clipFont = m_reserveFont = m_killFont = m_statFont = INVALID_FONT;
	for (int i = 0; i < ARRAYSIZE(m_defaultFonts); ++i)
		m_defaultFonts[i] = INVALID_FONT;
	Reset();
}
void CHudAmmo::Init()
{
	ListenForGameEvent("player_death");
	ListenForGameEvent("player_spawn");
	const char *names[] = {"HUDAmmoKillIcon", "HUDAmmoKillHeadshotIcon"};
	const char *files[] = {"vgui/hud/kill_icon", "vgui/hud/kill_headshot_icon"};
	for (int i = 0; i < ARRAYSIZE(names); ++i)
	{
		if (gHUD.GetIcon(names[i]))
			continue;
		CHudTexture icon;
		Q_strncpy(icon.szShortName, names[i], sizeof(icon.szShortName));
		Q_strncpy(icon.szTextureFile, files[i], sizeof(icon.szTextureFile));
		icon.bRenderUsingFont = false;
		icon.rc.left = icon.rc.top = 0;
		icon.rc.right = icon.rc.bottom = 32;
		gHUD.AddSearchableHudIconToList(icon);
	}
	m_animation->SetScriptFile(g_pClientMode->GetViewport()->GetVPanel(),
							   "scripts/hudanimations_ammo_csso2.txt", true);
	Reset();
}
void CHudAmmo::PlayAmmoAnimation(bool kill)
{
	if (m_animation->StartAnimationSequence(g_pClientMode->GetViewport(),
											kill ? "HUDAmmoKill" : "HUDAmmoShoot"))
		return;
	// Older Android resource deployments may lack the extra script. Keep the same
	// timings functional using registered animation properties in that case.
	if (kill)
	{
		m_killScale = 0.4f;
		m_killAlpha = 0;
		m_animation->RunAnimationCommand(this, "kill_icon_first_scale", 1.25f, 0, 0.13f,
										 AnimationController::INTERPOLATOR_LINEAR);
		m_animation->RunAnimationCommand(this, "kill_icon_first_scale", 1.0f, 0.13f, 0.2f,
										 AnimationController::INTERPOLATOR_LINEAR, 0, false);
		m_animation->RunAnimationCommand(this, "kill_icon_first_alpha", 1.0f, 0, 0.13f,
										 AnimationController::INTERPOLATOR_LINEAR);
	}
	else
	{
		m_bulletOffset = -10;
		m_lastAlpha = 0;
		m_shellX = m_shellY = 0;
		m_shellAlpha = 1;
		m_animation->RunAnimationCommand(this, "bullet_icon_xoffset", 0.0f, 0, 0.13f,
										 AnimationController::INTERPOLATOR_LINEAR);
		m_animation->RunAnimationCommand(this, "bullet_icon_last_alpha", 1.0f, 0, 0.13f,
										 AnimationController::INTERPOLATOR_LINEAR);
		m_animation->RunAnimationCommand(this, "bullet_shell_icon_xoffset", 120.0f, 0, 0.5f,
										 AnimationController::INTERPOLATOR_LINEAR);
		m_animation->RunAnimationCommand(this, "bullet_shell_icon_yoffset", 120.0f, 0, 0.5f,
										 AnimationController::INTERPOLATOR_ACCEL);
		m_animation->RunAnimationCommand(this, "bullet_shell_icon_alpha", 0.0f, 0.125f, 0.375f,
										 AnimationController::INTERPOLATOR_LINEAR);
	}
}
void CHudAmmo::ResetKills()
{
	m_normalKills = m_headshotKills = 0;
	m_lastHeadshot = false;
	m_killScale = m_killAlpha = 1;
}
void CHudAmmo::Reset()
{
	m_animation->CancelAllAnimations();
	m_weapon = NULL;
	m_clip = m_reserve = m_exhaustibleCount = 0;
	m_burst = -1;
	m_usesClips = m_exhaustible = m_hasWeapon = false;
	m_bulletOffset = m_shellX = m_shellY = m_shellAlpha = 0;
	m_lastAlpha = 1;
	ResetKills();
	SetPaintEnabled(false);
	SetPaintBackgroundEnabled(false);
}
void CHudAmmo::FireGameEvent(IGameEvent *event)
{
	C_CSPlayer *local = C_CSPlayer::GetLocalCSPlayer();
	if (!local)
		return;
	const int user = local->GetUserID();
	if (!Q_strcmp(event->GetName(), "player_spawn"))
	{
		if (event->GetInt("userid") == user)
			Reset();
		return;
	}
	if (event->GetInt("userid") == user)
	{
		m_animation->StopAnimationSequence(g_pClientMode->GetViewport(), "HUDAmmoKill");
		ResetKills();
		return;
	}
	if (event->GetInt("attacker") != user)
		return;
	bool teamkill = event->GetBool("teamkill");
	// Plus servers do not send teamkill. Derive it using the same enemy relation as gameplay.
	if (event->IsEmpty("teamkill"))
	{
		C_CSPlayer *victim =
			ToCSPlayer(UTIL_PlayerByIndex(engine->GetPlayerForUserID(event->GetInt("userid"))));
		if (!victim)
			return;
		teamkill = !local->IsOtherEnemy(victim);
	}
	if (teamkill)
		return;
	m_lastHeadshot = event->GetBool("headshot");
	if (m_lastHeadshot)
		++m_headshotKills;
	else
		++m_normalKills;
	PlayAmmoAnimation(true);
}
int CHudAmmo::V(const char *key, int fallback) const
{
	int sw, sh;
	surface()->GetScreenSize(sw, sh);
	return (int)(m_layout->GetFloat(key, fallback) * sh / 1080.0f);
}
Color CHudAmmo::C(const char *key, Color fallback)
{
	const char *value = m_layout->GetString(key, "");
	if (!*value)
		return fallback;
	if ((*value >= '0' && *value <= '9') || *value == '-')
		return m_layout->GetColor(key);
	return scheme()->GetIScheme(GetScheme())->GetColor(value, fallback);
}
void CHudAmmo::ApplySettings(KeyValues *settings)
{
	KeyValues *copy = settings->MakeCopy();
	copy->SetBool("skip_autoresize", true);
	BaseClass::ApplySettings(copy);
	copy->deleteThis();
	m_layout->deleteThis();
	m_layout = settings->MakeCopy();
	GetBounds(m_original[0], m_original[1], m_original[2], m_original[3]);
	int sw, sh;
	surface()->GetScreenSize(sw, sh);
	m_simple[2] = V("simple_wide", 282);
	m_simple[3] = V("simple_tall", 54);
	ComputePos(this, settings->GetString("simple_xpos", "r282"), m_simple[0], m_simple[2], sw, 1920, 1080,
			   true, OP_SET);
	ComputePos(this, settings->GetString("simple_ypos", "r54"), m_simple[1], m_simple[3], sh, 1920, 1080,
			   false, OP_SET);
	m_screenTall = 0;
}
void CHudAmmo::OnScreenSizeChanged(int oldWide, int oldTall)
{
	BaseClass::OnScreenSizeChanged(oldWide, oldTall);
	m_screenTall = 0;
}
void CHudAmmo::LoadImages()
{
	m_bullet.Load("materials/vgui/hud/svg/bullet.svg", V("bullet_icon_tall", 16));
	m_shell.Load("materials/vgui/hud/svg/bullet_shell.svg", V("bullet_icon_tall", 16));
	m_burstImage[0].Load("materials/vgui/hud/svg/bullet_burst_outline.svg", V("burst_icon_wide", 20));
	m_burstImage[1].Load("materials/vgui/hud/svg/bullet_burst.svg", V("burst_icon_wide", 20));
	int sw, sh;
	surface()->GetScreenSize(sw, sh);
	if (m_screenTall != sh)
	{
		m_screenTall = sh;
		HFont *fonts[] = {&m_clipFont, &m_reserveFont, &m_killFont, &m_statFont};
		const int sizes[] = {52, 25, 28, 16};
		const char *keys[] = {"clip_font", "reserve_font", "kill_count_font", "stattrak_font"};
		for (int i = 0; i < ARRAYSIZE(fonts); ++i)
		{
			if (m_defaultFonts[i] == INVALID_FONT)
				m_defaultFonts[i] = surface()->CreateFont();
			*fonts[i] = m_defaultFonts[i];
			if (!surface()->SetFontGlyphSet(*fonts[i], "Stratum2 Bold",
											MAX(1, (int)(sizes[i] * sh / 1080.0f)), 0, 0, 0,
											ISurface::FONTFLAG_ANTIALIAS))
				surface()->SetFontGlyphSet(*fonts[i], "Tahoma", MAX(1, (int)(sizes[i] * sh / 1080.0f)), 700,
										   0, 0, ISurface::FONTFLAG_ANTIALIAS);
			const char *custom = m_layout->GetString(keys[i], "");
			if (*custom)
			{
				HFont font = scheme()->GetIScheme(GetScheme())->GetFont(custom, IsProportional());
				if (font != INVALID_FONT)
					*fonts[i] = font;
			}
		}
	}
}
void CHudAmmo::OnThink()
{
	m_animation->UpdateAnimations(gpGlobals->curtime);
	const int *bounds = cl_hud_healthammo_style.GetBool() ? m_simple : m_original;
	SetBounds(bounds[0], bounds[1], bounds[2], bounds[3]);
	Color bg = C("bgcolor_override", Color(0, 0, 0, 255));
	bg[3] = clamp((int)(cl_hud_background_alpha.GetFloat() * 255), 0, 255);
	SetBgColor(bg);
	C_CSPlayer *player = GetHudPlayer();
	C_WeaponCSBase *weapon = player ? player->GetActiveCSWeapon() : NULL;
	m_hasWeapon = weapon != NULL;
	SetPaintEnabled(m_hasWeapon);
	if (!weapon)
	{
		m_weapon = NULL;
		SetPaintBackgroundEnabled(false);
		return;
	}
	const bool changed = weapon != m_weapon;
	m_usesClips = !(weapon->GetWpnData().iFlags & ITEM_FLAG_EXHAUSTIBLE) && weapon->UsesClipsForAmmo1();
	m_exhaustible = (weapon->GetWpnData().iFlags & ITEM_FLAG_EXHAUSTIBLE) && !weapon->UsesClipsForAmmo1();
	SetPaintBackgroundEnabled(m_usesClips);
	if (changed)
	{
		// Do not carry the previous weapon's shell into another weapon or an observer target.
		m_animation->StopAnimationSequence(g_pClientMode->GetViewport(), "HUDAmmoShoot");
		m_shellAlpha = m_bulletOffset = 0;
		m_lastAlpha = 1;
	}
	else if (m_usesClips && weapon->Clip1() < m_clip)
		PlayAmmoAnimation(false);
	m_weapon = weapon;
	m_clip = MAX(0, weapon->Clip1());
	m_reserve = MAX(0, weapon->GetReserveAmmoCount(AMMO_POSITION_PRIMARY));
	// Optional compatibility with servers exposing the CSSO 2 magazine reload rule.
	ConVarRef magazineReload("sv_weapon_magazine_reload", true);
	if (magazineReload.IsValid() && magazineReload.GetBool() && m_usesClips && weapon->GetMaxClip1() > 0)
		m_reserve /= weapon->GetMaxClip1();
	m_exhaustibleCount = m_exhaustible ? MAX(0, player->GetAmmoCount(weapon->GetPrimaryAmmoType())) : 0;
	m_burst = weapon->WeaponHasBurst() ? (weapon->IsInBurstMode() ? 1 : 0) : -1;
	LoadImages();
	if (m_exhaustible)
	{
		const char *name = weapon->GetClassname() + 7;
		// Plus entity alias differs from the original HUD asset name.
		if (!Q_strcmp(name, "incgrenade"))
			name = "incendiarygrenade";
		m_exhaustibleImage.Load(UTIL_VarArgs("materials/vgui/hud/svg/%s.svg", name),
								V("exhaustible_icon_tall", 32));
	}
}
int CHudAmmo::Text(HFont font, const wchar_t *text, int x, int y, Color color, bool right)
{
	int wide, tall;
	surface()->GetTextSize(font, text, wide, tall);
	surface()->DrawSetTextFont(font);
	surface()->DrawSetTextColor(color);
	surface()->DrawSetTextPos(right ? x - wide : x, y);
	surface()->DrawPrintText(text, V_wcslen(text));
	return wide;
}
void CHudAmmo::KillIcon(CHudTexture *icon, int x, int y, bool animate)
{
	if (!icon)
		return;
	int wide = V("kill_icon_wide", 22), tall = V("kill_icon_tall", 22);
	Color color = C("kill_count_fgcolor", Color(255, 255, 255, 255));
	if (animate)
	{
		const int w = MAX(1, (int)(wide * m_killScale)), h = MAX(1, (int)(tall * m_killScale));
		x += (wide - w) / 2;
		y += (tall - h) / 2;
		wide = w;
		tall = h;
		color[3] = (int)(color.a() * clamp(m_killAlpha, 0.0f, 1.0f));
	}
	icon->DrawSelf(x, y, wide, tall, color);
}
void CHudAmmo::Paint()
{
	if (!m_hasWeapon || !m_weapon)
		return;
	// SVGs and the flying shell extend beyond the short HUD panel.
	g_pMatSystemSurface->DisableClipping(true);
	wchar_t text[64];
	Color hud = gHUD.GetHUDColor(cl_hud_color.GetInt());
	if (m_usesClips)
	{
		if (!cl_hud_healthammo_style.GetBool())
		{
			Color bullet = C("bullet_icon_fgcolor", Color(255, 255, 255, 230));
			if (cl_hud_color.GetInt())
			{
				bullet[0] = hud.r();
				bullet[1] = hud.g();
				bullet[2] = hud.b();
			}
			if (m_weapon->GetMaxClip1() > 0 && (float)m_clip / m_weapon->GetMaxClip1() <= 0.2f)
				bullet = C("bullet_icon_low_fgcolor", Color(255, 0, 0, 230));
			int limit = clamp(m_layout->GetInt("bullet_icon_count", 5), 0, 64);
			for (int i = 0; i < MIN(m_clip, limit); ++i)
			{
				Color color = bullet;
				if (i == limit - 1)
					color[3] = (int)(color.a() * clamp(m_lastAlpha, 0.0f, 1.0f));
				m_bullet.Draw(V("bullet_icon_xpos", 292) + i * V("bullet_icon_xrepeat", -10) +
								  (int)(m_bulletOffset * m_screenTall / 1080.0f),
							  V("bullet_icon_ypos", 24), V("bullet_icon_wide", 16), V("bullet_icon_tall", 16),
							  color);
			}
			bullet[3] = (int)(bullet.a() * clamp(m_shellAlpha, 0.0f, 1.0f));
			m_shell.Draw(V("bullet_icon_xpos", 292) + (int)(m_shellX * m_screenTall / 1080.0f),
						 V("bullet_icon_ypos", 24) + (int)(m_shellY * m_screenTall / 1080.0f),
						 V("bullet_icon_wide", 16), V("bullet_icon_tall", 16), bullet);
		}
		V_snwprintf(text, ARRAYSIZE(text), L"/ %d", m_reserve);
		Color reserve = C("reserve_fgcolor", Color(255, 255, 255, 230));
		Color clip = C("clip_fgcolor", Color(255, 255, 255, 255));
		if (cl_hud_color.GetInt())
		{
			reserve[0] = clip[0] = hud.r();
			reserve[1] = clip[1] = hud.g();
			reserve[2] = clip[2] = hud.b();
		}
		const int reserveLeft = V("reserve_xpos", 242) - Text(m_reserveFont, text, V("reserve_xpos", 242),
															  V("reserve_ypos", 20), reserve, true);
		V_snwprintf(text, ARRAYSIZE(text), L"%d", m_clip);
		Text(m_clipFont, text, reserveLeft + V("clip_xpos", -9), V("clip_ypos"), clip, true);
		if (m_burst >= 0)
		{
			CAmmoSVG &image = m_burstImage[m_burst];
			int wide = V("burst_icon_wide", 20);
			int tall = image.width ? wide * image.height / image.width : wide;
			for (int i = 0; i < 3; ++i)
				image.Draw(reserveLeft + i * wide, V("burst_icon_ypos", 7), wide, tall,
						   C("burst_icon_fgcolor", Color(255, 255, 255, 255)));
		}
	}
	if (m_exhaustible)
	{
		int tall = V("exhaustible_icon_tall", 32);
		int wide = m_exhaustibleImage.width;
		for (int i = 0;
			 i < MIN(m_exhaustibleCount, clamp(m_layout->GetInt("exhaustible_icon_count", 3), 0, 64)); ++i)
			m_exhaustibleImage.Draw(V("exhaustible_icon_xpos", 285) + i * V("exhaustible_icon_xrepeat", -32),
									V("exhaustible_icon_ypos", 9), wide, tall,
									C("exhaustible_icon_fgcolor", Color(255, 255, 255, 230)));
	}
	const int kills = m_normalKills + m_headshotKills;
	const int limit = clamp(m_layout->GetInt("kill_icon_count", 5), 1, 64);
	const int x = V("kill_count_xpos", 120), y = V("kill_count_ypos", 4);
	CHudTexture *normal = gHUD.GetIcon("HUDAmmoKillIcon"),
				*headshot = gHUD.GetIcon("HUDAmmoKillHeadshotIcon");
	if (kills > limit)
	{
		V_snwprintf(text, ARRAYSIZE(text), L"%d", kills);
		Text(m_killFont, text, x, y, C("kill_count_fgcolor", Color(255, 255, 255, 255)));
		KillIcon(normal, x + V("kill_icon_xpos", -23), y + V("kill_icon_ypos", 5), true);
	}
	else
		for (int i = 0; i < kills; ++i)
			KillIcon(i < m_headshotKills ? headshot : normal,
					 x + V("kill_icon_xpos", -23) + i * V("kill_icon_xrepeat", -22),
					 y + V("kill_icon_ypos", 5), i == (m_lastHeadshot ? 0 : kills - 1));
	// Preserve Plus StatTrak ownership and local statistics without affecting CSSO kill counters.
	C_CSPlayer *player = GetHudPlayer();
	if (player && m_weapon->HasStatTrak())
	{
		int owner = player->IsControllingBot() ? player->GetControlledBotIndex() : player->entindex();
		int count =
			m_weapon->GetOriginalOwnerIndex() == owner
				? g_CSClientGameStats
					  .GetStatById(GetWeaponTableEntryFromWeaponId(m_weapon->GetCSWeaponID()).killStatId)
					  .iStatValue
				: 0;
		if (count > 0)
		{
			V_snwprintf(text, ARRAYSIZE(text), L"%d", count);
			wchar_t localized[128];
			const wchar_t *format = g_pVGuiLocalize->Find("#Cstrike_WPNHUD_StatTrak");
			if (format)
			{
				g_pVGuiLocalize->ConstructString(localized, sizeof(localized), format, 1, text);
			}
			else
				V_wcsncpy(localized, text, sizeof(localized));
			Text(m_statFont, localized, V("stattrak_xpos", 120), V("stattrak_ypos", 37), hud, true);
		}
	}
	surface()->DrawFlushText();
	g_pMatSystemSurface->DisableClipping(false);
}
bool CHudAmmo::ShouldDraw()
{
	return !cl_draw_only_deathnotices.GetBool() && CHudElement::ShouldDraw();
}

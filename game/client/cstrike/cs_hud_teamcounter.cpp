//========= Copyright Valve Corporation, All rights reserved. ============//
// CSSO 2.0 team counter, adapted to the Android HUD scale and avatar paths.
#include "cbase.h"
#include "c_cs_player.h"
#include "c_cs_playerresource.h"
#include "c_cs_team.h"
#include "c_plantedc4.h"
#include "cs_gamerules.h"
#include "cs_hud_scaledpanel.h"
#include "filesystem.h"
#include "gametypes/igametypes.h"
#include "hudelement.h"
#include "iclientmode.h"
#include "vgui_avatarimage.h"
#include <vgui/ILocalize.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/VectorImagePanel.h>

using namespace vgui;
ConVar hud_playercount_pos("hud_playercount_pos", "0", FCVAR_ARCHIVE, "0 = top, 1 = bottom (legacy alias)");
ConVar cl_hud_playercount_pos("cl_hud_playercount_pos", "0", FCVAR_ARCHIVE, "0 = default (top), 1 = bottom");
ConVar cl_hud_playercount_showcount("cl_hud_playercount_showcount", "0", FCVAR_ARCHIVE,
									"0 = player avatars, 1 = alive count numbers");
extern ConVar cl_draw_only_deathnotices;

// 0x1802cf37e: progressive mode displays weapon progression, other modes score.
static int TeamCounterScore(int index)
{
	C_CS_PlayerResource *resource = GetCSResources();
	return CSGameRules() && CSGameRules()->IsPlayingGunGameProgressive() && resource
			   ? resource->GetPlayerGunGameWeaponIndex(index)
			   : g_PR->GetPlayerScore(index);
}

static bool TeamCounterBefore(int left, int right)
{
	const int primaryLeft = TeamCounterScore(left), primaryRight = TeamCounterScore(right);
	if (primaryLeft != primaryRight)
		return primaryLeft > primaryRight;
	if (g_PR->GetPlayerScore(left) != g_PR->GetPlayerScore(right))
		return g_PR->GetPlayerScore(left) > g_PR->GetPlayerScore(right);
	if (g_PR->GetFrags(left) != g_PR->GetFrags(right))
		return g_PR->GetFrags(left) > g_PR->GetFrags(right);
	// CSSO2 compares assists here; that field is not networked in this port.
	if (g_PR->GetDeaths(left) != g_PR->GetDeaths(right))
		return g_PR->GetDeaths(left) < g_PR->GetDeaths(right);
	return left < right;
}

class CTeamCounterAvatar;

// Panel::PaintBorder is gated by a non-null IBorder. Use a normal child Paint
// pass so the custom frame/health overlay does not depend on scheme borders.
class CTeamCounterAvatarOverlay : public Panel
{
	DECLARE_CLASS_SIMPLE(CTeamCounterAvatarOverlay, Panel);

  public:
	CTeamCounterAvatarOverlay(CTeamCounterAvatar *owner);
	void Paint() override;

  private:
	CTeamCounterAvatar *m_owner;
};

class CTeamCounterAvatar : public Panel
{
	DECLARE_CLASS_SIMPLE(CTeamCounterAvatar, Panel);

  public:
	CTeamCounterAvatar(Panel *parent, int index)
		: BaseClass(parent, "PlayerAvatar"), m_index(index), m_userID(-1), m_nextRefresh(0), m_avatarCRC(0),
		  m_team(-1), m_statusSize(0), m_statusTall(0)
	{
		SetMouseInputEnabled(false);
		SetKeyBoardInputEnabled(false);
		SetPaintBackgroundEnabled(false);
		m_avatar = new CAvatarImagePanel(this, "Avatar");
		m_avatar->SetShouldDrawFriendIcon(false);
		m_avatar->SetShouldScaleImage(true);
		m_avatar->SetClickable(false);
		m_avatar->SetMouseInputEnabled(false);
		m_avatar->SetZPos(0);
		m_overlay = new CTeamCounterAvatarOverlay(this);
		m_status = new VectorImagePanel(this, "Status");
		m_status->SetMouseInputEnabled(false);
		m_status->SetZPos(2);
		m_score = new Label(this, "Score", L"");
		m_score->SetMouseInputEnabled(false);
		m_statusKind = -1;
		m_health = 0;
		m_avatarSize = 1;
		m_border = m_healthTall = m_avatarY = 0;
		m_alive = m_showHealth = false;
	}
	Color StyleColor(KeyValues *style, const char *key, Color fallback)
	{
		const char *value = style->GetString(key, "");
		if (!value[0])
			return fallback;
		if ((value[0] >= '0' && value[0] <= '9') || value[0] == '-')
			return style->GetColor(key);
		return scheme()->GetIScheme(scheme()->GetScheme("ClientScheme"))->GetColor(value, fallback);
	}
	void Update(int x, int y, int wide, int tall, int border, int healthTall, int scoreTall, bool large,
				HFont font, Color teamColor, Color healthColor, Color lowColor, bool bottom, KeyValues *style,
				float scale)
	{
		const int scoreX = large ? (int)(style->GetInt("large_avatar_score_xpos", 0) * scale) : 0;
		const int scoreY = large ? (int)(style->GetInt("large_avatar_score_ypos", 0) * scale) : 0;
		const int scoreWide =
			large ? MAX(1, (int)(style->GetInt("large_avatar_score_wide", 62) * scale)) : wide;
		SetBounds(x, y, MAX(wide, scoreX + scoreWide), tall + scoreTall + MAX(0, scoreY));
		m_overlay->SetBounds(0, 0, wide, tall + scoreTall + MAX(0, scoreY));
		m_avatarSize = tall;
		m_avatarWide = wide;
		m_avatarY = bottom ? scoreTall + MAX(0, scoreY) : 0;
		SetVisible(true);
		m_border = border;
		m_healthTall = healthTall;
		m_teamColor = teamColor;
		m_healthColor = healthColor;
		m_lowColor = lowColor;
		const bool alive = g_PR->IsAlive(m_index);
		m_alive = alive;
		C_CSPlayer *local = C_CSPlayer::GetLocalCSPlayer();
		C_BasePlayer *player = UTIL_PlayerByIndex(m_index);
		const int maxHealth = player ? MAX(1, player->GetMaxHealth()) : 100;
		m_health = clamp((float)g_PR->GetHealth(m_index) / maxHealth, 0.0f, 1.0f);
		// Enemy health is not public information in team modes.
		m_showHealth = local && (local->GetTeamNumber() == g_PR->GetTeam(m_index) || local->IsObserver());
		m_avatar->SetBounds(border, m_avatarY + border, MAX(1, wide - 2 * border), MAX(1, tall - 2 * border));
		player_info_t info;
		if (engine->GetPlayerInfo(m_index, &info) &&
			(m_userID != info.userID || m_avatarCRC != info.customFiles[2] ||
			 m_team != g_PR->GetTeam(m_index) ||
			 (!m_avatar->IsValid() && gpGlobals->curtime >= m_nextRefresh)))
		{
			m_team = g_PR->GetTeam(m_index);
			m_userID = info.userID;
			m_avatarCRC = info.customFiles[2];
			m_nextRefresh = gpGlobals->curtime + 2.0f;
			m_avatar->SetDefaultAvatar(scheme()->GetImage(
				g_PR->GetTeam(m_index) == TEAM_CT ? CSTRIKE_DEFAULT_CT_AVATAR : CSTRIKE_DEFAULT_T_AVATAR,
				true));
			m_avatar->SetPlayer(m_index, large ? k_EAvatarSize64x64 : k_EAvatarSize32x32);
		}
		// The original dead-player branch draws only the elimination icon.
		m_avatar->SetVisible(alive);
		int kind = !alive ? 0
						  : (local && local->IsPlayerDominatingMe(m_index)
								 ? 2
								 : (local && local->IsPlayerDominated(m_index) ? 1 : -1));
		const char *status = kind == 0 ? "skull" : kind == 1 ? "dominated" : "nemesis";
		char key[96];
		Q_snprintf(key, sizeof(key), "%savatar_%s_wide", large ? "large_" : "", status);
		int iconWide = MAX(1, (int)(style->GetInt(key, large ? 42 : 20) * scale));
		Q_snprintf(key, sizeof(key), "%savatar_%s_tall", large ? "large_" : "", status);
		int iconTall = MAX(1, (int)(style->GetInt(key, large ? 42 : 20) * scale));
		if (kind != m_statusKind || iconWide != m_statusSize || iconTall != m_statusTall)
		{
			m_statusKind = kind;
			m_statusSize = iconWide;
			m_statusTall = iconTall;
			if (kind >= 0)
			{
				m_status->SetRenderSize(iconWide, iconTall);
				m_status->SetTexture(kind == 0	 ? "materials/vgui/hud/svg/elimination.svg"
									 : kind == 1 ? "materials/vgui/hud/svg/teamcounter_dominated.svg"
												 : "materials/vgui/hud/svg/teamcounter_nemesis.svg");
			}
		}
		m_status->SetBounds((wide - iconWide) / 2, m_avatarY + (tall - iconTall) / 2, iconWide, iconTall);
		m_status->SetVisible(kind >= 0);
		Q_snprintf(key, sizeof(key), "avatar_%s_fgcolor", status);
		m_status->SetFgColor(
			StyleColor(style, key, kind == 0 ? Color(71, 84, 95, 178) : Color(255, 255, 255, 255)));
		m_score->SetVisible(large && TeamCounterScore(m_index) > 0);
		if (large)
		{
			wchar_t text[32];
			V_snwprintf(text, ARRAYSIZE(text), L"%d", TeamCounterScore(m_index));
			m_score->SetText(text);
			m_score->SetFont(font);
			m_score->SetBounds(scoreX, bottom ? MAX(0, -scoreY) : tall + scoreY, scoreWide, scoreTall);
			m_score->SetContentAlignment(Label::a_center);
			m_score->SetPaintBackgroundEnabled(true);
			m_score->SetPaintBackgroundType(0);
			m_score->SetBorder(NULL);
			m_score->SetBgColor(StyleColor(style, "large_avatar_score_bgcolor", Color(0, 0, 0, 111)));
			m_score->SetTextInset(0, 0);
			m_score->SetFgColor(StyleColor(style,
										   local && local->entindex() == m_index
											   ? "large_avatar_score_self_fgcolor"
											   : "large_avatar_score_fgcolor",
										   Color(255, 255, 255, 51)));
		}
	}
	void PaintAvatarOverlay()
	{
		if (!m_alive)
			return;
		surface()->DrawSetColor(m_teamColor);
		for (int i = 0; i < m_border; ++i)
			surface()->DrawOutlinedRect(i, m_avatarY + i, m_avatarWide - i, m_avatarY + m_avatarSize - i);
		if (m_alive && m_showHealth)
		{
			// 0x1802cfec9..0x1802cfeed: inset by the border; low health <= 20%.
			const int base = m_avatarY + m_avatarSize - m_border;
			surface()->DrawSetColor(m_health <= 0.2f ? m_lowColor : m_healthColor);
			surface()->DrawFilledRect(m_border, base - m_healthTall,
									  m_border + (int)((m_avatarWide - 2 * m_border) * m_health), base);
		}
	}
	void InvalidateIdentity()
	{
		m_userID = -1;
		m_nextRefresh = 0;
	}

  private:
	int m_index, m_userID, m_border, m_healthTall, m_statusKind, m_avatarSize, m_avatarWide, m_avatarY;
	float m_nextRefresh, m_health;
	CRC32_t m_avatarCRC;
	int m_team, m_statusSize, m_statusTall;
	bool m_alive, m_showHealth;
	Color m_teamColor, m_healthColor, m_lowColor;
	CAvatarImagePanel *m_avatar;
	CTeamCounterAvatarOverlay *m_overlay;
	VectorImagePanel *m_status;
	Label *m_score;
};

CTeamCounterAvatarOverlay::CTeamCounterAvatarOverlay(CTeamCounterAvatar *owner)
	: BaseClass(owner, "AvatarFrameAndHealth"), m_owner(owner)
{
	SetMouseInputEnabled(false);
	SetKeyBoardInputEnabled(false);
	SetPaintBackgroundEnabled(false);
	SetPaintBorderEnabled(false);
	SetZPos(1);
}

void CTeamCounterAvatarOverlay::Paint()
{
	m_owner->PaintAvatarOverlay();
}

class CHudTeamCounter : public CHudElement, public CCSHudScaledPanel
{
	DECLARE_CLASS_SIMPLE(CHudTeamCounter, CCSHudScaledPanel);

  public:
	CHudTeamCounter(const char *name);
	~CHudTeamCounter()
	{
		m_style->deleteThis();
	}
	void Init();
	void Reset();
	void FireGameEvent(IGameEvent *event);
	bool ShouldDraw();
	void OnThink();
	void OnScreenSizeChanged(int, int)
	{
		LoadStyle();
	}
	void ApplySettings(KeyValues *data)
	{
		// Explicit pixel layout must survive EditablePanel's deferred size messages.
		KeyValues *settings = data->MakeCopy();
		settings->SetBool("skip_autoresize", true);
		BaseClass::ApplySettings(settings);
		settings->deleteThis();
		if (m_style)
			LoadStyle();
	}

  private:
	void LoadStyle();
	void UpdateMode();
	int S(int value) const
	{
		return (int)(value * (ScreenHeight() / 1080.0f));
	}
	int V(const char *key, int fallback = 0) const
	{
		return S(m_style->GetInt(key, fallback));
	}
	int Y(int y, int tall)
	{
		return m_bottom ? GetTall() - y - tall : y;
	}
	Color C(const char *key, Color fallback);
	HFont Font(const char *key, const char *fallback);
	void Text(Label *label, const char *prefix, const wchar_t *text);
	void Avatars(CUtlVector<int> &players, int team, bool large);
	KeyValues *m_style;
	Label *m_timer, *m_wins[2], *m_count[2], *m_aliveText[2];
	Panel *m_timerBackground;
	ImagePanel *m_background[2];
	VectorImagePanel *m_bomb, *m_skull[2];
	CTeamCounterAvatar *m_avatars[MAX_PLAYERS + 1];
	bool m_bottom, m_large, m_noTimer, m_lastLegacy, m_lastPos, m_defused;
	int m_modeType, m_modeIndex;
	bool m_bombPlanted, m_bombDefused, m_bombExploded;
	float m_bombBlowTime, m_nextBombPulse;
};
DECLARE_HUDELEMENT(CHudTeamCounter);

CHudTeamCounter::CHudTeamCounter(const char *name)
	: CHudElement(name), CCSHudScaledPanel(NULL, "HudTeamCounter")
{
	SetParent(g_pClientMode->GetViewport());
	SetProportional(true);
	SetScheme(scheme()->GetScheme("ClientScheme"));
	SetPaintBackgroundEnabled(false);
	SetMouseInputEnabled(false);
	SetKeyBoardInputEnabled(false);
	KeyValues *layout = new KeyValues("HudTeamCounter");
	layout->SetBool("skip_autoresize", true);
	BaseClass::ApplySettings(layout);
	layout->deleteThis();
	SetHiddenBits(HIDEHUD_PLAYERDEAD);
	m_style = new KeyValues("HudTeamCounter");
	m_timerBackground = new Panel(this, "RoundTimerBackground");
	m_timerBackground->SetMouseInputEnabled(false);
	m_timerBackground->SetKeyBoardInputEnabled(false);
	m_timerBackground->SetPaintBackgroundType(0);
	m_timerBackground->SetBorder(NULL);
	m_timerBackground->SetZPos(-1);
	m_timer = new Label(this, "RoundTimerLabel", L"");
	m_bomb = new VectorImagePanel(this, "BombIcon");
	m_bomb->SetPaintBackgroundEnabled(false);
	m_bomb->SetPaintBorderEnabled(false);
	m_bomb->SetMouseInputEnabled(false);
	m_bomb->SetKeyBoardInputEnabled(false);
	m_bomb->SetZPos(2);
	m_bombPlanted = m_bombDefused = m_bombExploded = false;
	m_bombBlowTime = m_nextBombPulse = 0;
	for (int i = 0; i < 2; ++i)
	{
		m_wins[i] = new Label(this, i ? "TWinCounterLabel" : "CTWinCounterLabel", L"");
		m_count[i] = new Label(this, i ? "TAliveCounterLabel" : "CTAliveCounterLabel", L"");
		m_aliveText[i] =
			new Label(this, i ? "TAliveTextLabel" : "CTAliveTextLabel", "#Cstrike_PlayerCount_Alive");
		m_background[i] = new ImagePanel(this, i ? "TBackground" : "CTBackground");
		m_background[i]->SetImage(i ? "hud/teamcounter_t_background" : "hud/teamcounter_ct_background");
		m_background[i]->SetShouldScaleImage(true);
		m_background[i]->SetMouseInputEnabled(false);
		m_background[i]->SetZPos(-1);
		m_skull[i] = new VectorImagePanel(this, i ? "TSkull" : "CTSkull");
	}
	for (int i = 0; i <= MAX_PLAYERS; ++i)
		m_avatars[i] = NULL;
	m_bottom = false;
	m_defused = false;
	m_large = false;
	m_noTimer = false;
	m_lastLegacy = hud_playercount_pos.GetBool();
	m_lastPos = cl_hud_playercount_pos.GetBool();
	m_modeType = m_modeIndex = -1;
	LoadStyle();
}
void CHudTeamCounter::Init()
{
	ListenForGameEvent("bomb_planted");
	ListenForGameEvent("bomb_defused");
	ListenForGameEvent("bomb_exploded");
	ListenForGameEvent("round_start");
	ListenForGameEvent("round_prestart");
	m_modeType = m_modeIndex = -1;
	Reset();
}
void CHudTeamCounter::Reset()
{
	m_bombPlanted = m_bombDefused = m_bombExploded = false;
	m_bombBlowTime = m_nextBombPulse = 0;
	m_bomb->SetVisible(false);
	for (int i = 1; i <= MAX_PLAYERS; ++i)
		if (m_avatars[i])
		{
			m_avatars[i]->SetVisible(false);
			m_avatars[i]->InvalidateIdentity();
		}
}
void CHudTeamCounter::FireGameEvent(IGameEvent *event)
{
	const char *name = event->GetName();
	if (!Q_stricmp(name, "round_start") || !Q_stricmp(name, "round_prestart"))
		Reset();
	else if (!Q_stricmp(name, "bomb_planted"))
	{
		m_bombPlanted = true;
		m_bombDefused = m_bombExploded = false;
		m_bombBlowTime = gpGlobals->curtime + mp_c4timer.GetFloat();
		m_nextBombPulse = gpGlobals->curtime;
	}
	else if (!Q_stricmp(name, "bomb_defused"))
	{
		m_bombPlanted = true;
		m_bombDefused = true;
	}
	else if (!Q_stricmp(name, "bomb_exploded"))
	{
		m_bombPlanted = false;
		m_bombExploded = true;
	}
}
void CHudTeamCounter::LoadStyle()
{
	m_style->Clear();
	// Retain usable central HUD geometry with older resource packs.
	m_style->LoadFromBuffer(
		"TeamCounterDefaults",
		"\"HudTeamCounter\" { "
		"\"round_timer_xpos\" \"c-42\" \"round_timer_wide\" \"84\" \"round_timer_tall\" \"27\" "
		"\"round_timer_font\" \"HUDTeamCounterRoundFont\" "
		"\"wins_ct_xpos\" \"c-42\" \"wins_ct_ypos\" \"28\" \"wins_ct_wide\" \"42\" \"wins_ct_tall\" \"27\" "
		"\"wins_ct_fgcolor\" \"TeamCT\" \"wins_ct_font\" \"HUDTeamCounterRoundFont\" "
		"\"wins_t_xpos\" \"c1\" \"wins_t_ypos\" \"28\" \"wins_t_wide\" \"41\" \"wins_t_tall\" \"27\" "
		"\"wins_t_fgcolor\" \"TeamT\" \"wins_t_font\" \"HUDTeamCounterRoundFont\" "
		"\"alive_count_number_font\" \"HUDTeamCounterAliveCountNumberFont\" "
		"\"alive_count_label_font\" \"HUDTeamCounterAliveCountLabelFont\" "
		"\"large_avatar_score_font\" \"HUDTeamCounterLargeAvatarScoreFont\" "
		"\"avatar_border_ct_fgcolor\" \"181 212 238 255\" "
		"\"avatar_border_t_fgcolor\" \"234 209 138 255\" "
		"\"avatar_healthbar_fgcolor\" \"202 202 202 255\" "
		"\"avatar_healthbar_low_fgcolor\" \"234 81 81 255\" }");
	KeyValues *resource = new KeyValues("HudTeamCounter");
	if (resource->LoadFromFile(g_pFullFileSystem, "resource/hud/teamcounter_csso2.res", "GAME"))
		for (KeyValues *key = resource->GetFirstSubKey(); key; key = key->GetNextKey())
			m_style->SetString(key->GetName(), key->GetString());
	resource->deleteThis();
	// Icons are rasterized at the current resolution, then shared with the HUD matrix scale.
	m_bomb->SetRenderSize(MAX(1, V("bomb_icon_wide", 42)), MAX(1, V("bomb_icon_wide", 42)));
	m_bomb->SetTexture(m_defused ? "materials/vgui/hud/svg/teamcounter_bomb_defused.svg"
								 : "materials/vgui/hud/svg/teamcounter_bomb_planted.svg");
	for (int i = 0; i < 2; ++i)
	{
		m_skull[i]->SetRenderSize(MAX(1, V("alive_count_skull_wide", 32)),
								  MAX(1, V("alive_count_skull_tall", 32)));
		m_skull[i]->SetTexture("materials/vgui/hud/svg/elimination.svg");
	}
}
Color CHudTeamCounter::C(const char *key, Color fallback)
{
	const char *value = m_style->GetString(key, "");
	if (!*value)
		return fallback;
	if ((*value < '0' || *value > '9') && *value != '-')
		return scheme()->GetIScheme(GetScheme())->GetColor(value, fallback);
	return m_style->GetColor(key);
}
HFont CHudTeamCounter::Font(const char *key, const char *fallback)
{
	IScheme *clientScheme = scheme()->GetIScheme(GetScheme());
	const char *name = m_style->GetString(key, fallback);
	// These four fonts are specified in 1080-height pixels in CSSO2. Creating
	// them at the final height avoids the lossy 28 -> 12 -> 27 conversion.
	static const char *names[] = {"HUDTeamCounterRoundFont", "HUDTeamCounterAliveCountNumberFont",
								  "HUDTeamCounterAliveCountLabelFont", "HUDTeamCounterLargeAvatarScoreFont"};
	static const int heights[] = {28, 46, 14, 24};
	static HFont fonts[4] = {INVALID_FONT, INVALID_FONT, INVALID_FONT, INVALID_FONT};
	static int pixels[4] = {0, 0, 0, 0};
	for (int i = 0; i < 4; ++i)
	{
		if (Q_stricmp(name, names[i]))
			continue;
		const int tall = MAX(1, S(heights[i]));
		if (fonts[i] == INVALID_FONT)
			fonts[i] = surface()->CreateFont();
		if (pixels[i] != tall)
		{
			int flags = ISurface::FONTFLAG_ANTIALIAS | ISurface::FONTFLAG_CUSTOM;
			if (i == 1 || i == 2)
				flags |= ISurface::FONTFLAG_DROPSHADOW;
			if (!surface()->SetFontGlyphSet(fonts[i], "Stratum2 Bold", tall, 0, 0, 0, flags))
				break;
			pixels[i] = tall;
		}
		return fonts[i];
	}
	HFont font = clientScheme->GetFont(name, true);
	return font != INVALID_FONT ? font : clientScheme->GetFont(fallback, true);
}
void CHudTeamCounter::Text(Label *label, const char *prefix, const wchar_t *text)
{
	char key[96];
	Q_snprintf(key, sizeof(key), "%s_xpos", prefix);
	const char *x = m_style->GetString(key, "0");
	int xpos = (*x == 'c') ? GetWide() / 2 + S(atoi(x + 1)) : S(atoi(x));
	Q_snprintf(key, sizeof(key), "%s_ypos", prefix);
	int y = V(key);
	Q_snprintf(key, sizeof(key), "%s_wide", prefix);
	int w = V(key);
	Q_snprintf(key, sizeof(key), "%s_tall", prefix);
	int h = V(key);
	label->SetBounds(xpos, Y(y, h), w, h);
	label->SetText(text);
	label->SetContentAlignment(Label::a_center);
	label->SetTextInset(0, 0);
	Q_snprintf(key, sizeof(key), "%s_font", prefix);
	label->SetFont(Font(key, "RoundTimerFont"));
	Q_snprintf(key, sizeof(key), "%s_fgcolor", prefix);
	label->SetFgColor(C(key, Color(255, 255, 255, 255)));
	Q_snprintf(key, sizeof(key), "%s_bgcolor", prefix);
	label->SetPaintBackgroundEnabled(true);
	label->SetPaintBackgroundType(0);
	label->SetBorder(NULL);
	label->SetBgColor(C(key, Color(0, 0, 0, 153)));
	label->SetMouseInputEnabled(false);
}
void CHudTeamCounter::UpdateMode()
{
	if (!g_pGameTypes)
		return;
	int type = g_pGameTypes->GetCurrentGameType(), mode = g_pGameTypes->GetCurrentGameMode();
	if (type == m_modeType && mode == m_modeIndex)
		return;
	m_modeType = type;
	m_modeIndex = mode;
	m_large = CSGameRules() &&
			  (CSGameRules()->IsPlayingGunGameProgressive() || CSGameRules()->IsPlayingGunGameDeathmatch());
	m_noTimer = CSGameRules() && CSGameRules()->IsPlayingGunGameProgressive();
	KeyValues *root = new KeyValues("GameModes");
	if (root->LoadFromFile(g_pFullFileSystem, "gamemodes.txt", "GAME"))
	{
		KeyValues *types = root->FindKey("gametypes");
		const char *tn = g_pGameTypes->GetGameTypeFromInt(type),
				   *mn = g_pGameTypes->GetGameModeFromInt(type, mode);
		KeyValues *t = types && tn ? types->FindKey(tn) : NULL;
		KeyValues *modes = t ? t->FindKey("gamemodes") : NULL;
		KeyValues *m = mn ? (modes ? modes->FindKey(mn) : t ? t->FindKey(mn) : NULL) : NULL;
		// Older game packages do not expose UI attributes; use the stock CSSO2 mode defaults.
		m_large = CSGameRules() && (CSGameRules()->IsPlayingGunGameProgressive() ||
									CSGameRules()->IsPlayingGunGameDeathmatch());
		m_noTimer = CSGameRules() && CSGameRules()->IsPlayingGunGameProgressive();
		KeyValues *attr = m ? m->FindKey("uiattributes") : NULL;
		if (attr)
		{
			m_large = attr->GetBool("teamcounter_largeavatars", m_large);
			m_noTimer = attr->GetBool("teamcounter_noroundtimer", m_noTimer);
		}
	}
	root->deleteThis();
}
bool CHudTeamCounter::ShouldDraw()
{
	C_CSPlayer *p = C_CSPlayer::GetLocalCSPlayer();
	return !cl_draw_only_deathnotices.GetBool() && p && !p->IsObserver() && CHudElement::ShouldDraw();
}
void CHudTeamCounter::Avatars(CUtlVector<int> &players, int team, bool large)
{
	const int size = MAX(1, V(large ? "large_avatar_wide" : "avatar_wide", large ? 62 : 27));
	const int tall = MAX(1, V(large ? "large_avatar_tall" : "avatar_tall", large ? 62 : 27));
	const int scoreTall = large ? MAX(1, V("large_avatar_score_tall", 24)) : 0;
	const int scoreOffset = large ? V("large_avatar_score_ypos") : 0;
	const int minimumGap = MAX(1, m_style->GetInt("min_gap_between_elements", 1));
	const int gap = MAX(minimumGap, V(large ? "large_avatar_margin" : "avatar_xmargin", 2));
	const int cols = MAX(1, m_style->GetInt("avatar_xmax", 6));
	const int rows = MAX(1, m_style->GetInt("avatar_ymax", 2));
	const int limit = large ? MAX(1, m_style->GetInt("large_avatar_max", 10))
							: cols * MAX(1, m_style->GetInt("avatar_ymax", 2));
	const int count = MIN(players.Count(), limit);
	const int total = count * size + MAX(0, count - 1) * gap;
	for (int n = 0; n < count; ++n)
	{
		int index = players[n];
		if (!m_avatars[index])
			m_avatars[index] = new CTeamCounterAvatar(this, index);
		int x = large ? (GetWide() - total) / 2 + n * (size + gap)
					  : GetWide() / 2 +
							(team == TEAM_CT
								 ? -V("round_timer_wide", 84) / 2 - gap - size - (n / rows) * (size + gap)
								 : V("round_timer_wide", 84) / 2 + gap + (n / rows) * (size + gap));
		int y = large ? (m_noTimer ? 0 : V("round_timer_ypos") + V("round_timer_tall", 27))
					  : (n % rows) * (tall + MAX(minimumGap, V("avatar_ymargin", 1)));
		Color color =
			C(g_PR->GetTeam(index) == TEAM_CT ? "avatar_border_ct_fgcolor" : "avatar_border_t_fgcolor",
			  g_PR->GetTeam(index) == TEAM_CT ? Color(181, 212, 238, 255) : Color(234, 209, 138, 255));
		m_avatars[index]->Update(
			x, Y(y, tall + scoreTall + MAX(0, scoreOffset)), size, tall,
			MAX(1, V(large ? "large_avatar_border_size" : "avatar_border_size", large ? 3 : 2)),
			MAX(1, V(large ? "large_avatar_healthbar_tall" : "avatar_healthbar_tall", large ? 4 : 3)),
			scoreTall, large, Font("large_avatar_score_font", "RoundTimerFont"), color,
			C("avatar_healthbar_fgcolor", Color(202, 202, 202, 255)),
			C("avatar_healthbar_low_fgcolor", Color(234, 81, 81, 255)), m_bottom, m_style,
			ScreenHeight() / 1080.0f);
	}
}
void CHudTeamCounter::OnThink()
{
	bool legacy = hud_playercount_pos.GetBool(), pos = cl_hud_playercount_pos.GetBool();
	if (legacy != m_lastLegacy && pos == m_lastPos)
	{
		cl_hud_playercount_pos.SetValue(legacy);
		pos = legacy;
	}
	else if (pos != m_lastPos)
	{
		hud_playercount_pos.SetValue(pos);
		legacy = pos;
	}
	else if (legacy && !pos)
	{
		cl_hud_playercount_pos.SetValue(legacy);
		pos = legacy;
	}
	m_lastLegacy = legacy;
	m_lastPos = pos;
	m_bottom = pos;
	SetHudScaleAnchor(0.5f, m_bottom ? 1 : 0);
	const int height = MAX(1, S(128)), edge = S(2);
	SetBounds(0, m_bottom ? ScreenHeight() - edge - height : edge, ScreenWidth(), height);
	for (int i = 1; i <= MAX_PLAYERS; ++i)
		if (m_avatars[i])
			m_avatars[i]->SetVisible(false);
	C_CSGameRules *rules = CSGameRules();
	if (!g_PR || !rules)
	{
		for (int i = 0; i < 2; ++i)
		{
			m_wins[i]->SetVisible(false);
			m_count[i]->SetVisible(false);
			m_aliveText[i]->SetVisible(false);
			m_background[i]->SetVisible(false);
			m_skull[i]->SetVisible(false);
		}
		m_timer->SetVisible(false);
		m_timerBackground->SetVisible(false);
		m_bomb->SetVisible(false);
		return;
	}
	UpdateMode();
	CUtlVector<int> ct, t, all;
	int alive[2] = {0, 0};
	for (int i = 1; i <= MIN(MAX_PLAYERS, gpGlobals->maxClients); ++i)
		if (g_PR->IsConnected(i))
		{
			int team = g_PR->GetTeam(i);
			if (team != TEAM_CT && team != TEAM_TERRORIST)
				continue;
			(team == TEAM_CT ? ct : t).AddToTail(i);
			all.AddToTail(i);
			if (g_PR->IsAlive(i))
				++alive[team == TEAM_CT ? 0 : 1];
		}
	// Restore the primary mode-specific ordering. CSSO2 additionally compares
	// match/team leader flags and assists, which this player resource does not network.
	for (int i = 1; i < all.Count(); ++i)
	{
		int j = i;
		while (j > 0 && TeamCounterBefore(all[j], all[j - 1]))
		{
			int tmp = all[j];
			all[j] = all[j - 1];
			all[j - 1] = tmp;
			--j;
		}
	}
	const bool digits = cl_hud_playercount_showcount.GetBool() && !m_large;
	for (int i = 0; i < 2; ++i)
	{
		int team = i ? TEAM_TERRORIST : TEAM_CT;
		wchar_t text[32];
		C_CSTeam *tm = GetGlobalCSTeam(team);
		V_snwprintf(text, ARRAYSIZE(text), L"%d", tm ? tm->Get_Score() : 0);
		Text(m_wins[i], i ? "wins_t" : "wins_ct", text);
		m_wins[i]->SetVisible(!m_large);
		const char *xp =
			m_style->GetString(i ? "alive_count_t_xpos" : "alive_count_ct_xpos", i ? "c45" : "c-100");
		int x = (*xp == 'c' ? GetWide() / 2 + S(atoi(xp + 1)) : S(atoi(xp)));
		int y = V(i ? "alive_count_t_ypos" : "alive_count_ct_ypos"),
			w = V(i ? "alive_count_t_wide" : "alive_count_ct_wide", 55),
			h = V(i ? "alive_count_t_tall" : "alive_count_ct_tall", 55);
		m_background[i]->SetBounds(x, Y(y, h), w, h);
		m_background[i]->SetVisible(digits);
		V_snwprintf(text, ARRAYSIZE(text), L"%d", alive[i]);
		m_count[i]->SetText(text);
		m_count[i]->SetFont(Font("alive_count_number_font", "AliveCountFont"));
		m_count[i]->SetFgColor(C("alive_count_number_fgcolor", Color(255, 255, 255, 255)));
		int fh = surface()->GetFontTall(m_count[i]->GetFont());
		m_count[i]->SetBounds(x, Y(y + V("alive_count_number_ypos", 24) - fh / 2, fh), w, fh);
		m_count[i]->SetContentAlignment(Label::a_center);
		m_count[i]->SetTextInset(0, 0);
		m_count[i]->SetVisible(digits && alive[i] > 0);
		m_aliveText[i]->SetFont(Font("alive_count_label_font", "AliveTextFont"));
		m_aliveText[i]->SetFgColor(C("alive_count_label_fgcolor", Color(255, 255, 255, 255)));
		fh = surface()->GetFontTall(m_aliveText[i]->GetFont());
		m_aliveText[i]->SetBounds(x, Y(y + V("alive_count_label_ypos", 46) - fh / 2, fh), w, fh);
		m_aliveText[i]->SetContentAlignment(Label::a_center);
		m_aliveText[i]->SetTextInset(0, 0);
		m_aliveText[i]->SetVisible(digits && alive[i] > 0);
		int skull = V("alive_count_skull_wide", 32);
		m_skull[i]->SetBounds(x + (w - skull) / 2, Y(y + (h - skull) / 2, skull), skull, skull);
		m_skull[i]->SetFgColor(C("alive_count_skull_fgcolor", Color(204, 0, 51, 178)));
		m_skull[i]->SetVisible(digits && alive[i] == 0);
	}
	if (m_large)
		Avatars(all, TEAM_CT, true);
	else if (!digits)
	{
		Avatars(ct, TEAM_CT, false);
		Avatars(t, TEAM_TERRORIST, false);
	}
	// SetDormant removes the entity from g_PlantedC4s. HUD state must survive
	// that transition, and also work when joining after the planted event.
	C_PlantedC4 *c4 = NULL;
	for (int i = 0; i < g_PlantedC4s.Count(); ++i)
		if (g_PlantedC4s[i] && (g_PlantedC4s[i]->IsBombActive() || g_PlantedC4s[i]->m_bBombDefused))
		{
			c4 = g_PlantedC4s[i];
			break;
		}
	const bool planted = !m_bombExploded && (m_bombPlanted || rules->m_bBombPlanted || c4);
	bool defused = m_bombDefused || (c4 && c4->m_bBombDefused);
	if (c4)
		m_bombBlowTime = c4->m_flC4Blow;
	else if (planted && m_bombBlowTime <= 0)
		m_bombBlowTime = gpGlobals->curtime + mp_c4timer.GetFloat();
	if (planted && !defused && gpGlobals->curtime >= m_nextBombPulse)
	{
		const float remaining = MAX(0.0f, m_bombBlowTime - gpGlobals->curtime);
		const float duration = c4 ? MAX(1.0f, c4->m_flTimerLength) : MAX(1.0f, mp_c4timer.GetFloat());
		const float interval = MAX(0.15f, 0.1f + 0.9f * clamp(remaining / duration, 0.0f, 1.0f));
		m_nextBombPulse = gpGlobals->curtime + interval;
	}
	if (defused != m_defused)
	{
		m_defused = defused;
		m_bomb->SetTexture(defused ? "materials/vgui/hud/svg/teamcounter_bomb_defused.svg"
								   : "materials/vgui/hud/svg/teamcounter_bomb_planted.svg");
	}
	int bw = V("bomb_icon_wide", 42), bh = bw;
	const char *bx = m_style->GetString("bomb_icon_xpos", "c5");
	m_bomb->SetBounds((*bx == 'c' ? GetWide() / 2 + S(atoi(bx + 1)) : S(atoi(bx))) - bw / 2,
					  Y(V("bomb_icon_ypos", 13) - bh / 2, bh), bw, bh);
	// CSSO 2.0 0x1802d0389: highlight during the final 50ms before the next glow.
	const bool beep = (c4 ? c4->m_flNextGlow : m_nextBombPulse) - gpGlobals->curtime < 0.05f;
	m_bomb->SetFgColor(C(defused ? "bomb_icon_defused_fgcolor"
						 : beep	 ? "bomb_icon_beep_fgcolor"
								 : "bomb_icon_fgcolor",
						 Color(184, 0, 0, 255)));
	m_bomb->SetVisible(!m_noTimer && planted && (defused || !c4 || !c4->m_bExplodeWarning));
	float remaining = rules->IsFreezePeriod() ? rules->GetRoundStartTime() - gpGlobals->curtime
											  : rules->GetRoundRemainingTime();
	int seconds = MAX(0, (int)remaining);
	wchar_t timer[32];
	V_snwprintf(timer, ARRAYSIZE(timer), L"%d : %02d", seconds / 60, seconds % 60);
	Text(m_timer, "round_timer", timer);
	int timerX, timerY, timerWide, timerTall;
	m_timer->GetBounds(timerX, timerY, timerWide, timerTall);
	m_timerBackground->SetBounds(timerX, timerY, timerWide, timerTall);
	m_timerBackground->SetBgColor(C("round_timer_bgcolor", Color(0, 0, 0, 153)));
	m_timerBackground->SetVisible(!m_noTimer);
	m_timer->SetPaintBackgroundEnabled(false);
	m_timer->SetVisible(!m_noTimer && !planted && !rules->IsTimeOutActive() && !rules->IsWarmupPeriod());
	if (seconds <= 10)
		m_timer->SetFgColor(C("round_timer_low_fgcolor", Color(255, 0, 0, 255)));
}

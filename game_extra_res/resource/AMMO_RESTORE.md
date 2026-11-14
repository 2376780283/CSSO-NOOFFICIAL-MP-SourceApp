# CSSO 2.0 Ammo 移植说明

本次将 `game/client/cstrike/cs_hud_ammo.cpp` 改为直接绘制数字和图标，继续继承 CCSHudScaledPanel，右下锚点及 cl_hud_scale 保留。旧 resource/hud/ammo.res 不再加载。

## 已接入

- HudAmmo 使用 1920×1080 参考坐标；默认 342×54、简洁 282×54。样式按零/非零处理。
- 普通枪械绘制弹匣、备弹；根据实际文字宽度向左定位。弹匣 y 独立于备弹 y。备弹通过武器 GetReserveAmmoCount(AMMO_POSITION_PRIMARY) 读取，兼容 Plus 的武器实体备弹及旧玩家弹药数组，不能只读 player->GetAmmoCount（普通枪械通常返回 0）。
- 默认样式最多五枚子弹；弹匣比例 <=20% 时子弹及弹壳变红，数字不随低弹药变红。
- 同武器弹匣数量下降启动 HUDAmmoShoot。切枪清除旧弹壳，避免不同武器之间残留。此清理是 Plus 兼容处理。
- 子弹偏移 -10→0、末端 alpha 0→1，均为 0.13 秒；末端指配置上限的末位。
- 弹壳 x/y 在 0.5 秒内到 120，x Linear/y Accel；alpha 延迟 0.125 秒后用 0.375 秒淡出。
- 消耗型武器使用 HUD SVG，最多三个；简洁样式仍支持此独立分支。Plus 的 weapon_incgrenade 映射原资源 incendiarygrenade.svg。
- 连发支持实心/轮廓三枚图标；不支持连发的武器不显示。
- player_death/player_spawn 维护本地玩家本次生命普通/爆头击杀；死亡及出生重置，队杀排除。
- <=5 击杀按类型分组排列，爆头在前；最近爆头动画作用首枚，最近普通击杀作用末枚。>5 时显示数字及单枚普通图标。
- HUDAmmoKill：scale .4→1.25（.13 秒）→1（再 .2 秒），alpha 0→1（.13 秒），围绕图标中心缩放。
- 背景使用原版 ammo_background；cl_hud_background_alpha 仅控制背景。
- 保留 Plus StatTrak 的原拥有者检查、本地统计及本地化，计数与击杀动画独立。
- 内置 Stratum2 Bold 字号为 52/25/28（1080 参考），字体缺失回退 Tahoma；布局可指定 clip_font/reserve_font/kill_count_font/stattrak_font Scheme 字体。

## 资源加载

Ammo 使用独立 AnimationController，在 OnThink 用 gpGlobals->curtime 推进，Init 加载 `scripts/hudanimations_ammo_csso2.txt`，查找目标显式使用 viewport。viewport ReloadScheme 清空全局动画时不再丢失 Ammo 事件。脚本缺失时以 RunAnimationCommand 执行相同时间线，支持旧 Android 资源部署。脚本使用原版事件名及属性，代码实际消费全部属性。

普通/爆头击杀图标若未由 mod_textures 注册，则在 Ammo Init 注册，避免覆盖整个 mod_textures.txt。SVG 按屏幕高度缓存和重栅格化，绘制支持动画边界；弹壳允许绘制到短面板之外。

部署需要客户端 so、scripts/hudlayout.res、scripts/hudanimations_ammo_csso2.txt、materials/vgui/hud 下本次新增材质及 SVG，随后完整重启。

## 原版证据及兼容边界

原 DLL：构造 0x1802b61b0；死亡/出生 0x1802b7a90；更新 0x1802b8480；绘制 0x1802b8930。资源对照 scripts/hudlayout.res HudAmmo、scripts/hudanimations.txt HUDAmmoShoot/HUDAmmoKill、resource/clientscheme.res HUDAmmo*Font。

Plus 当前服务端死亡事件不发送 teamkill，缺字段时用 IsOtherEnemy 推导；已有字段时服从事件。观察模式弹药继续使用 GetHudPlayer，击杀事件按原版使用本地玩家，未擅自改成观察目标击杀。

Plus 未实现 sv_weapon_magazine_reload。客户端仅对存在该 ConVar 的兼容环境提供备用弹匣数量换算，不新增服务器变量或改变装填机制。原版额外武器配置门控没有对应字段，尚不能宣称该可选模式完全一致。

本次没有改服务器、全局 VGUI 或 HUD 缩放基类。编译通过不代表视觉已与原版一致。待实机验证：连续射击及低弹药、装填/快速换枪、默认/简洁、普通/爆头混合与第六杀、死亡重生、观察目标切换、三连发及投掷物、不同分辨率和 cl_hud_scale。

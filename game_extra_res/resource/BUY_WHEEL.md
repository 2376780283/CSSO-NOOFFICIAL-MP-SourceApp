# CSSO 2.0 风格购买菜单

当前入口为 resource/ui/buymenu_csso2.res。动态圆盘的按钮样式来自 buymenu_csso2_buttons.res，预览灯光来自 buymenu_csso2_scenes.res。旧多页面布局、classes 属性页、独立购买 scheme、专用字体与材质已从资源包移除。

数据来自 scripts/playerloadout.txt 和 cfg/loadout.txt；后者不存在时读取 cfg/loadout_default.txt。先按阵营过滤，再按分类配装生成按钮。当前仍沿用 CSSO 原二选一槽位规则：配装中 m4a4、hkp2000 等可以解析为玩家槽位的另一把武器。尚未移植 CSSO2 分类配装编辑器或服务器自定义配装同步，不能通过修改客户端配置绕过服务器购买规则。

圆盘采用完整面板大小的动态扇区按钮，角度和圆形边界共同决定命中。奇数项补空扇区。纹理按 CSSO2 triangle_uv 模式逐扇区映射：外弧 U 从0走到1、V为0，圆心UV为(0.5,1)，约每2度生成一个独立三角形。原材质正常显示时不垫实色背景，以保留透明边缘；纯色只用于材质不可用的回退。依据 client.dll 0x18054cb30（读取triangle_uv）、0x18054cbf0（生成三角形/UV）、0x18054d570（逐三角形绘制），不能使用整圆XY到UV的投影。材质来自 CSSO2 的 vgui/ui/buymenu_button 和 buymenu_button_hover；武器图标沿用当前游戏 SVG。数字 1–9 选择；Esc 或底栏返回先回主分类，再关闭；Ctrl 购买并丢出；支持原 closeonbuy、autobuy、rebuy。旧收藏命令仍可用，但未放入新界面。

cs_buymenu_item_render_mode：0 人物持武器，1 世界武器模型，2 第一人称模型。装备无第一人称模型时回退世界模型。复用现有 Android/ToGLES 模型渲染与皮肤链，世界模型按包围盒中心调整相机，第一人称关闭旋转。需要实机核验不同武器的姿态、取景及手套。

部署：更新 build/game/client/libcsso_client.so，并合并 game_extra_res 到游戏根目录，完整重启。至少包含三份 buymenu_csso2*.res、scripts/playerloadout.txt、cfg/loadout_default.txt、resource/buymenu2_*.txt、现有字体/scheme、materials/vgui/ui/buymenu* 与 sound/ui/buymenu*.wav。不要覆盖玩家已有 cfg/loadout.txt。

若游戏目录曾部署旧菜单，请同步删除旧 classes/、materials/vgui/buymenu/、resource/clientscheme_buymenu.res、resource/buywheel_*.txt，以及 resource/ui/ 下旧 base_*.res、base_x.res、mainbuymenu.res 和各阵营旧购买页面；合并复制不会自动删除这些旧文件。保留 freezepanel_basic.res 和 buymenu_csso2*.res。旧专用字体 csd_icons.ttf、buymenu_equipment.ttf、stratumno2black.ttf 也已移除；其余 HUD 共用字体保留。

本版只有客户端圆盘与预览重建，不能声称完整移植 CSSO2：退购服务端记录及资格判断尚未实现，因此界面没有退购按钮。原二进制的逐物品退购、全部退购、分类配装编辑、稀有度名称背景仍待后续接入。

已完成 Android client 编译链接；未运行游戏。需验证 CT/T 六分类、触摸与鼠标扇区边界、布局/分辨率重载、价格/购买反馈、Ctrl 丢出、人物/世界/第一人称预览、关闭触摸恢复。

# CSSO 2.0 聊天逐行背景

客户端的 `CHudChatHistory::PaintBackground()` 绘制背景，使用 RichText 的可见行布局和文字淡出记录。自动换行、滚动、历史裁剪和清空无需维护第二份消息索引。打开聊天框后背景跟随文字保持显示，整体历史背景淡入时逐行背景退出。

## 资源

以下资源取自本地 CSSO 2.0 安装：

- `resource/ui/basechat.res`：原版聊天布局，包括 `line_background` 和 `line_outer_background`；各控件显式指定 1920×1080 缩放基准，以适配本项目。
- `scripts/hudlayout.res`：`HudChat` 同步使用 1920×1080 基准，位置 `(10, r453)`、尺寸 `569×399`。在 1920×1080 下外框左上角为 `(10, 627)`，底边距屏幕底部 54 像素。
- `materials/vgui/hud/chatline_background.vmt`
- `materials/vgui/hud/chatline_outer_background.vmt`
- `materials/vgui/hud/chatline_background.vtf`：两个材质共用的纹理。

将 `game_extra_res/` 的内容合并到游戏内容目录，并重新编译客户端及 `vgui_controls`。字体继续使用项目现有 ChatScheme。

## 自定义

在 `HudChatHistory` 的资源块中可指定背景材质：

```
"line_background"       "vgui/hud/chatline_background"
"line_outer_background" "vgui/hud/chatline_outer_background"
```

代码默认使用这些材质，因此自定义聊天布局也可使用。将某项显式设为 `""` 可禁用相应背景；禁用主背景也会禁用外侧背景。只删除配置项会回退到代码默认值。

主背景在历史控件内绘制，外侧背景延伸到最近的屏幕边缘。透明度峰值为 192，与当前项目 `CHAT_HISTORY_ALPHA` 的整体背景过渡同步。

## 验证

已使用项目 Android 编译数据库检查修改的两个 C++ 文件。游戏内还需检查：短消息、长消息自动换行、连续消息独立淡出、打开/关闭聊天框、滚动历史、不同分辨率及 `-nocustomchat` 布局。

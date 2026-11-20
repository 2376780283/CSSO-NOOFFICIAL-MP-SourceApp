# Android 玩家附近方形透明区域排查

## 现象与结论边界

其他玩家或 bot 在掩体后或部分遮挡时，其附近出现约 1:1 的方形区域；区域能看见后方环境，却看不见隐藏玩家。低性能设备更明显。以上来自问题描述，尚无帧捕获或实机开关对照。

当前未确认根因，未修改渲染代码。方形提示存在矩形代理、精灵、阴影贴图或局部渲染目标操作，但形状本身不能证明其中某条路径有错。低性能设备差异也不能单独证明线程竞争或 GPU 驱动故障。

## 已核对的源码

- `game/client/cstrike/c_cs_player.cpp`：`C_CSPlayer::DrawModel()`（约 1787 行）检查 dormant/visible 后交给基类；此处没有额外方框、局部清屏或深度覆盖逻辑。不能据此排除基类、模型材质和后端问题。
- 同文件 `ShadowCastType()`（约 3336 行）：其他可见玩家请求动态 render-to-texture 阴影；第一人称不投影。实际类型受阴影系统配置影响。
- `game/client/clientshadowmgr.cpp`：`r_shadows` 默认 1，`r_shadowrendertotexture` 默认 0。不能把请求动态阴影视为实机一定启用了阴影图集。简单阴影材质为 `decals/simpleshadow`，贴图阴影为 `decals/rendershadow` 与 `decals/rendermodelshadow`。
- 同文件 `DrawRenderToTextureShadow()`（约 3726 行）：设置图集 tile 的矩形 viewport 后清颜色；`ComputeShadowTextures()`（约 3994 行）负责 render target 入栈/出栈及矩阵恢复。若只有贴图阴影触发，需检查此链路绑定的 FBO、viewport/scissor、清屏目标及恢复状态。
- `materialsystem/stdshaders/shadow.cpp`：阴影使用 ZERO/SRC_COLOR 乘法混合，正常只改变表面颜色，不应显示墙后环境。仅凭阴影矩形边界不能解释穿透。
- `game/client/c_pixel_visibility.cpp`：`PixelVisibility_DrawProxy()`（52 行）构造面向视角的代理，默认等宽高时投影近似方形；用于硬件可见性查询。
- `materialsystem/stdshaders/occlusion_dx9.cpp`：代理禁止颜色、alpha 和深度写入。正常代理不应出现在画面中。是否有代理位于出问题玩家处仍需实测。
- `game/client/cstrike/c_cs_player.cpp` 的 `Simulate()`：其他玩家手电光束及 halo 使用 `sprites/glow01.vmt`；`game/client/view_beams.cpp` 的 halo 路径调用像素可见性查询。不是所有玩家都无条件创建此效果。
- `game/client/cstrike/radio_status.cpp`：无线电头顶图标绘制 3D 四边形。语音/无线电活动可作为观察条件。
- `game/client/cstrike/cs_hud_headname.cpp`：`cl_headname` 控制队友姓名、生命值及图标 HUD。它属于屏幕空间绘制，应与 3D 阴影/深度路径分别隔离。
- `togles/linuxwin/glmgr.cpp`：`GLMContext::Clear()`（约 1924 行）保存并恢复写掩码和局部清屏 scissor；目前没有发现仅由该函数文本就能确认的状态泄漏。
- `build/compile_commands.json` 包含 `togles/linuxwin` 编译项，支持从 GLES 后端继续排查；这不证明用户运行的 APK 正是该构建。

## 独立发现：查询结果语义不一致

`togles/linuxwin/cglmquery.cpp` 的 `CGLMQuery::Start()/Stop()` 使用 `GL_ANY_SAMPLES_PASSED`，结果是是否存在通过测试的样本。`CPixelVisibilityQuery::GetFractionVisible()` 在 `r_pixelvisibility_partial=1` 时把可见查询结果除以总覆盖查询结果，当作像素面积比例。布尔结果只能提供粗略可见性，不能恢复部分遮挡面积。

这可能解释 halo 的部分遮挡过渡异常，但查询本身不清除墙体，不能宣称它就是透明洞根因。不要将 `gl_nullqueries` 当作关闭代理绘制：它只使底层查询为空操作，并不跳过代理几何。

## 实机隔离顺序

在离线本地地图固定视点、bot 位置和画质。先在控制台输入下面变量名，记录原值；恢复使用记录值，不要假设都是默认值。需要 cheat 的命令仅在本地 `sv_cheats 1` 下测试。每项独立测试后恢复原值，避免叠加关闭导致误判。

1. `r_shadows 0`：观察方框是否消失。若消失，优先追踪简单阴影/贴图阴影及接收表面状态。
2. 恢复阴影后，若原本 `r_shadowrendertotexture` 为 1，测试 `r_shadowrendertotexture 0`。若仅此项消除，范围收窄到贴图阴影及其切换路径；若原本为 0，这项对照没有信息增益。
3. `r_dopixelvisibility 0`：测试代理查询链路。该变量参与关卡初始化，建议改值后重新进入地图再比较，恢复后也重新进入地图。关闭查询可能改变光晕的可见性回退，测试时只判断洞是否消失。
4. `cl_headname 0`：隔离队友头顶 2D HUD；结合敌我双方是否都触发区分线索。
5. 本地 `sv_cheats 1` 下，分别测试 `r_drawtranslucentrenderables 0` 和 `r_drawentities 0`，各自恢复后再做下一项。这些开关会移除大量内容，仅用于定位绘制类别，不能直接作为修复。`r_drawentities` 主要影响 studio 模型，不等于停掉所有玩家附属效果。

额外记录：方框中心在脚下、躯干还是头顶；是否随距离变大/变小；是否仅无线电、语音、手电或开火期间出现；方框中是实时同视点的后方环境，还是旧帧/天空/另一角度画面；是否仅某些掩体材质受影响。以上差异能区分贴图覆盖、深度异常和实际几何漏绘。

## 下一阶段验证

取得能消除问题的单项对照后，再对对应 pass 增加低频状态日志或 GPU 帧捕获。记录 draw 前后 framebuffer、viewport/scissor、depth test/function/write mask、RGBA write mask、blend、绑定材质及纹理。定位第一笔使错误像素出现的 draw/clear，再决定修改材质、状态恢复、渲染目标绑定或 GLES 能力适配。

若所有开关均无法隔离，需扩展至世界几何的深度预通道、透明材质排序、shader 转译及实际运行 APK 的后端。当前没有足够证据为某一候选路径打补丁，也没有进行游戏内验证。

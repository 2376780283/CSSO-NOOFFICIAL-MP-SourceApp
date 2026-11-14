# Android 启动与地图加载分析

## 范围与结论

这是对 csso-plus 当前源码的静态分析，未采集实机时间。以下区分已经确认的代码行为与待测瓶颈，不能据此声称某项占用了具体秒数或承诺加速比例。本轮不修改加载行为。

现代手机的多核 CPU、快速 UFS 和大内存无法自动加速串行预缓存、驱动编译或大量文件元数据访问。应先测量，再减少实际工作量和同步等待。

## 加载链路

启动：Android 原生入口 → 引擎及模块初始化 → 文件系统挂载 → Host/材质/声音/客户端初始化 → 主菜单。启动器 Java 层在本次未完整追踪，因此 APK 资源复制、校验等耗时仍待单独排查。

本地开图：Host_NewGame → CGameServer::SpawnServer → BSP/碰撞/世界模型 → 服务端 LevelInit 和实体预缓存 → connect localhost → 客户端 signon → 模型、材质、声音和实体初始化 → Map Load Complete。

远程联机不包含本机服务端 SpawnServer；还需要单独区分资源下载、网络等待和本地资源加载。

入口证据：engine/host.cpp 的 Host_NewGame；engine/sv_main.cpp 的 SpawnServer；engine/client.cpp 的 SetSignonState；engine/modelloader.cpp 的 Map_LoadModel；engine/cl_main.cpp 的 Map Load Complete。

## 已确认的重点路径

### 1. GLES 纹理可能在 CPU 解压后上传

位置：togles/linuxwin/cglmtex.cpp，WriteTexels、CompressedTexImage2D、uncompressDXTc。

WriteTexels 依据 m_bHave_GL_EXT_texture_compression_dxt1 选择直接 glCompressedTexImage2D 或软件解压。回退路径按 4×4 块解压 DXT1/3/5，分配临时像素缓冲，再 glTexImage2D 上传 RGB/RGBA。GPU 能力标志是否准确、具体格式是否支持，需要在设备上记录；不能只凭 GPU 型号判断一定走哪条路径。

例如 2048×2048 DXT5 单级约 4 MiB，RGBA8 为 16 MiB，尚未计入完整 mip 链、临时缓冲和驱动内部副本。大量模型与贴图会同时放大解码时间、上传流量和内存压力。

优化方向：按具体 DXT 格式及 sRGB 能力判断直传；为不支持设备离线准备 ETC2/ASTC 资源或可靠的版本化转码缓存。转换要保留 alpha、法线、sRGB 和 mip 语义；ASTC 不能假定所有设备可用。离线压缩优先于每次启动现场压缩。

### 2. 模型加载默认同步，并主动触碰全部数据

位置：datacache/mdlcache.cpp；engine/modelloader.cpp。

AsyncMdlCache() 固定为 0，mod_load_mesh_async、mod_load_anims_async、mod_load_vcollide_async 默认关闭，mod_forcedata 默认开启。mod_forcetouchdata 默认 1，模型加载路径会调用 TouchAllData。

这些设置说明默认倾向于提前做完加载工作。仅切换宏或一次开启所有异步开关，不能证明加载流程就具备线程安全或真正并行。

优化方向：先给模型读取、解析、碰撞、网格、动画和材质依赖分别计时；随后拆成后台读取/解码与主线程提交，设置并发数和在途字节数上限。异步试验需检测首次开火、切枪、玩家出现和换图的卡顿及资源正确性。

### 3. 玩家预缓存范围很大

位置：game/server/cstrike/cs_player.cpp，CCSPlayer::Precache；game/shared/cstrike/cs_shareddefs.h。

代码遍历 7 组 CT、7 组 T 角色模型、全部探员以及手套和袖子配置。MAX_AGENTS_CT=39、MAX_AGENTS_T=42，两个循环均包含上界，合计进行 83 次探员模型候选检查；这不是 83 个独立文件的实测计数，可能存在重复。

IsModelPrecached 可避免已经登记模型的重复注册，但首轮涉及的模型、动画与材质依赖仍然很多。

优化方向：记录唯一模型及依赖数量，区分必要角色、当前装备和可选外观。网络预缓存表与实体生成规则约束必须保留，不能直接删除候选模型；优先将渲染数据的提前加载与网络注册解耦。此项主要解释本地建服路径，远程客户端需结合服务端下发资源分析。

### 4. 散文件及失败路径查询可能放大 Android I/O

位置：filesystem/filesystem_stdio.cpp，FS_fopen、FS_stat；filesystem/linux_support.cpp，findFileInDirCaseInsensitive。

普通打开失败后会尝试大小写不敏感查找，后者通过目录遍历匹配名称；成功打开且请求大小时还会按路径 stat。多个搜索目录、可选缺失资源和大小写不一致会增加元数据操作。

资源若放在 /storage/emulated/0，实际还受 Android 版本和存储实现影响。不能将其等同于直接读取 UFS 裸设备。

优化方向：同一资源集比较散文件/VPK、共享存储/应用内部目录；规范资源大小写；统计失败打开次数与目录扫描耗时；增加有失效策略的路径解析及负查询缓存；对已打开的文件考虑使用 fstat 获取长度。缓存必须在挂载路径变化、资源安装和开发热更新时正确失效。

### 5. GLES 着色器运行时编译与链接

位置：togles/linuxwin/cglmprogram.cpp。

当前路径使用 glCompileShader 和 glLinkProgram。现有 pair cache 和 program_cache.cfg 是程序组合/提示机制；本次检索未发现该后端使用 glGetProgramBinary/glProgramBinary 的持久缓存实现。驱动自身可能缓存，不能因此认定每次都会完全重新编译。

优化方向：先测 compile/link 次数与耗时，再考虑 GLES program binary 缓存。键应包含 shader 内容、版本、编译选项、设备/驱动标识和 binary format；加载失败时回退编译，采用原子写入及容量限制。缓存应放在应用可写缓存目录。

### 6. 加载进度条可能触发同步重绘

位置：engine/vgui_baseui_interface.cpp，CEngineVGui::UpdateProgressBar。

GameUI 要求刷新时会调用 V_RenderVGuiOnly。应记录重绘次数、总耗时及 present 等待，确认是否受垂直同步影响。可将加载界面重绘限制到合理频率，同时保留事件处理、取消与最后一帧刷新。当前尚无证据证明这是主要瓶颈。

## 第一轮测量

现有 COM_TimestampedLog 通过启动参数 -profile 开启，写入当前工作目录的 timestamped.log。实现位于 tier0/dbg.cpp，首次写入会清除旧日志，每条记录都会打开/追加/关闭文件，因此详细采样本身有成本，日志不可写时现有实现也缺少 fopen 失败检查。使用前确认工作目录可写，不要把 -profile 当作长期加速参数。

已有阶段标记包括启动初始化、Host_NewGame、SpawnServer、BSP 各 lump、客户端 signon 及 Map Load Complete。地图加载结束还输出模型与声音的累计时间；它们可能嵌套或重叠，不能直接全部相加当总时间。

建议用单调时钟聚合，阶段结束再写日志，增加以下统计：

- 文件打开/失败/大小写扫描次数，读取字节数和等待时间，按 VPK/散文件分类。
- 模型读取、解析、TouchAllData、碰撞和材质依赖耗时。
- 纹理直接上传/软件解压数量、解压字节数、解压时间与 GL 上传时间。
- shader compile/link 次数及总耗时。
- 服务端玩家/武器预缓存、客户端实体初始化、声音缓存与解码时间。
- 加载界面重绘/present 耗时、内存峰值、CPU 频率和温度。

测试矩阵：

1. 全新进程第一次启动、退出后重启、同进程第二次加载同地图、换另一地图。
2. 本地无机器人、本地有机器人、连接已有远程服务器。
3. 相同资源的 VPK 与散文件；有条件时比较应用内部目录与共享存储。
4. 固定画质、分辨率和电源状态，每组至少三次，记录中位数和范围。

重新启动进程不等于清除 OS 页缓存；报告应分别记录进程、文件缓存和 shader 缓存状态。不要为了测试随意清理用户数据。

## 现代设备优化优先级

第一优先：补阶段统计，查明 CPU 解码、I/O 等待和 GPU/驱动等待各占多少；修复资源大小写和反复失败查找。

第二优先：根据测量选择纹理直传/离线移动格式、资源打包、程序二进制缓存；这些通常比盲目增加线程更有针对性。

第三优先：减少不必要的完整预加载；分离网络注册与重渲染数据加载，建立有内存预算的后台流水线。

第四优先：按 big.LITTLE 实际表现调度少量工作线程，避免线程数等于逻辑核数造成争抢；GL 提交仍遵循上下文约束。大内存设备可提高经过测量的缓存预算，但应依据可用内存和 Android 内存压力事件控制，不能只看标称 RAM。

不要直接把 -preload、更多线程、mat_queue_mode 或同时打开全部异步 cvar 当作加载加速方案。-preload 在 engine/sv_precache.cpp 中会触发强制服务器预加载，可能增加进入地图前的工作量；运行期渲染并行也不等于地图加载并行。

# MVP 实现与清理记录

本轮基线：`dd2ba6ef`。目标是现有 5 种模式、44 项购买目录和 18 个人物在 Apple Silicon Mac 上稳定完成本地游戏 + BOT。

唯一验收组合：SDL + 桌面 OpenGL + 静态 client/server + VGUI2 + 原 HUD 风格 2。

## 已删除

- 两套备用菜单及子模块：`mainui_cpp`、`hymenu`，以及仅供旧菜单使用的二维码依赖。
- 旧 ImGui 核心、扩展、测试购买页、引擎渲染/输入接入、调试窗口和构建开关。
- Lua 接入、客户端脚本事件、LuaJIT、nameof、脚本资源及 CI 的下载步骤。
- 未使用的反射导出和在线公告实现。
- macOS Touch Bar 实现及 SDL 调用点。
- HUD 风格 0/1 的专属绘制、字段、资源加载、切换控件和本地资源。风格 2 的计分板仍在 `hud/legacy/hud_scoreboard_legacy.cpp` 中。

客户端 GUI 和预缓存回调表保留空槽以保持接口布局，删除其原声明、导出查找和调用。空槽不引用已删除框架。

本地额外清掉无使用方的 Boost、Bullet、libexpat、OpenSSL、ASTC、cpuinfo、oneTBB 等遗留目录。连同脚本依赖约移除 1088.9 MiB 文件内容；这些主要是被忽略的本地依赖，不等同于应用体积下降。旧 `build`、`build-rebuild`、`build-citrus`、`build-filament` 的生成产物也已清理。

## 保留与尚未确定的范围

- 非 Mac 平台源码、工程、移动触控、手柄和平台渲染转译层暂时保留。旧的 Mac-only 删除计划与后续四端目标有冲突，需确定平台边界后再处理。本轮没有进行 iOS、Android 或 Windows 构建验收。
- 实时语音、远程连接/发现和录像回放没有删除，功能范围尚未确定。
- 本地服务器、loopback、网络消息、移动预测、武器 C++ 事件、BOT、人质、地图实体和资源加载保留。
- VGUI2 当前需要的 SourceSDK、FreeType、共享字体/贴图、KeyValues、JSON 与桥接代码保留。
- 更深的训练/战役/教学与 HL 实体裁切后置，不能只凭地图未直接引用就删注册或运行时生成路径。

## 构建与资源

```sh
./tools/build-cso-ui.sh
python3 tools/prune-mvp-resources.py --game-root dist/csmoe
# 确认输出的本地资源清单后应用：
python3 tools/prune-mvp-resources.py --game-root dist/csmoe --apply
# 已有可写覆盖目录也应同步清理：
python3 tools/prune-mvp-resources.py --game-root build-cso-ui/run/csmoe --apply
./tools/run-cso-ui.sh --check
./tools/run-cso-ui.sh --smoke-quit
```

资源工具默认只预览；`--apply` 删除明确退役的脚本与旧 HUD 资源、更新精灵表计数、删除旧设置控件和配置项。它不遍历 Steam 的 `cstrike`/`valve` 链接，也不删除共享武器、人物和移动触控素材。对已清理包再次执行应报告零改动。

素材和本地依赖不进入 Git；源码提交携带清理工具，不携带资源包。构建脚本产物是 `build-cso-ui/game_launch/CSMoE.app`，日常应用位于 `dist/CSMoE.app`。

## 验证记录

- 从空生成目录完成 Mac arm64 构建；编译动作由原审计的 851 条降到 759 条。已删除模块没有进入构建图，最终二进制未发现 ImGui/Lua 运行时符号。
- 44 项购买目录的分类、价格/命令路由和五组配装测试通过；44 张彩色购买图齐全。
- HUD 的 11 个 TextMsg、4 个 TeamScore、24 个 Brass 生产/消费用例与异常包诊断通过。
- 竞技本地回合和购买、DM/TDM 死亡重生与购买、生化Ⅰ/Ⅱ感染与僵尸武器均完成回归，每种模式确认 1 名本地玩家和 3 名 BOT。
- 对局冒烟覆盖队伍/人物菜单、M4A1 和 M134 购买与开火、计分板、购买菜单及正常退出，并检查实际截图。
- 真实 VGUI 控件的五模式下拉菜单、选项点击、取消和设置页打开通过；中文名字输入并应用成功，旧 HUD 风格控件不再出现。此项使用 VGUI 测试输入，不等于系统输入法候选窗口验收。

这些记录不等于所有武器动作、所有地图或外网联机完整验收。武器资源检查仍有原有的 `k1a_boltpull.wav`、`mg3-2.wav` 缺失；与本轮清理前报告一致。原生事件注册的两个 `.sc` 名称无需因此补造同名文件。

应用仍使用本机 Homebrew SDL2/FreeType 和 Steam 基础素材路径；复制到另一台 Mac 即可运行的独立分发包不在本轮验收范围。

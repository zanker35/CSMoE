# Mac MVP 完整裁切记录

当前目标：Apple Silicon macOS 上的本地游戏 + BOT，保留 5 种模式、44 项购买目录和 18 个人物。唯一构建路径是 **SDL2 + 桌面 OpenGL + 静态 client/server + VGUI2 + 当前 HUD**。

原计划中保留其他平台、触屏、手柄、语音、发现和录像的待定范围，已按后续“全部清理”要求执行。本分支不再提供原上游多平台工程。

## 已完成的删除

| 主题 | 删除范围 |
|---|---|
| 平台和构建 | Android、Xcode-iOS、Win32/WinRT/Emscripten 后端、旧 Android.mk、iOS/MinGW 工具链、多平台 CI、vcpkg 和非 Mac 打包入口 |
| 渲染与启动变体 | NanoGL、WES、QindieGL、gl4es；独立服务器启动、动态游戏库加载和无窗口回退路径 |
| 重复 UI 和脚本 | mainui_cpp、hymenu、旧 ImGui、Lua/LuaJIT、nameof、专属接入和资源 |
| 输入 | 触控、触屏预设/编辑器、移动 API、震动、手柄/摇杆、evdev、Touch Bar、触屏旁观者/生化技能面板 |
| 外围功能 | 玩家实时语音及设置、服务器发现/心跳、远程连接菜单和命令、录像录制与回放 |
| 训练和战役 | career_tasks、tutor、training_gamerules、singleplay_gamerules，以及任务购买、教学消息和专属实体注册 |
| HL 实体 | 气瓶、医疗包/医疗站、护甲充电站、迫击炮/炮塔及控制器；玩家炮塔控制、对应技能参数和注册表；未使用的 plane 工具 |
| 其他遗留 | 旧 HUD 风格 0/1、未使用的反射导出/公告实现、60 个未参与构建的 SDK/UI 源文件及旧图标、无使用方的本地依赖和旧生成产物 |

源代码之外还清理了本地触屏图片/配置、语音设置、旧菜单条目、旧 HUD 精灵和贴图。删除对象的配置、菜单、注册和调用链一起处理，不以关闭一个构建开关代替删除。

## 保留的共享主干

- 本地服务器与 loopback、网络消息、移动/碰撞预测、武器规则及客户端开火事件。
- 现有五种规则、BOT、人质、门、梯子、可破坏物、列车、水和相机等地图实体。
- VGUI2、SourceSDK 实际参与构建的依赖、FreeType、字符处理、KeyValues 和资源加载。
- BOT 无线电音效与提示；它们不依赖已删除的玩家实时语音。
- 引擎标准函数表中的必要空槽，防止其他字段错位；这些槽没有已删除功能的实现或调用。

两个共享实现经过单独处理：

1. 桌面鼠标视角之前通过旧移动回调传递，现在使用独立 `IN_MouseLook`。HUD 的缩放文字绘制移出移动 API，继续供桌面 HUD 使用。
2. 普通 `sv_restart` 也调用原 `CareerRestart`。保留重开对局和刷新玩家状态的部分，重命名为 `PrepareMatchRestart`，删除其中的战役任务逻辑。

`src/ui/vgui2/.../linuxfont.cpp` 是当前 Mac 使用的 FreeType 字体实现；`hud/legacy/hud_scoreboard_legacy.cpp` 仍承载当前计分显示。这些文件按实际使用关系保留。

## 地图与资源边界

重新扫描当前 25 张 CS 地图，没有直接使用已删除的医疗站、炮塔、迫击炮及训练实体。可破坏物的 `spawnobject` 没有非零值。医疗包掉落表位置留空并增加空项检查，保留其后项目的地图编号。

资源清理只操作项目自己的 `csmoe` 包和可写覆盖目录，不删除外部 Steam 的 `cstrike`/`valve` 安装内容。共享模型、声音、贴图和外部 `.seq` 动作包继续保留。此结论针对当前地图集合，不保证任意 HL/自定义地图可用。

```sh
python3 tools/prune-mvp-resources.py --game-root dist/csmoe
python3 tools/prune-mvp-resources.py --game-root dist/csmoe --apply
python3 tools/prune-mvp-resources.py --game-root build-cso-ui/run/csmoe --apply
```

默认只预览；`--apply` 应用清单。重复执行已清理包应得到零待处理项。素材与本地依赖不进入 Git，源码提交携带可重复执行的清理工具。

## 构建与检查

```sh
./tools/build-cso-ui.sh
node tools/check-cso-hud-messages.js
python3 tools/check-cso-weapon-icons.py
python3 tools/check-cso-weapon-assets.py
./tools/run-cso-ui.sh --check
./tools/run-cso-ui.sh --smoke-quit
```

- 本轮从空目录完成 Mac arm64 构建；编译动作由上轮 759 条降至 732 条，已删除模块没有进入构建图。最初裁切前的记录为 851 条。
- 44 项购买目录/命令路由和五组配装测试通过，44 张购买图齐全。
- HUD 的 11 个 TextMsg、4 个 TeamScore、24 个 Brass 用例与异常包诊断通过。
- 原有两条声音缺失 `k1a_boltpull.wav`、`mg3-2.wav` 与裁切前一致；两个 `.sc` 名称由原生事件注册，不因此补造文件。

五种模式均完成本地玩家 + 3 个 BOT 回归：竞技购买、DM/TDM 死亡重生与购买、生化Ⅰ/Ⅱ感染均通过。VGUI 控件测试覆盖五模式下拉、菜单选择/取消、中文名字输入与应用，并确认触摸/语音/旧 HUD 选项不再出现。游戏内移动命令已验证能改变角色位置；系统级原生键鼠自动化暂受窗口识别限制，不记为已通过。本地应用仍依赖本机 SDL2/FreeType 和 Steam 基础素材；独立分发包属于交付打包工作，不等于源码裁切。

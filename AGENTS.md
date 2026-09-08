# CSMoE 开发约定

## 当前产品边界

- Apple Silicon macOS；SDL2 + 桌面 OpenGL；静态 client/server；VGUI2。
- 本地游戏 + BOT，保留 `none`、`dm`、`tdm`、`zb1`、`zb2` 五种模式。
- 当前购买目录 44 项；人类人物 18 个（截至 2009 年）；武器截至 2010-06-09，排除圣诞版。内容细则见 `docs/content-scope.md`。
- 从完整分支迁入功能时，只恢复明确需要的功能及其依赖。不要顺带恢复已移除的平台、Lua/ImGui、备用 UI、触屏、语音聊天、服务器发现、录像或训练系统。

## 源码职责与落位

- `src/app/`：macOS 应用入口与打包。
- `src/engine_api/`：引擎和游戏之间的协议、函数表、ABI 类型。不能放玩法实现。
- `src/engine/`：运行时、网络、客户端/服务端基础设施、渲染、音频、资源和平台适配。
- `src/game/server/`：模式和回合、玩家状态、战斗结算、地图实体、BOT/人质 AI。
- `src/game/client/`：输入、预测、视角、效果、HUD、局内菜单。
- `src/game/shared/`：实际被游戏两端消费的数据、契约以及分别编译到两端的移动和武器算法。不是杂物目录。
- `src/ui/gameui/`：主菜单、建房和设置页面。
- `src/ui/vgui2/`：VGUI2 控件、字体、面板和引擎桥接。
- `src/base/`：不依赖玩法、UI、游戏两端的通用工具。
- `vendor/`：第三方源码，保留原许可证与来源。

详细子目录、历史耦合例外和旧路径映射见 `docs/source-layout.md`、`docs/source-path-map.json`。不得为兼容旧分支建立旧目录软链接或转发头文件。

## 依赖与接口规则

1. 跨模块 include 写清职责路径（例如 `game/shared/movement/pm_shared.h`）；不要通过添加整个服务端目录到客户端 include 搜索路径来解决编译错误。
2. CMake 新增 include、宏、源码使用 target 级作用域；不把实现源码通过 PUBLIC/INTERFACE 意外传播给消费者。
3. 移动和武器共享实现仍分别在 `CLIENT_DLL` / `SERVER_DLL` 下编译，保留 `cl` / `sv` 命名空间。不能合成一份运行时对象替代两端实现。
   双端 CPP 只在 `src/game/shared/sources.cmake` 维护一份清单；客户端/服务端使用各自编译宏消费它，目录守卫验证两侧均参与编译。
4. 现存共享实体声明带有历史服务端依赖；目录迁移不等于已解耦。新增共享逻辑不得扩大这一例外；具体例外必须在架构文档记录。
5. 引擎函数表、消息和结构体的保留槽位不得随意删除、重排；本地服务器仍依赖 loopback、实体同步和预测。
6. 游戏资源路径与 C++ 源码路径不是一回事。不要机械改写 `models/`、`sound/`、`events/` 等运行时标识。

## 从完整分支迁入功能

每个功能独立提交，并在 `docs/feature-imports.md` 增加记录：来源 remote/branch、完整 commit SHA、功能范围、旧路径到新路径、依赖及资源、主动排除的部分、验证结果和未验证项。

先用 `docs/source-path-map.json` 查旧路径，再按职责判断新文件落位；映射描述本次重构基线，不是以后无条件覆盖文件的脚本。完整分支没有映射的新文件也必须明确归属。不要整目录覆盖，也不要通过恢复旧 CMake 文件引入被裁掉的功能。

武器迁入要同时检查服务端、客户端预测、客户端事件、购买目录及模型/声音/精灵；人物迁入要检查名单、菜单、模型和外置动作；模式迁入要检查规则注册、回合、HUD、菜单和 BOT 行为。

## 验证与交付

- 先检查分支和工作区，保留用户未提交改动。
- 构建：`./tools/build-cso-ui.sh`。
- 路径/分层：`python3 tools/check-source-layout.py`。
- HUD 消息：`node tools/check-cso-hud-messages.js`。
- 购买图标/资源：`python3 tools/check-cso-weapon-icons.py`、`python3 tools/check-cso-weapon-assets.py`。
- 启动检查与冒烟：`./tools/run-cso-ui.sh --check`、`./tools/run-cso-ui.sh --smoke-quit`。模式或输入/UI 改动还要做对应实际运行验证。
- 区分源码检查、编译通过、游戏进程运行和实际交互通过；保留基线资源告警说明。
- 构建目录、`dist/`、`.mydocs/` 是本地文件，不强制加入 Git；长期规则和迁移记录必须放在受版本控制的 AGENTS/docs 中。
- 不修改 `dist/cstrike`、`dist/valve` 指向的 Steam 共用资源。不在用户未要求时改系统设置。

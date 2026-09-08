# CSMoE — Mac MVP

基于 Xash3D 和 CS 1.6 的早期 CSOL 内容实现。本分支只维护 **Apple Silicon macOS、本地游戏 + BOT**。

保留 5 种模式、44 项购买目录和 18 个人物。使用 SDL2、桌面 OpenGL、静态客户端/服务端、VGUI2 和统一 HUD。

| 模式配置 | 玩法 |
|---|---|
| `none` | 竞技 |
| `dm` | 个人竞技 |
| `tdm` | 团队竞技 |
| `zb1` | 生化Ⅰ |
| `zb2` | 生化Ⅱ |

具体人物和武器见 [内容范围](docs/content-scope.md)。裁切范围、共享接口和验收记录见 [MVP 清理记录](docs/mvp-cleanup.md)。

## 构建

需要 Apple Silicon Mac、Xcode Command Line Tools、CMake、Ninja，以及可被 CMake 找到的 SDL2 和 FreeType。

```sh
./tools/build-cso-ui.sh
```

构建产物：`build-cso-ui/game_launch/CSMoE.app`。

本分支已经删除 Android/iOS 工程、Windows/其他平台实现、替代渲染后端、独立服务器启动方式和动态游戏库变体；原上游的多平台构建说明不再适用于此分支。

## 本地资源与运行

运行入口使用现有 `dist/csmoe` 资源包，以及本机的 `dist/cstrike` 和 `dist/valve` 基础素材路径。游戏素材不包含在 Git 仓库中。

```sh
./tools/run-cso-ui.sh --check
./tools/run-cso-ui.sh
```

配置、截图、日志写入 `build-cso-ui/run`，资源通过只读搜索路径读取。也可以双击 `tools/CSMoE-CSO-UI.command`。

导入旧资源包后，先查看并应用退役资源清单：

```sh
python3 tools/prune-mvp-resources.py --game-root dist/csmoe
python3 tools/prune-mvp-resources.py --game-root dist/csmoe --apply
```

工具清理旧脚本、触屏配置、语音设置和旧 HUD 素材，不修改 Steam 的基础素材目录。

## 验证

```sh
node tools/check-cso-hud-messages.js
python3 tools/check-cso-weapon-icons.py
python3 tools/check-cso-weapon-assets.py
./tools/run-cso-ui.sh --smoke-quit
```

冒烟测试等待真实客户端连接，并检查 BOT、选人、购买和开火；截图和真实键鼠操作另外验收。详细运行与界面说明见 [UI 运行说明](docs/cso-ui-port.md)。

本地服务器、loopback、网络消息、玩家预测、BOT、人质及保留地图所需实体仍是运行主干。服务器发现、远程连接菜单、玩家实时语音、录像回放、触控、手柄、训练、战役和教学系统已删除。

## 来源与许可

项目源自 [MoeMod/CSMoE](https://github.com/MoeMod/CSMoE)。保留各源文件中的原作者声明；许可信息见 [LICENSE.md](LICENSE.md)。本轮裁切不改变代码和外部素材各自的许可条件。

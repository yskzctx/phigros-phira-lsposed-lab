# Phigros / Phira LSPosed 本地实验模块

两个寄生原版应用的 LSPosed 开发模块，共用此公开仓库。与 [Phigros 修改版项目](https://github.com/yskzctx/phigros-offline-lab) 分开维护。

**0.2 测试预发行已公开，供手动验收；尚无已真机验证的正式模块。** 旧0.1因启用失败和功能不完整继续撤回。0.2经过代码修订与主机检查，实际接入、游玩和恢复仍待真机确认。

2026-10-25：两个入口已改为浅色卡片与分阶段连接报告；修正 Phira 原生回调注册顺序、引擎 data/ 层级、历史与会话恢复。Phira 目标分数、准确率、规则列表及原版触摸输入路径已连接生产代码，已做组件和部分原版 ARM64 函数验证；完整原版 Judge 与手机游玩尚未验收。用户于2026-10-25要求在外下载测试，现以明确标注未验收的预发行提供这次候选。

## 下载

[0.2 测试预发行下载页](https://github.com/yskzctx/phigros-phira-lsposed-lab/releases/tag/v0.2.0-pre.1)。直接安装对应 APK，无需解压或下载源码包。

- [Phigros 模块 APK（约47 MB）](https://github.com/yskzctx/phigros-phira-lsposed-lab/releases/download/v0.2.0-pre.1/Phigros-Lab-LSPosed-157-arm64-v0.2-DIAGNOSTICS-DEV.apk)：官方 Phigros 4.0.1（157），com.PigeonGames.Phigros，arm64。
- [Phira 模块 APK（约145 KB）](https://github.com/yskzctx/phigros-phira-lsposed-lab/releases/download/v0.2.0-pre.1/Phira-Lab-LSPosed-0.8.2-arm64-v0.2-DIAGNOSTICS-DEV.apk)：官方 Phira 0.8.2（39），org.flos.phira，arm64。
- [安装与启用步骤](docs/INSTALL-v0.2.md)。需要 root / LSPosed，勾选对应原版宿主并冷启动；音量上键打开面板，下键收起，自动游玩默认关闭。

旧 `v0.1.0-pre.1` 仍为撤回草稿。Phira 原有自制谱、资源包导入不作为本项目新增功能。两个新 APK 及所有附带文件已经匿名完整下载并核对 SHA-256；这只证明附件传输完整，不是手机功能验收。

| 模块 | 目标原版 | 0.2接入的代码（未完成实机验证） | 已知问题 |
| --- | --- | --- | --- |
| Phigros 本地实验模块 | com.PigeonGames.Phigros，4.0.1（157），arm64 | 控制面板、自动游玩、分数/准确率规划、判定规则、RKS/历史记录编辑、独立实验存档、自制谱播放器、音符皮肤 | 原生章节扩展特效/音效/判定线颜色未完成；两处皮肤存储独立；全部设备行为待验证 |
| Phira 本地实验模块 | org.flos.phira，0.8.2（39），arm64 | 原版触摸输入自动游玩、目标分数/准确率、规则列表与按键筛选、Good 早晚/随机、音量键面板、独立实验存档、历史全 AP/恢复与会话恢复 | 账号 RKS 不直接改写；完整原版判定与全部设备行为待验证 |

两者均默认关闭自动游玩；只有对应版本、进程和原生库匹配才允许激活。Phira 模块不替换原版引擎；Phigros 模块的自制谱播放器是独立子进程，不包含官方 Unity 引擎或 Phigros 游戏资源。

## 数据与网络

模块保留宿主登录、联网和原版上传流程，包括模拟成绩的上传尝试；是否接受由原版资格判断与服务器决定。修改版仅限本地、不可登录和联网。模块独立实验档的本地恢复不会撤回已经上传的记录；不复制原始账号令牌或覆盖原始存档。详见 [联网范围](docs/NETWORK-POLICY.md)。尚未做真实账号与上传验收。

## 说明、验证与源码

- [Phigros 使用与验收](docs/phigros/使用与验收.md)，[构建](modules/phigros/README.md)。
- [Phira 使用与验收](docs/phira/使用与验收.md)，[源码与构建](modules/phira/README.md)。
- `docs/*/verification.json`、`host-verification.json` 为构建时验证记录；里面 published=false 表示当时未发布，当前公开状态以 Release 和 publication.json 为准，设备验收仍为 false。
- [Phigros 自制谱播放器对应源码](chapter-player/README.md)：基于 TeamFlos/phira 固定提交，保留 GPL-3.0 许可；预发行同时提供完整对应源码压缩包。
- [免责声明](DISCLAIMER.md)。不上传签名私钥、凭据、手机日志或私人存档。

请先备份需要保留的数据，针对已知兼容版本测试。反馈附版本、问题步骤和经过脱敏的必要日志，不要公开登录令牌、私人存档或签名文件。

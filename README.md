# Phigros / Phira LSPosed 模块

**当前正式发行：0.2.2** · [统一下载页](https://github.com/yskzctx/phigros-phira-lsposed-lab/releases/tag/v0.2.2) · [安装说明](docs/INSTALL-v0.2.2.md)

两个寄生对应原版游戏的模块，在同一个发行页提供两个 APK；按需要下载，不必同时安装。与 [Phigros 离线共存修改版](https://github.com/yskzctx/phigros-offline-lab) 分开维护。

Phira 0.2.2 启动初始化修订后，用户反馈“效果不错，我很满意”。这不是全部功能和设备的完整验收。Phigros 模块未完成真机验收，按用户本次要求一并正式发布；没有把它写成已测通过。

## 0.2.3 合并修复候选

[两个模块0.2.3修复候选](https://github.com/yskzctx/phigros-phira-lsposed-lab/releases/tag/v0.2.3-pre.1) · [本次修复、原档备份与恢复](docs/INSTALL-v0.2.3.md)

用户在0.2.2后报告Phira密集/特殊谱面漏判和误触暂停，随后明确要求继续使用自研自动游玩、直接使用原版现有存档。0.2.3修正坐标完整Y转换、模拟事件作用域与密集输入时刻，Phira改为原档直连并自动备份；Phigros模块与修改版175同步修复音符数组更换时的规则缓存失效及全表查找开销。仅完成电脑端验证，候选未获本次真机确认，默认AP没有改用官方自动游玩。

- [Phira 0.2.3候选APK（原档直连）](https://github.com/yskzctx/phigros-phira-lsposed-lab/releases/download/v0.2.3-pre.1/Phira-Lab-LSPosed-0.8.2-arm64-v0.2.3-TEST.apk)
- [Phigros 0.2.3候选APK（保留原实验存档方案）](https://github.com/yskzctx/phigros-phira-lsposed-lab/releases/download/v0.2.3-pre.1/Phigros-Lab-LSPosed-157-arm64-v0.2.3-TEST.apk)

## 下载与版本

| 模块 | 直接下载 | 唯一适配宿主 |
| --- | --- | --- |
| Phigros | [APK，约47 MB](https://github.com/yskzctx/phigros-phira-lsposed-lab/releases/download/v0.2.2/Phigros-Lab-LSPosed-157-arm64-v0.2.2.apk) | com.PigeonGames.Phigros，4.0.1（157），arm64 |
| Phira | [APK，约145 KB](https://github.com/yskzctx/phigros-phira-lsposed-lab/releases/download/v0.2.2/Phira-Lab-LSPosed-0.8.2-arm64-v0.2.2.apk) | org.flos.phira，0.8.2（39），arm64 |

两个模块版本名均为0.2.2、versionCode5，与此前模块使用同一签名，可覆盖安装。需要 root 和支持原生 Hook 的 LSPosed；只勾选对应原版宿主。音量上键打开面板，下键收起；自动游玩每次冷启动默认关闭。目标与规则保存后下一张谱面生效。

直接安装 APK，无需分卷解压或下载源码。此前旧预发行已撤回为草稿并保留附件；新增0.2.3合并修复候选单独提供，正式0.2.2继续保留。

## 功能与实际验证范围

| 模块 | 已接入的功能 | 验证与已知限制 |
| --- | --- | --- |
| Phigros | 控制面板、自动游玩、目标分数/准确率、Perfect/Good早晚与随机/Bad/Miss规则、按键筛选、RKS与历史记录、独立实验存档、自制章节播放器、音符皮肤 | 主机检查通过，未真机验收；原生章节扩展特效/音效/判定线颜色未完成，宿主与自制章节皮肤存储独立 |
| Phira | 原版触摸输入自动游玩、目标分数/准确率、规则列表与按键筛选、Good早晚/随机、音量键面板、独立实验档、历史AP/恢复与会话恢复、启动诊断 | 初始化修订后获用户正向反馈；完整规则、最终结算、网络行为及所有设备未全面验收；账号RKS不直接改写 |

Phira 自制谱、完整资源包与皮肤切换使用原版功能，不重复制作播放器。目标是规划值，不覆盖最终结算，也不保证逐分精确命中。两模块均校验宿主包名、版本、ABI与原生库，不能把已核对偏移套到其他版本。

本次统一发行保留Phira已反馈可用的初始化顺序；对比确认除模块首页文字外的Java类、原生库、HTML界面和存档逻辑保持不变，仅统一版本与打包声明。

## 存档与联网

正式0.2.2使用独立实验目录是数据保护设计，不是 LSPosed 强制要求。Phira0.2.3候选已按要求改为原档直连、修改前本地自动备份，详见上方候选说明；Phigros模块仍保持原方案。历史全AP、RKS和规则产生的记录在实验档中处理；原始记录只读备份，禁用模块并冷启动可回到原始路径。当前没有“从原档导入实验档”的迁移按钮，也不直接覆盖原档。若以后以原档为基础，应先备份并显式复制需要的本地记录和资源；登录令牌不自动复制。

模块保留宿主登录、联网和原版上传流程，自动游玩与规则干预成绩也允许按原版条件尝试上传；原版资格判断、账号/服务器响应和完整性检查保留，不保证接受。实验档的本地恢复不会撤回已上传数据。没有进行真实账号或上传的完整验收。离线修改版仍仅限本地、不可登录与联网，策略与模块不同。

## 说明与源码

- [安装、作用域、独立存档与恢复](docs/INSTALL-v0.2.2.md)。
- [Phigros构建](modules/phigros/README.md) · [Phigros范围](docs/phigros/使用与验收.md)。
- [Phira构建](modules/phira/README.md) · [Phira范围](docs/phira/使用与验收.md) · [初始化修订原理](docs/STARTUP-0.2.2.md)。
- [联网范围](docs/NETWORK-POLICY.md) · [免责声明](DISCLAIMER.md)。
- [Phigros自制谱播放器对应源码](chapter-player/README.md)，发行附件提供完整对应源码与许可；普通安装不需要下载。

`docs/*/verification.json`为构建时主机报告，device_tested=false不否认随后用户反馈；本次发布和用户验收范围以publication.json和发行页为准。签名、对齐、源码构建一致和文件校验不代替真机功能验收。仓库不包含签名私钥、账号凭据、设备日志或私人存档。

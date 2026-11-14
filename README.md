# Phigros / Phira LSPosed 模块

**最新预发行：** [v0.3.2-pre.1](https://github.com/yskzctx/phigros-phira-lsposed-lab/releases/tag/v0.3.2-pre.1)。本机构建和签名已完成，尚未完成本轮真实 LSPosed 验收。

0.3.2 修复判定保护：必须打开自动游玩后才生效。分数安全时保留真实触摸；即将掉出保护目标时，在 Perfect 窗口最后一毫秒接管。关闭“屏蔽真实触摸”后，真实触摸不再被自动输入挡住。

| 模块 | 产物 | 目标 |
|---|---|---|
| Phigros | Phigros-Lab-LSPosed-157-arm64-v0.3.2-TEST.apk | 4.0.1（157）arm64 |
| Phira | Phira-Lab-LSPosed-0.8.3-arm64-v0.3.2-TEST.apk | 0.8.3（40）arm64 |

覆盖安装后强停目标游戏再打开。两个模块保留宿主登录、联网和原版上传流程，不绕过服务器、账号、购买或反作弊校验。

# Phigros / Phira LSPosed 模块

**公开预发行：** [v0.3.0-pre.1](https://github.com/yskzctx/phigros-phira-lsposed-lab/releases/tag/v0.3.0-pre.1)（真实LSPosed验收前保持预发行）

**当前：Phira0.3.0 / Phigros0.3.0（候选）** · [安装与恢复](docs/INSTALL-v0.3.0.md) · [验证边界](docs/VERIFICATION-v0.3.0.json) · [免责声明](DISCLAIMER.md)

两个0.3.0模块是同一发布周期。此次加入随机K音符规则、自动游玩时是否屏蔽真实触摸、S/FC/AT成绩保护和额外保护分数；保护只在目标规划产生判定时救判，显式规则仍然优先。**0.3.0尚未经过本轮手机/真实LSPosed验收，因此只应作为预发行候选。**

| 模块 | 下载 | 唯一适配宿主 |
|---|---|---|
|Phigros|[Phigros-Lab-LSPosed-157-arm64-v0.3.0-TEST.apk](https://github.com/yskzctx/phigros-phira-lsposed-lab/releases/download/v0.3.0-pre.1/Phigros-Lab-LSPosed-157-arm64-v0.3.0-TEST.apk)|com.PigeonGames.Phigros，4.0.1（157），arm64|
|Phira|[Phira-Lab-LSPosed-0.8.3-arm64-v0.3.0-TEST.apk](https://github.com/yskzctx/phigros-phira-lsposed-lab/releases/download/v0.3.0-pre.1/Phira-Lab-LSPosed-0.8.3-arm64-v0.3.0-TEST.apk)|org.flos.phira，0.8.3（40），arm64|

按需要下载一份即可。Phira code11、Phigros code8，使用同一签名覆盖旧模块；安装后强行停止游戏再打开，不清数据。需要root与支持原生Hook的LSPosed。音量上键打开面板，下键收起，冷启动自动游玩默认关闭。仍使用模块自研自动游玩，没有改成官方自动游玩。

Phira模块保留[可调结算时间误差（±毫秒）](docs/INSTALL-v0.2.5.md)：通过随机Perfect时机接近目标，规则优先，设置持久保存，实际效果待手机确认。

两个模块直接使用宿主原档，修改前自动备份；自动会话只恢复历史记录，保留账号、资源与设置。完整恢复方法见安装说明。Phigros仍只适配已核对完整ELF的4.0.1（157）；4.0.1.2尚未取得APK/ELF，不能仅改版本号冒充适配。

**免责声明在实际使用中展示**：模块自身首页、每次进程冷启动首次进入控制功能的原生提示、游戏内控制面板顶部均有完整说明。模块保留原版网络/登录/上传流程及原版资格判断；不伪造上传成功，不取消服务器或反作弊校验。模拟成绩不是真实手动成绩，不可用于冒充或误导性证明。

控制目标包括AP/目标分数/准确率、Perfect/Good早晚及随机/Bad/Miss规则、随机K、按键筛选、S/FC/AT保护与保护分数。Phigros还包含历史AP/恢复和历史RKS；Phira继续使用自身已有谱面、音符皮肤、账号和RKS，不重复制造这些功能。**原生章节扩展特效/音效/判定线颜色等未完成项仍存在，不声称两模块与修改版完全一致。**

已检查主机回归、签名/16KB、APK内免责声明、九个原生入口前缀及公开源码构建一致性。编译与主机探针不等于手机接入或游玩成功。代码在modules/；完整自制谱后端对应源码随发行附件提供。不得公开私人备份、日志、密钥或凭据。

## 发行历史

公开下载只保留当前 `v0.3.0-pre.1`。0.1.0至0.2.7的预发行、候选和草稿已撤下；仓库中的安装说明、验证文件与源码快照继续用于追溯，不应再安装旧二进制。真实设备验收完成前，0.3.0保持预发行状态。

本轮两模块均修正免责声明按钮的浅色布局；Phira保留0.8.3（40）适配。[0.3.0 安装说明](docs/INSTALL-v0.3.0.md)。尚未做本轮真机验收。

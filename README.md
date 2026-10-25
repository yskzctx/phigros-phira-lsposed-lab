# Phigros / Phira LSPosed 模块

**当前：0.2.4 修复候选** · [统一下载页](https://github.com/yskzctx/phigros-phira-lsposed-lab/releases/tag/v0.2.4-pre.1) · [安装、原档备份与恢复](docs/INSTALL-v0.2.4.md) · [免责声明](DISCLAIMER.md)

Phigros旧模块已收到第三方“无法使用”反馈；维护者本人没有下载该游戏，也没有替本轮做手机验收。排查确认旧版包名保护会在原生库加载前拒绝正版宿主，现已修正并补回归；旧版不应再作为可用正式版推荐。**0.2.4尚未经过手机/真实LSPosed验收。**

| 模块 | 下载 | 唯一适配宿主 |
|---|---|---|
|Phigros|[0.2.4候选APK](https://github.com/yskzctx/phigros-phira-lsposed-lab/releases/download/v0.2.4-pre.1/Phigros-Lab-LSPosed-157-arm64-v0.2.4-TEST.apk)|com.PigeonGames.Phigros，4.0.1（157），arm64|
|Phira|[0.2.4候选APK](https://github.com/yskzctx/phigros-phira-lsposed-lab/releases/download/v0.2.4-pre.1/Phira-Lab-LSPosed-0.8.2-arm64-v0.2.4-TEST.apk)|org.flos.phira，0.8.2（39），arm64|

按需要下载一份即可。两模块code7，同签名覆盖；安装后强行停止游戏再打开，不清数据。需要root与支持原生Hook的LSPosed。音量上键打开面板，下键收起，冷启动自动游玩默认关闭。仍使用模块自研自动游玩，没有改成官方自动游玩。

本轮修复Phigros包名误拒绝、只检查一次原生就绪、加载先后顺序；两模块均直接使用原档，修改前备份。Phigros自动会话只恢复谱面记录，保留账号与其他设置；旧实验数据不合并不删除。完整恢复方法见安装说明。

**免责声明在实际使用中展示**：模块自身首页、每次进程冷启动首次进入控制功能的原生提示、游戏内控制面板顶部均有完整说明。模块保留原版网络/登录/上传流程及原版资格判断；不伪造上传成功，不取消服务器或反作弊校验。模拟成绩不是真实手动成绩，不可用于冒充或误导性证明。

控制目标包括AP/目标分数/准确率、Perfect/Good早晚及随机/Bad/Miss规则和按键筛选、历史AP与恢复。Phigros还包含历史RKS和独立自制章节播放器；Phira使用自身已有谱面/皮肤管理，不把这些冒充新增功能。**原生章节扩展特效/音效/判定线颜色等未完成项仍存在，不承诺两模块与修改版完整同等。** Phira账号RKS不直接改写。

已检查主机回归、签名/16KB、APK内免责声明、九个原生入口前缀及公开源码构建一致性。编译与主机探针不等于手机接入或游玩成功。代码在modules/；完整自制谱后端对应源码随发行附件提供。不得公开私人备份、日志、密钥或凭据。

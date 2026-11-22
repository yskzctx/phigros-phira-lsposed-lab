# Phigros / Phira LSPosed 模块

**当前预发行：[v0.7.1](https://github.com/yskzctx/phigros-phira-lsposed-lab/releases/tag/v0.7.1-pre.1)**（Phigros）· [v0.5.0](https://github.com/yskzctx/phigros-phira-lsposed-lab/releases/tag/v0.5.0-pre.1)（Phira）

> **未经过真机 LSPosed 验收。** 电脑端能验证的是编译、签名、原生库齐全、dex、manifest 声明和 25 项回归测试；**不能**替代真实 LSPosed 环境下的注入、启动和整局游玩验证。

两个模块都只做运行时附加：**不修改游戏 APK、不改签名、不重打包**。保留宿主登录、联网和原版上传流程，不绕过服务器、账号、购买或反作弊校验。

| 模块 | 产物 | 目标 |
|---|---|---|
| Phigros | `Phigros-Lab-LSPosed-157-161-arm64-v0.7.1-TEST.apk` | 4.0.1(157) **或** 4.1.0(161) arm64 |
| Phira | `Phira-Lab-LSPosed-0.8.3-arm64-v0.5.0-TEST.apk` | 0.8.3(40) arm64 |

覆盖安装后强停目标游戏再打开。

---

## Phigros 模块

### 支持 4.1.0

按**函数序言匹配**自动选钩子表，不按版本号猜：

- 遍历候选表（157 用 `core/expected-hooks.h`，4.1.0 用 `core/expected-hooks-161.h`）
- 选第一张 9 个函数序言全部匹配当前 `libil2cpp.so` 的表
- 157 表不匹配时自动落到 4.1.0 表
- `bad_call` 随表走（157 `0x1d875c4` / 4.1.0 `0x1d89dcc`）

`GameProfile` 接受 157 和 161，各自校验 libil2cpp 的 sha256。

### 功能

**这局怎么打（判定保护 `mode=3`）**
- 保护 AP / FC / 分数
- 通过 `active_plan()` 生效——**不需要开自动游玩**。模块从不注入触摸（`active()` 恒假），计划判定独立运行

**结算页强制显示**
- 8 个独立字段：分数、准确率、P/G/B/M、最大连击
- 结算符号：不干预 / Φ / FC（符号由判定数量派生，选 Φ 会把 Perfect 设为总数）
- Good 早/晚：可选开关
- 独立开关，接受真实一局不可能出现的组合

**目标历史 RKS**
- 四位小数，上限 17.32（= 全 AP）

**恢复历史成绩与全解锁**
- 全解锁 ＋ 全部 AP / 仅全解锁 / 恢复上次备份

**导入 / 导出存档**
- 导入游戏原始 `playerprefs.xml` 或面板导出的 ZIP
- 先检查（来源、条目数、大小）→ 确认 → 自动备份 → 写入
- 备份列表一键回退；恢复限制在备份目录内
- 不解密不重加密，账号身份完整保留
- 导入后需完全退出重启

**我的自制章节** / **音符皮肤** / **初始 Data** 与离线版一致。

### 曲库

**1072 谱面 / 329 首**（4.1.0 数据）。157 的 1037 条全部保留，新增 35 条。

35 个新谱面定数缺失 → 留空 → NaN → 从 RKS 计算排除（`RksPlanner.known()`）。不编造数值。

### 边界：绝不隐藏或阻止登录

登录隐藏只属于离线修改版（它本来不联网）。模块运行在原版游戏里，登录、账号、云同步必须正常。

`tools/check_module_no_login_block.py` 强制检查三项：
- 模块源码不含 `privacy_hooks` / `hide_account` / `login_hidden` / `StatsFrameController` 等符号
- 产物不含 `libphigros_privacy.so`
- 原生库不导出相关符号

### 已知限制

- **4.1.0 皮肤渲染不可用**：12 个 Unity 辅助 RVA 已恢复 7 个，5 个未解（`get_rect` / `get_pivot` / `op_Implicit` / `Texture2D..ctor` / `LoadImage`）。已安全降级——库能加载、不装错 hook、不崩，但功能不可用。判定保护、结算、成绩、曲库不受影响。

---

## Phira 模块

### 功能

- **这局怎么打**：判定保护 `mode=3`
- **结算时间误差**：目标 ±ms
- **结算页强制显示**：分数、准确率、P/G/B/M、最大连击、结算符号、Good 早/晚
- **Phira 历史记录**：目标 RKS
- **恢复历史成绩**

Phira 原生支持自制谱和自制音符，模块不干预。

### 已修的 bug

判定保护在模块里曾完全无效：`judge_update` 里 `requested = atomic_load(&enabled)`，而 `enabled` 是自动游玩开关——模块从不开启自动游玩，所以 `observed_judge` 恒为 NULL，`commit_observer` 的保护分支整个被跳过。

现在 `requested = enabled || mode==3`，判定保护独立生效。

---

## 构建

```bash
python modules/phigros/build_module.py \
    --sdk <Android SDK> --ndk <NDK> --xposed-api <api-82.jar> \
    --keystore <jks> --alias <alias> --password-file <txt> \
    --resources-from-apk <离线版 APK> \
    --chapter-so <libphigros_chapter.so> \
    --out <输出目录>
```

`--resources-from-apk` 的校验按**内容**判定，不是哈希白名单：
- 包名必须是 `com.PigeonGames.Phigros.offline`
- 必须含 `assets/offline/controls-ui.html`
- 必须无 INTERNET 权限

原版游戏三条全不符合，且不会因重建而失效。

### 原生库

模块必须自带它加载的每个库，缺任一启动即崩：
- `libphigros_lsp.so`（判定/结算 hook）
- `libphigros_skins.so`（皮肤渲染）
- `libphigros_chapter.so`（自制章节引擎，Rust 构建，无 Rust 工具链时从 `build-cache/` 取）

---

## 变更记录

见 [CHANGELOG.md](CHANGELOG.md)。

## 免责声明

见 [DISCLAIMER.md](DISCLAIMER.md)。

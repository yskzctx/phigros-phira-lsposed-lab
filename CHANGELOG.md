# 变更记录

按时间倒序。所有版本均未真机 LSPosed 验收，电脑端检查见 offline 仓库的 `tools/run_all_tests.py`。

---

## Phigros 模块 v0.7.1 — 资源 APK 校验改为内容校验 + 存档往返修复

- 构建脚本不再用硬编码 sha256 白名单校验 `--resources-from-apk`（每版都要改代码，过期时会静默挡住正常构建）。改为内容校验：包名 + `assets/offline/` + 无 INTERNET 权限
- 同步离线仓库对 `SaveTransfer` 的两处修复：ZIP 只读出第一个条目、损坏压缩包报错不可读

## Phigros 模块 v0.7.0 — 4.1.0 曲库

- 曲库换成 1072 谱面 / 329 首
- **修复硬编码的 `size()!=1037`**：那行会直接拒绝任何新曲库，是「模块曲库不更新」的真正原因。改为结构性校验（非空、key 唯一、定数合理）
- 空定数解析为 NaN，`RksPlanner.known()` 从 RKS 计算排除
- 状态文本不再写死 `AP %d / 1037`，改为真实谱面数

## Phigros 模块 v0.6.0 — 导入 / 导出存档

- 面板新增「导入 / 导出存档」：导入原始 `playerprefs.xml` 或面板 ZIP，先检查再写、自动备份、一键回退
- 通过 `ModuleScreens` 用对话框 + `ACTION_OPEN_DOCUMENT` 实现，与皮肤选择器一致（不新增 Activity）

## Phigros 模块 v0.5.1 — 判定保护失效 + 拒绝修改存档

- **判定保护在模块里完全无效**：所有钩子只 gate 在 `enabled`（自动游玩开关）上，而模块启动时走 `changeAutoplay(false)`，于是计划算了却从不执行。拆分为 `active()`（触摸注入）与 `active_plan()`（计划判定，自动游玩 **或** mode 3）
- **「拒绝修改其他包的存档」**：`RksEditor.applyPending` 只允许 `com.PigeonGames.Phigros.offline`，但模块运行在原版 `com.PigeonGames.Phigros` 里，守卫把自己的宿主拒了。两个包名都放行
- 两者都写进回归测试，并验证过对旧逻辑会失败

## Phigros 模块 v0.5.0 — 支持 4.1.0

- 原生入口按函数序言匹配自动选钩子表（157 / 4.1.0），`bad_call` 随表走
- `GameProfile` 接受 157 和 161
- manifest 描述纠正：原来写「157 与 160」，160 从来不存在
- 结构偏移已验证两版一致（`verify_struct_offsets.py`），原生结构代码不改

## Phira 模块 v0.5.0 — 判定保护失效修复 + 结算符号

- **判定保护完全无效**：`requested = atomic_load(&enabled)`，模块从不开启自动游玩，`observed_judge` 恒 NULL，`commit_observer` 保护分支被跳过。改为 `enabled || mode==3`
- 结算页新增结算符号（Φ / FC）与 Good 早/晚

## Phigros 模块 v0.4.0 — 自制章节引擎补齐

- `build_module.py` 缺少 `libphigros_chapter.so` 时硬报错（除非显式 `--without-chapter`），修复「自制章节 UI 死掉」
- 解析顺序：`--chapter-so` → 资源 APK → `build-cache/`

## Phira 模块 v0.3.5 — 判定保护细节

- 修复模式 3 下 Click 在 PreJudge 被多次判定导致连击超限、分数突破百万
- 修复模式 3 下手打 Perfect 被误降级为 Good
- Good 判定窗口修正为 0.19s

## Phigros 模块 v0.3.8 — 全解锁补齐

- 修复「恢复全部 AP」未写入曲目、收藏品、曲绘、头像、章节解锁
- 补全 12 项遗漏曲目解锁键
- 新增「仅全解锁（保留成绩）」

## 更早

0.3.2 及之前：判定保护改为必须开自动游玩、临界接管；模式 3 手打 Perfect 修复等。

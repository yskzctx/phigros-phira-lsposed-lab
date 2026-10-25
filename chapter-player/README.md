# 自制谱播放器对应源码

本目录用于 **Phigros 模块的独立自制谱播放器**，不替换 Phira 原版引擎。基于 [TeamFlos/phira](https://github.com/TeamFlos/phira/tree/c830dee5e07a2b8f06104576f30d8045e0859aa7) 提交 `c830dee5e07a2b8f06104576f30d8045e0859aa7`，GPL-3.0-only；完整许可见 LICENSE-GPL-3.0.txt。

0.2.2发行额外附带 `Phigros-chapter-corresponding-source-v0.2.2.tar.gz`：它是用于当前后端的完整已修改源码，不含 Cargo 缓存、target、原版 APK、凭据或私人数据。可直接构建，不要再运行补丁脚本。此目录内 `.rs` 和 apply_player_patch.py 是固定上游的独立补丁形式，供审查和从上游重建使用。

构建环境为 Rust nightly-2026-01-01、aarch64-linux-android、NDK26.3.11579264。将 Android 链接器、CC/CXX、AR 指向该 NDK 的 ARM64 API26 工具，使用：

```text
cargo +nightly-2026-01-01 rustc --locked --release --target aarch64-linux-android -p phira --no-default-features -- -C link-arg=-Wl,-z,max-page-size=16384
```

输出 `target/aarch64-linux-android/release/libphira.so`，作为 Phigros 模块构建器的 `--chapter-so` 参数。构建后必须核对 ELF PT_LOAD 为16KB对齐。当前发行所用后端 SHA-256：`d03dba5088b4f83dcd9fe45a708938f3879e5af40a76cced02e8674de46e79a8`；编译环境不同可导致文件哈希不同。

Android 表面/Activity 适配源自 Mivik/prpr-miniquad，保留 MIT/Apache 声明，位于模块 assets/offline 的许可文件中。默认字体、声音、图片等资产许可不因此改变；Phigros 官方资源不属于此上游许可，也没有打入该模块。

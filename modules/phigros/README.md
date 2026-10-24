# Phigros 4.0.1（157）ARM64 LSPosed 开发模块

源代码包括宿主适配、复用的控制/皮肤/章节 Java、原生控制与皮肤源码。不是改包名共存版；只作用于 `com.PigeonGames.Phigros`。

当前完整范围与限制见 [使用与验收](../../docs/phigros/使用与验收.md)。原生章节扩展特效、音效和判定线颜色未完成；自制章节与宿主两处皮肤存储独立。尚未真机验收。

## 构建

需要 JDK 17+、Python 3.11+、Android SDK platform 36 / build-tools 36.0.0、NDK 26.3.11579264，以及官方 [Xposed API 82](https://api.xposed.info/de/robv/android/xposed/api/82/api-82.jar) 编译依赖，不打包 Xposed API 定义。

```text
python build_module.py --sdk <SDK目录> --ndk <NDK目录> --xposed-api <api-82.jar> --keystore <自有签名库> --alias <别名> --password-file <本地密码文件> --resources-from-apk <本预发行的Phigros模块APK> --out <输出目录>
```

脚本校验资源输入 APK 的完整 SHA-256，只从匹配的公开模块候选中读取默认播放器资源及现有后端，不接受原版游戏、其他 APK 或私人构建作为这个输入。这个资源输入不是签名私钥，也不包含原版 Phigros 引擎。默认字体/资源包等资产沿用上游许可，完整资源已经随对应公开候选提供。

脚本重新编译本仓库的 Java、原生控制适配和音符皮肤库。自制谱后端可按 [对应源码说明](../../chapter-player/README.md) 自行编译，然后用 `--chapter-so <libphira.so>` 替换默认后端输入。该后端只在模块的自制谱子进程运行。

签名库与密码文件必须由构建者在仓库外提供；仓库不包含发布者私钥。输出检查签名、16KB 对齐和 APK CRC；这些仍不是手机验收。

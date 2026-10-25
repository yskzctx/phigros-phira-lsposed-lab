# Phira 0.2.2：原生初始化顺序修订

用户截图为：独立存档激活被拒绝；N=1 A=0 S=5 E=0 R=0 P=1 T=1。它证明游戏进程内Java面板运行、原生Hook最终安装、收到一次路径调用且进行过一次失败激活；不是未勾作用域。截图没有记录激活时刻原生状态，不能单凭它保证唯一根因。

代码发现与该现象一致的缺陷：Entry使用XposedHelpers.findClass解析QuadNative，该API使用initialize=false，不会保证运行类的静态初始化。原版APK的QuadNative.<clinit>负责System.loadLibrary("phira")和initializeEnvironment。旧代码仅确保模块库先加载，未保证宿主库也在首次路径激活前完成初始化。

修订：模块库先注册回调，再以Class.forName(name,true,hostLoader)完成原版入口类初始化，之后安装路径Hook与执行激活。没有手工置ready/armed成功，没有在游戏已经读原始数据之后补切目录，没有取消版本/库哈希/特征或提前运行检查。原版账号、联网与上传资格流程不变。

此前JVM正向测试将目标解析器写成initialize=true，未覆盖生产的lazy解析。新增测试先重现“类仅解析→激活失败→原始调用才初始化→后来ready但仍未激活”的主机状态，再验证新代码在激活前完成初始化；测试不模拟真实LSPosed/ART，因此手机效果仍待确认。

游戏面板标题栏新增“复制诊断”，直接复制当前进程状态，不依赖模块入口的连接报告回传。失败消息保留当时读取的原生状态便于后续排查。“尚未收到连接报告”不代表游戏面板或原生Hook未加载；没有通过模块入口发起启动或启动Intent未更新时回传可能未到达。

安装：覆盖安装Phira-Lab-LSPosed-0.8.2-arm64-v0.2.2-TEST.apk；版本Code4/0.2.2-test，官方org.flos.phira 0.8.2（39）arm64，保留原有模块签名。LSPosed只勾选Phira，在系统应用设置“强行停止”游戏再启动。不要清除或卸载原版数据。不需要电脑或USB。

先试自动游玩开关能否开启，是否显示独立存档已切换。若仍失败，直接点游戏面板右上“复制诊断”发送文字即可；这次截图已经提供足够证据用于本修订，无需重复提供同一张。

主机JVM/C组件测试、签名/16KB对齐、公开源码DEX/原生库/HTML逐项一致检查通过；javap已确认Entry编译产物调用loadAndInitialize后才安装setDataPath Hook。版本与推荐作用域清单已从APK核对。仍未在用户手机安装或验证，不声明实际启动和结算已通过。

参考：XposedHelpers.findClass实现 https://github.com/rovo89/XposedBridge/blob/art/app/src/main/java/de/robv/android/xposed/XposedHelpers.java

"""Build the Phigros LSPosed adapter; never repackage the host game."""
import argparse,hashlib,os,struct,subprocess,zipfile
from pathlib import Path,PurePosixPath
p=argparse.ArgumentParser(description=__doc__)
for arg in ['sdk','ndk','xposed-api','keystore','password-file','out','resources-from-apk']:p.add_argument('--'+arg,required=True,type=Path)
p.add_argument('--alias',required=True);p.add_argument('--chapter-so',type=Path);p.add_argument('--build-tools',default='36.0.0');p.add_argument('--platform',default='android-36')
# The local-chapter engine is a real feature, not an optional extra: shipping without
# it silently removes自制章节 from the module. Omitting it must therefore be an
# explicit decision, never a side effect of a missing file.
p.add_argument('--without-chapter',action='store_true',help='explicitly build without the local-chapter engine')
a=p.parse_args();root=Path(__file__).resolve().parent;out=a.out.resolve();out.mkdir(parents=True,exist_ok=True);build=out/'module-build';build.mkdir(exist_ok=True)
def run(*args):subprocess.run([str(v)for v in args],check=True)
def digest(path):
 with path.open('rb')as f:return hashlib.file_digest(f,'sha256').hexdigest()
assert digest(a.resources_from_apk)in {'28ea2851b276024e1fe41c7f1f7ff8ac9f334b6e7b9c75cf9bc31b9003cd726c','d186832bd2600a1353ef6421d9159987d88639756ffc02f2a120bcd5f6597968','61fedd402042ad301de66b87a0e2ed844568b242a1052c1e8cf07401041195ac','edec4ce3f4ca761085d2a9fe745ac47082e5bbf1f2a51e39d001a7b966643ded','6557fa5c12789f36a897c26bd59b4494faefe0eb9ef3c92841fa23fae2f504b6','95810e0b3beccb759ac22df8e1b20212d174926fe86a6624201af66d18e80bd1','b329f3dfe4e6351c36136379239e6f61eab4a20a5a16a46375060df92cf5e4ed'},'Use the exact verified v174/v175/v177/v181 modified APK (or retained 0.1 module), not an original game or private APK'
sdk=a.sdk.resolve();tools=sdk/'build-tools'/a.build_tools;android=sdk/'platforms'/a.platform/'android.jar';api=a.xposed_api.resolve()
classes=build/'classes';classes.mkdir(exist_ok=True)
# A fresh javac output prevents removed anonymous classes entering the DEX.
for obsolete in classes.rglob('*.class'):
 if not obsolete.is_symlink() and classes.resolve() in obsolete.resolve().parents:obsolete.unlink()
dex=build/'dex';dex.mkdir(exist_ok=True)
run('javac','--release','8','-encoding','UTF-8','-classpath',str(android)+os.pathsep+str(api),'-d',classes,*sorted((root/'src').rglob('*.java')))
run('java','-Djava.io.tmpdir='+str(build),'-cp',tools/'lib/d8.jar','com.android.tools.r8.D8','--min-api','26','--lib',android,'--lib',api,'--output',dex,*sorted(classes.rglob('*.class')))
host='windows-x86_64'if os.name=='nt'else'linux-x86_64';compiler=a.ndk.resolve()/'toolchains/llvm/prebuilt'/host/'bin'/('aarch64-linux-android26-clang.cmd'if os.name=='nt'else'aarch64-linux-android26-clang')
native={}
for leaf,source in [('libphigros_lsp.so','lsp-entry.c'),('libphigros_skins.so','skins.c')]:
 path=build/leaf;native[leaf]=path
 run(compiler,'-shared','-fPIC','-O2','-std=c11','-Wall','-Wextra','-Werror','-Wno-unused-function','-Wno-unused-variable','-fstack-protector-strong','-Wl,-z,max-page-size=16384','-Wl,-z,relro,-z,now','-o',path,root/'native'/source,'-llog','-ldl','-lm')
resources=build/'resources.apk';run(tools/('aapt2.exe'if os.name=='nt'else'aapt2'),'link','-I',android,'--manifest',root/'AndroidManifest.xml','-o',resources)
unsigned=build/'unsigned.apk';aligned=build/'aligned.apk'
with zipfile.ZipFile(resources)as res,zipfile.ZipFile(a.resources_from_apk)as old,zipfile.ZipFile(unsigned,'w')as z:
 for e in res.infolist():z.writestr(e,res.read(e))
 z.write(dex/'classes.dex','classes.dex',compress_type=zipfile.ZIP_DEFLATED)
 for e in old.infolist():
  if e.filename.startswith('assets/'):
   assert '..'not in PurePosixPath(e.filename).parts and '\\'not in e.filename
   if e.filename in {'assets/xposed_init','assets/native_init','assets/xposed_scope'}:continue
   if e.filename.startswith(('assets/aa/','assets/bin/')):continue
   z.writestr(e,(root/'assets/offline/controls-ui.html').read_bytes()if e.filename=='assets/offline/controls-ui.html'else old.read(e))
 for name,file in native.items():z.write(file,'lib/arm64-v8a/'+name,compress_type=zipfile.ZIP_STORED)
 # The local-chapter backend is a Rust build (see chapter-player/README.md). It is a
 # shipped feature: the module registers ChapterLibraryActivity / ChapterPlayerActivity
 # and ChapterPlayerActivity calls System.loadLibrary("phigros_chapter"), so a module
 # without this library has dead UI and no chapter playback.
 #
 # Resolution order: --chapter-so, then the resource APK (older modules carried it),
 # then a sibling build-cache copy. Missing entirely is a hard error unless
 # --without-chapter was passed.
 chapter=None;chapter_source=None
 if a.chapter_so:
  chapter=a.chapter_so.read_bytes();chapter_source=str(a.chapter_so)
 elif 'lib/arm64-v8a/libphigros_chapter.so' in old.namelist():
  chapter=old.read('lib/arm64-v8a/libphigros_chapter.so');chapter_source='resource APK'
 else:
  cached=root.parent.parent/'build-cache'/'libphigros_chapter.so'
  if cached.is_file():chapter=cached.read_bytes();chapter_source=str(cached)
 if chapter:
  assert chapter[:5]==b'\x7fELF\x02'and struct.unpack_from('<H',chapter,18)[0]==183,'chapter library is not an aarch64 ELF'
  z.writestr('lib/arm64-v8a/libphigros_chapter.so',chapter,compress_type=zipfile.ZIP_STORED)
  print(f'chapter engine: {len(chapter)} bytes from {chapter_source}')
 elif a.without_chapter:
  print('WARNING: --without-chapter given; the local-chapter feature is NOT in this build')
 else:
  raise SystemExit(
   'libphigros_chapter.so not found. This module ships a local-chapter engine, and building\n'
   'without it silently removes that feature (ChapterPlayerActivity calls\n'
   'System.loadLibrary("phigros_chapter")). Provide it with --chapter-so, or place it at\n'
   'build-cache/libphigros_chapter.so, or pass --without-chapter to accept the loss.')
 z.writestr('assets/xposed_init','com.phigros.lab.Entry\n');z.writestr('assets/native_init','libphigros_lsp.so\n');z.writestr('assets/xposed_scope','com.PigeonGames.Phigros\n')
zipalign=tools/('zipalign.exe'if os.name=='nt'else'zipalign');run(zipalign,'-P','16','-f','4',unsigned,aligned)
apk=out/'Phigros-Lab-LSPosed-157-161-arm64-v0.6.0-TEST.apk'
run('java','-Djava.io.tmpdir='+str(build),'-jar',tools/'lib/apksigner.jar','sign','--ks',a.keystore.resolve(),'--ks-key-alias',a.alias,'--ks-pass','file:'+str(a.password_file.resolve()),'--min-sdk-version','26','--v1-signing-enabled','false','--v2-signing-enabled','true','--v3-signing-enabled','true','--v4-signing-enabled','false','--out',apk,aligned)
run('java','-jar',tools/'lib/apksigner.jar','verify','--verbose',apk);run(zipalign,'-c','-P','16','4',apk)
with zipfile.ZipFile(apk)as z:
 assert z.testzip()is None
 assert not any(n.startswith(('assets/aa/','assets/bin/'))or n.endswith(('libunity.so','libil2cpp.so'))for n in z.namelist()),'Host engine/resources must never enter the module'
(out/'SHA256SUMS.txt').write_text(digest(apk)+'  '+apk.name+'\n',encoding='utf-8')
for file in [unsigned,aligned,resources]:file.unlink()
print('Built module:',apk)

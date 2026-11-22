"""Guard the module build against silently dropping shipped features.

The 0.4.0 module was built without libphigros_chapter.so because the build script
treated a missing chapter engine as a warning. The APK still installed and started,
so nothing failed loudly - the local-chapter feature was simply gone. This test pins
the rule: a built module must contain every native library its Java code loads.

It reads the Java sources for System.loadLibrary names, maps them to the library
files the module must contain, and checks the built APK. Run it after a build.
"""
from __future__ import annotations

import re
import sys
import zipfile
from pathlib import Path

MODULE = Path(__file__).resolve().parent.parent
APK = None  # resolved in main() as the newest built Phigros module APK

LOAD = re.compile(r'System\.loadLibrary\(\s*"([^"]+)"\s*\)')

# Libraries provided by the Xposed/LSPosed runtime or the Android platform, not by
# this module, so their absence from the APK is expected.
PLATFORM = {'android', 'log', 'c', 'm', 'dl'}


def loaded_libraries() -> dict[str, list[str]]:
    found: dict[str, list[str]] = {}
    for source in sorted((MODULE / 'src').rglob('*.java')):
        for name in LOAD.findall(source.read_text(encoding='utf-8')):
            found.setdefault(name, []).append(source.name)
    return found



def newest_apk(prefix: str):
    """Newest built APK whose name starts with prefix, or None."""
    candidates = sorted(Path(r'I:\dsh\work\release\modules').glob(f'{prefix}*.apk'),
                        key=lambda p: p.stat().st_mtime, reverse=True)
    return candidates[0] if candidates else None

def main() -> int:
    global APK
    APK = newest_apk("Phigros-Lab-LSPosed-")
    libraries = loaded_libraries()
    if not libraries:
        print('FAIL no System.loadLibrary calls found; the scan is broken')
        return 1

    print(f'libraries loaded by module Java: {len(libraries)}')
    for name, files in sorted(libraries.items()):
        print(f'  lib{name}.so  <- {", ".join(sorted(set(files)))}')

    if not APK.is_file():
        print(f'\nSKIP {APK} not built yet; the library set is the only check available')
        expected = {name for name in libraries if name not in PLATFORM}
        print(f'expected in a build: {sorted("lib" + n + ".so" for n in expected)}')
        return 0

    with zipfile.ZipFile(APK) as archive:
        present = {Path(n).name for n in archive.namelist()
                   if n.startswith('lib/arm64-v8a/') and n.endswith('.so')}
    print(f'\nnative libraries in {APK.name}: {sorted(present)}')

    missing = []
    for name in sorted(libraries):
        if name in PLATFORM:
            continue
        expected = f'lib{name}.so'
        if expected not in present:
            missing.append(f'{expected} (loaded by {", ".join(sorted(set(libraries[name])))})')

    print()
    if missing:
        print('FAIL the built module is missing native libraries its code loads:')
        for entry in missing:
            print(f'  {entry}')
        print('\nA module missing these still installs and starts, so the loss is silent.')
        return 1
    print(f'PASS all {len(libraries) - len(set(libraries) & PLATFORM)} module libraries present')
    return 0


if __name__ == '__main__':
    sys.exit(main())

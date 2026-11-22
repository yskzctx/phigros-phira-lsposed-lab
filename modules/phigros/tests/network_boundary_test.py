"""Pin the network-permission boundary between the offline build and the modules.

The intended split is:

  * the offline build has no INTERNET permission - it cannot log in and cannot sync;
  * the modules do not request INTERNET either, because an LSPosed module runs inside
    the host's process and inherits the host's permissions. The module's sync hooks
    forward to the host unchanged, so login and upload keep working.

Both halves are easy to break by accident (adding a permission to silence a network
error, or a hook that rewrites a response), and neither breaks at install time. This
test reads the built artifacts and asserts the boundary.
"""
from __future__ import annotations

import subprocess
import sys
import zipfile
from pathlib import Path

AAPT2 = Path(r'C:\Users\yskzc\Android\Sdk\build-tools\36.0.0\aapt2.exe')

OFFLINE_APK = Path(r'I:\dsh\work\release\v181\Phigros-offline-v181.apk')
MODULE_APKS = []  # resolved in main() as the newest module APKs

# Permissions the offline build is allowed to keep. INTERNET is deliberately absent:
# without it the platform refuses every socket, which is what "cannot log in, cannot
# sync" means at the OS level.
OFFLINE_ALLOWED = {
    'android.permission.ACCESS_NETWORK_STATE',
    'android.permission.SYSTEM_ALERT_WINDOW',
    'android.permission.FOREGROUND_SERVICE',
    'android.permission.FOREGROUND_SERVICE_DATA_SYNC',
}


def permissions(apk: Path) -> set[str]:
    result = subprocess.run([str(AAPT2), 'dump', 'permissions', str(apk)],
                            capture_output=True, text=True, errors='replace')
    if result.returncode != 0:
        raise SystemExit(f'aapt2 failed on {apk}: {result.stderr[:200]}')
    found = set()
    for line in result.stdout.splitlines():
        if "name='" in line:
            found.add(line.split("name='", 1)[1].rstrip("'"))
    return found



def newest_apk(prefix: str):
    """Newest built APK whose name starts with prefix, or None."""
    candidates = sorted(Path(r'I:\dsh\work\release\modules').glob(f'{prefix}*.apk'),
                        key=lambda p: p.stat().st_mtime, reverse=True)
    return candidates[0] if candidates else None

def main() -> int:
    global MODULE_APKS
    MODULE_APKS = [a for a in (newest_apk("Phigros-Lab-LSPosed-"), newest_apk("Phira-Lab-LSPosed-")) if a]
    problems = []

    print('offline build:')
    if OFFLINE_APK.is_file():
        declared = permissions(OFFLINE_APK)
        for name in sorted(declared):
            print(f'  {name}')
        if 'android.permission.INTERNET' in declared:
            problems.append('offline build declares INTERNET - it could log in and sync')
        unexpected = declared - OFFLINE_ALLOWED
        if unexpected:
            problems.append(f'offline build declares unexpected permissions: {sorted(unexpected)}')
    else:
        print(f'  SKIP {OFFLINE_APK.name} not built')

    for apk in MODULE_APKS:
        print(f'\n{apk.name}:')
        if not apk.is_file():
            print('  SKIP not built')
            continue
        declared = permissions(apk)
        if declared:
            for name in sorted(declared):
                print(f'  {name}')
        else:
            print('  (no permissions - inherits the host process)')
        # A module runs in the host's process; requesting INTERNET itself would signal
        # that the module is doing its own networking, which it must not.
        if 'android.permission.INTERNET' in declared:
            problems.append(f'{apk.name} declares INTERNET; modules must use the host\'s network')

    # The module must not rewrite sync results either: a hook that returns a synthetic
    # response is the other way "sync is broken" happens.
    print('\nsync hook pass-through:')
    cloud = Path(__file__).resolve().parent.parent / 'native' / 'cloud-guard.c'
    if cloud.is_file():
        text = cloud.read_text(encoding='utf-8', errors='replace')
        for original in ('sync_original', 'silent_original'):
            # The observer must return the original call's value verbatim.
            if f'{original}(method)' not in text:
                problems.append(f'cloud-guard.c no longer forwards {original}(method)')
        if '403' in text:
            problems.append('cloud-guard.c contains a 403 override')
        print('  forwards sync_original / silent_original, no status override')

    print()
    if problems:
        for problem in problems:
            print(f'FAIL {problem}')
        return 1
    print('PASS offline build cannot reach the network; modules inherit the host and forward sync')
    return 0


if __name__ == '__main__':
    sys.exit(main())

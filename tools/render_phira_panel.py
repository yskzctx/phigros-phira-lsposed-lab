"""Render the Phira panel's forced-settlement card for visual review."""
import shutil
import subprocess
from pathlib import Path

SRC = Path(r'I:\dsh\work\phigros-phira-lsposed-lab\modules\phira\assets\controls-ui.html')
OUT_DIR = Path(r'I:\dsh\work\release\ui-preview')
OUT_DIR.mkdir(parents=True, exist_ok=True)
CHROME = r'C:\Program Files\Google\Chrome\Application\chrome.exe'

html = SRC.read_text(encoding='utf-8')
html = html.replace('<section id="play" class="page active">', '<section id="play" class="page">')
html = html.replace('<section id="history" class="page">', '<section id="history" class="page active">')
staged = OUT_DIR / 'phira-panel.html'
staged.write_text(html, encoding='utf-8', newline='\n')

shot = OUT_DIR / 'phira-panel.png'
subprocess.run([CHROME, '--headless=new', '--disable-gpu', '--hide-scrollbars',
                '--force-device-scale-factor=2', '--window-size=420,1600',
                f'--screenshot={shot}', staged.as_uri()],
               capture_output=True, text=True, errors='replace')
print('screenshot:', shot, shot.stat().st_size if shot.is_file() else 'FAILED')

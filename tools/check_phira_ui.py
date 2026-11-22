"""Verify the Phira panel keeps its id contract after the redesign."""
import re
import sys
from pathlib import Path

UI = Path(r'I:\dsh\work\phigros-phira-lsposed-lab\modules\phira\assets\controls-ui.html')
s = UI.read_text(encoding='utf-8')
start = s.find('<script>')
markup, js = s[:start], s[start:]

ids = {m.group(1) for m in re.finditer(r"""\$\(\s*['"]([A-Za-z0-9_\-]+)['"]\s*\)""", js)}
present = set(re.findall(r'\bid="([A-Za-z0-9_\-]+)"', markup))

missing = sorted(ids - present)
print(f'js ids: {len(ids)}   markup ids: {len(present)}')
if missing:
    print(f'FAIL ids referenced but absent: {missing}')
    sys.exit(1)
print('PASS every js-referenced id exists')

tabs = re.findall(r'data-tab="([\w-]+)"', markup)
pages = re.findall(r'<section id="([\w-]+)" class="page', markup)
print(f'tabs : {tabs}')
print(f'pages: {pages}')
for tab in tabs:
    if tab not in pages:
        print(f'FAIL tab {tab} has no page')
        sys.exit(1)
print('PASS every tab has a page')

# The tab handler must not depend on the removed .pages container.
if ".pages'" in js or '.pages"' in js:
    print('FAIL tab handler still references .pages')
    sys.exit(1)
print('PASS tab handler has no .pages dependency')

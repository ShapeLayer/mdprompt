"""Regenerate the TextMate bundle from the VS Code grammar."""
from pathlib import Path
import json
import plistlib

root = Path(__file__).resolve().parents[1]
grammar = json.loads((root / 'editors/vscode/syntaxes/mdprompt.tmLanguage.json').read_text())
(root / 'editors/textmate/mdprompt.tmLanguage.json').write_text(json.dumps(grammar, indent=2) + '\n')
bundle = root / 'editors/textmate/mdprompt.tmbundle'
(bundle / 'Syntaxes').mkdir(parents=True, exist_ok=True)
(bundle / 'info.plist').write_bytes(plistlib.dumps({'name':'mdprompt','uuid':'723CBE2C-B489-4599-9FCB-7372E63E55E4'}))
grammar['uuid'] = '6B044516-A153-45D3-9C1A-BC9C40E3807D'
(bundle / 'Syntaxes/mdprompt.tmLanguage').write_bytes(plistlib.dumps(grammar))

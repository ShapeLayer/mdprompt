"""mdprompt transport. Parsing is delegated to the native GFM extension."""
import json
import os
from pathlib import Path
import shutil
import subprocess

__version__ = "0.1.0"

class ParseError(ValueError):
    pass

def executable(explicit=None):
    if explicit:
        return str(explicit)
    if os.environ.get("MDPROMPT_NATIVE"):
        return os.environ["MDPROMPT_NATIVE"]
    found = shutil.which("mdprompt-cli")
    if found:
        return found
    local = Path(__file__).resolve().parents[2] / "build" / "mdprompt-cli"
    if local.is_file():
        return str(local)
    raise RuntimeError("Build mdprompt-cli, put it on PATH, or set MDPROMPT_NATIVE.")

def parse(source: str, *, native=None) -> dict:
    result = subprocess.run([executable(native), "--json"], input=source,
                            text=True, encoding="utf-8", capture_output=True, check=False)
    if result.returncode:
        raise ParseError(result.stderr.strip() or "native parser failed")
    return json.loads(result.stdout)

def messages(source: str, *, native=None) -> list[dict]:
    payload = parse(source, native=native)
    protocol = Path(__file__).with_name("protocol.txt").read_text(encoding="utf-8")
    return [{"role": "system", "content": protocol},
            {"role": "user", "content": json.dumps(payload, ensure_ascii=False)}]

def render_html(source: str, *, native=None) -> str:
    result = subprocess.run([executable(native), "--html"], input=source,
                            text=True, encoding="utf-8", capture_output=True, check=False)
    if result.returncode:
        raise ParseError(result.stderr.strip() or "native parser failed")
    return result.stdout

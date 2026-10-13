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

def _run_native(source: str, mode: str, native=None) -> str:
    # Text-mode stdin turns CRLF into CRCRLF on Windows. Keep transport binary
    # and let cmark-gfm normalize the document's original newline bytes.
    result = subprocess.run([executable(native), mode], input=source.encode("utf-8"),
                            capture_output=True, check=False)
    if result.returncode:
        raise ParseError(result.stderr.decode("utf-8", errors="replace").strip()
                         or "native parser failed")
    return result.stdout.decode("utf-8")

def parse(source: str, *, native=None) -> dict:
    return json.loads(_run_native(source, "--json", native))

def messages(source: str, *, native=None) -> list[dict]:
    payload = parse(source, native=native)
    protocol = Path(__file__).with_name("protocol.txt").read_text(encoding="utf-8")
    return [{"role": "system", "content": protocol},
            {"role": "user", "content": json.dumps(payload, ensure_ascii=False)}]

def render_html(source: str, *, native=None) -> str:
    return _run_native(source, "--html", native)

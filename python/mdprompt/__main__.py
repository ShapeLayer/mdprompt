import argparse
import json
from pathlib import Path
import sys
from . import ParseError, messages, parse, render_html

def main():
    ap = argparse.ArgumentParser(description="Parse mdprompt and prepare AI messages")
    ap.add_argument("file", nargs="?", help=".mdp or .mdprompt file; default stdin")
    ap.add_argument("--format", choices=["json", "messages", "html", "content"], default="json")
    ap.add_argument("--native", help="path to mdprompt-cli")
    args = ap.parse_args()
    try:
        source = Path(args.file).read_text(encoding="utf-8") if args.file else sys.stdin.read()
        if args.format == "html":
            sys.stdout.write(render_html(source, native=args.native))
        elif args.format == "content":
            sys.stdout.write(parse(source, native=args.native)["content"])
        else:
            output = messages(source, native=args.native) if args.format == "messages" else parse(source, native=args.native)
            print(json.dumps(output, ensure_ascii=False, indent=2))
    except (ParseError, RuntimeError, OSError) as exc:
        print(str(exc), file=sys.stderr)
        return 1
    return 0

if __name__ == "__main__":
    raise SystemExit(main())

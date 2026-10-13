# mdprompt

**mdprompt**는 GFM과 TeX 수식에 AI 지시 영역을 추가한 문서 형식입니다. 파일 확장자는 **`.mdp`**, **`.mdprompt`**입니다.

```mdprompt
# 보고서

이 문장은 문서 콘텐츠입니다. ^%이 문장을 짧게 다듬어라.%^

^%%
표의 항목과 수식을 유지하면서 보고서 전체를 교정하라.
%%^

수식 $E = mc^2$도 문서 콘텐츠입니다.
```

- `^% … %^`: 같은 줄에 작성하는 인라인 AI 지시
- 단독 줄의 `^%%` `…` `%%^`: 여러 줄에 작성하는 블록 AI 지시
- 그 밖의 GFM, 코드, HTML, `$ … $`, `$$ … $$`: 문서 콘텐츠

지시 안의 텍스트는 raw text로 추출되어 모델에 전달됩니다. (이 텍스트가 어떤 포맷으로 모델에 읽혀지는지는 모델에 따라 다를 수 있으나, 일반적으로 모델들은 마크다운을 입력으로 받습니다.) 문서 렌더링에서는 지시문이 숨김 처리됩니다.  

## 구현 구성

| 경로 | 역할 |
| --- | --- |
| `docs/spec.md` | 구분자, 우선순위, 이스케이프, 오류 처리 명세 |
| `native/` | 실제 cmark-gfm 구문 확장, 공유 라이브러리, CLI |
| `python/mdprompt/` | 같은 네이티브 파서를 쓰는 Python API 및 AI 메시지 생성 |
| `prompts/` | 파싱된 JSON용 프로토콜과 수동 전달용 설명 프롬프트 |
| `editors/` | VS Code, TextMate, Sublime Text, Vim/Neovim, Emacs 지원 |
| `examples/` | 한국어 예제 |
| `tests/` | 문법, GFM, 경계, 렌더링, AI 전달 검증 |

## 빌드 및 실행

C 라이브러리와 CLI를 빌드하려면 C 컴파일러와 CMake 3.16 이상이 필요합니다.


```sh
cmake -S . -B build
cmake --build build --parallel
ctest --test-dir build --output-on-failure

build/mdprompt-cli --json < examples/welcome.mdp
build/mdprompt-cli --content < examples/welcome.mdp
build/mdprompt-cli --html < examples/welcome.mdp
```

오프라인 또는 이미 받아 둔 cmark-gfm 소스가 있다면:

```sh
cmake -S . -B build -DFETCHCONTENT_SOURCE_DIR_CMARK=/absolute/path/to/cmark-gfm
```

Python 보조 도구를 설치하고 AI용 메시지 배열을 생성하려면:

```sh
python3 -m pip install -e .
export MDPROMPT_NATIVE="$PWD/build/mdprompt-cli"
mdprompt examples/welcome.mdp --format messages
mdprompt examples/welcome.mdp --format html
```

```python
from mdprompt import parse, messages, render_html

source = "문서 내용. ^%문장을 더 짧게 써라.%^"
payload = parse(source)
ai_messages = messages(source)
html = render_html(source)
```

# 검증

이 문법 테스트는 Python 3.10 이상을 사용합니다. C 라이브러리와 CLI의 빌드·실행에는 Python이 필요하지 않습니다. Python이 없으면 CMake는 이 테스트를 등록하지 않습니다. `-DMDPROMPT_PYTHON_TESTS=OFF`로 Python 테스트를 명시적으로 끌 수 있습니다.

```sh
cmake -S . -B build
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

테스트는 실제 네이티브 CLI를 실행해 인라인/블록 지시, 순서, 위치, UTF-8, 코드와 HTML 보호, 수식 보호, 백슬래시 홀짝, 비중첩 원문, 빈 지시, CRLF, 미완성 오류, GFM HTML, 대용량 스트리밍과 AI 메시지 권한 분리를 검증합니다.

TextMate 정규식 엔진 검증:

```sh
cd tests/textmate
npm install
npm test
```

Vim syntax 로딩 검증:

```sh
vim -Nu NONE -i NONE -n -es -c 'set rtp+=./editors/vim' -c 'syntax on' \
  -c 'e examples/welcome.mdp' -c 'set ft=mdprompt' -c 'qa!'
```

VS Code 진입점은 `node --check editors/vscode/extension.js`로 검사합니다. GUI 웹뷰, Sublime Text 실제 로딩, Emacs byte-compile 검증은 해당 에디터가 설치되어 있는 호스트에서 별도로 수행해야 합니다.

실제 VS Code의 Markdown 문법과 함께 검사하려면 `MDPROMPT_MARKDOWN_GRAMMAR` 환경 변수에 VS Code 설치 폴더의 `markdown-basics/syntaxes/markdown.tmLanguage.json` 경로를 지정하고 같은 TextMate 테스트를 실행합니다. 코드 블록의 일반 텍스트 scope와 블록을 닫은 뒤 지시 하이라이트가 다시 적용되는 동작도 검증합니다.

코드 블록의 균일 색상 범위 테스트는 같은 울타리 문자, 긴 닫기 울타리, 미완성 블록, 지시·수식 안의 울타리와 편집 후 범위 변경을 검증합니다.

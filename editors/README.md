# 에디터 지원

`.mdp`, `.mdprompt` 확장자를 갖는 파일을 mdprompt로 인식해 하이라이트합니다.  

## VS Code

`editors/vscode`를 확장 개발 폴더로 열거나 아래처럼 실행합니다.

```sh
code --extensionDevelopmentPath="$PWD/editors/vscode" examples/welcome.mdp
```

## TextMate 및 JetBrains TextMate 지원

`editors/textmate/mdprompt.tmbundle`을 TextMate에 설치합니다. `text.html.markdown` 문법이 사용 가능한 상태여야 합니다.  

## Sublime Text

`editors/sublime/mdprompt.sublime-syntax`를 `Packages/User`에 복사합니다. Markdown 패키지가 설치되어 있어야 합니다.

## Vim / Neovim

`editors/vim`을 runtimepath에 추가하거나 그 내용을 사용자 runtime 디렉터리에 복사합니다.

```vim
set runtimepath+=/absolute/path/to/mdprompt/editors/vim
filetype plugin on
syntax on
```

Markdown 하이라이트를 로드한 뒤 mdprompt 구간을 추가합니다. Neovim에서도 동일한 Vim syntax 파일을 사용합니다. Tree-sitter 문법은 제공하지 않습니다.

## Emacs

`markdown-mode`를 설치한 후:

```elisp
(add-to-list 'load-path "/absolute/path/to/mdprompt/editors/emacs")
(require 'mdprompt-mode)
```

`gfm-mode`를 기반으로 합니다. 수식 표시를 켜고 지시용 face를 추가합니다. `mdprompt-instruction-face`를 커스터마이즈해서 지시 색상을 바꿀 수 있습니다.  

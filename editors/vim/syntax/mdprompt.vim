if exists("b:current_syntax")
  finish
endif
runtime! syntax/markdown.vim
unlet! b:current_syntax
syn region mdpromptMathInline start=/\$\@<!\$\$\@!/ skip=/\\./ end=/\$/ end=/$/ oneline keepend
syn region mdpromptMathDisplayInline start=/\$\$\$\@!/ skip=/\\./ end=/\$\$/ end=/$/ oneline keepend
syn region mdpromptMathBlock start=/^ \{0,3}\$\$\s*$/ end=/^ \{0,3}\$\$\s*$/ keepend
syn region mdpromptInstructionInline matchgroup=mdpromptDelimiter start=/\^%\%(%\%(\^\)\@!\)\@!/ skip=/\\./ end=/%\^/ end=/$/ oneline keepend
syn region mdpromptInstructionBlock matchgroup=mdpromptDelimiter start=/^ \{0,3}\^%%\s*$/ end=/^ \{0,3}%%\^\s*$/ keepend
" Define protected code last so it wins at equal start positions.
syn region mdpromptCodeSpan start=/\z(`\+\)/ end=/\z1/ keepend
syn region mdpromptCodeFence start=/^ \{0,3}\z(`\{3,}\)[^`]*$/ end=/^ \{0,3}\z1`*\s*$/ keepend
syn region mdpromptCodeFence start=/^ \{0,3}\z(\~\{3,}\).*$/ end=/^ \{0,3}\z1\~*\s*$/ keepend
syn match mdpromptEscape /\\[\\^$]/
hi def link mdpromptInstructionInline Special
hi def link mdpromptInstructionBlock Special
hi def link mdpromptDelimiter Delimiter
hi def link mdpromptMathInline String
hi def link mdpromptMathBlock String
hi def link mdpromptMathDisplayInline String
hi def link mdpromptCodeSpan String
hi def link mdpromptCodeFence String
hi def link mdpromptEscape SpecialChar
let b:current_syntax = "mdprompt"

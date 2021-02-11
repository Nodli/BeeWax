set background=dark
set fillchars=

"hi clear
if exists("syntax_on")
    syntax reset
endif

" text
hi! Normal guifg=#d6b48b guibg=grey20 gui=NONE
hi! Comment guifg=white guibg=NONE gui=NONE
hi! String guifg=#2ca198 guibg=NONE gui=NONE
hi! Number guifg=#70c5bf guibg=NONE gui=NONE
hi! Statement guifg=#ffffff guibg=NONE gui=NONE
hi! PreProc guifg=#9DE3C0 guibg=NONE gui=NONE
hi! SpecialComment guifg=#87875f guibg=NONE gui=reverse
hi! Underlined guifg=#af5f5f guibg=NONE gui=NONE

hi! link Constant Statement
hi! link Character Number
hi! link Boolean Number
hi! link Float Number
hi! link Identifier Normal
hi! link Operator Normal
hi! link Type PreProc
hi! link Special Normal
hi! link SpecialChar String
hi! link Todo Comment
hi! link Title Normal

" interface
hi! Cursor                     guifg=#000000       guibg=#dfdfaf      gui=NONE
hi! MoreMsg                    guifg=#dfaf87       guibg=NONE         gui=NONE
hi! Question                   guifg=#875f5f       guibg=NONE         gui=NONE
hi! Search                     guifg=white         guibg=#2ca198    gui=NONE
hi! Pmenu                      guifg=white         guibg=#0a535c       gui=NONE
hi! MatchParen                 guifg=#dfdfaf       guibg=#875f5f      gui=NONE

hi! link Visual Search
hi! link PmenuSel Search
hi! link LineNr Normal
hi! link VertSplit StatusLine

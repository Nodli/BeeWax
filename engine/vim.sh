#!/bin/bash

cd $(dirname $0)
cd source

executable_path="../linux/makerun.sh"
session_path="session.vim"
vimrc_path="${PWD}/../.vimrc"

vim -c ":nnoremap <F5> :wa<CR> :!${executable_path}<CR>"                                                                                                                                                                \
    -c ":nnoremap <F6> :!<CR>"                                                                                                                                                                                          \
    -c ":autocmd VimEnter * if filereadable(\"${session_path}\") | :source ${session_path} | :source ${vimrc_path} | else | :args **/*.h **/*.cpp **/*.inl **/*.txt | :source ${vimrc_path} | :b todo.txt | endif"      \
    -c ":autocmd VimLeave * :mksession! ${session_path}"                                                                                                                                                                \
    -c ":nnoremap <F7> :e %:p:s,.h$,.X123X,:s,.cpp$,.h,:s,.X123X$,.cpp,<CR>"

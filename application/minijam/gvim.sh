#!/bin/bash

pushd $(dirname $0) > /dev/null

session_path=$PWD/session.vim
vimrc_path=~/.vimrc

gvim -c ":autocmd VimEnter * if filereadable(expand(\"$session_path\")) | :source $session_path | :source $vimrc_path | else | :args source/*.* | :set filetype=cpp | :e todo.txt | :source $vimrc_path | endif" \
     -c ":autocmd VimLeave * :mksession! $session_path"

popd

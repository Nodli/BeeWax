@echo off

pushd %~dp0

set session_path=%cd%\session.vim
set gvim_path="C:\Program Files (x86)\Vim\vim82\gvim.exe"

start /MAX "" /D "." %gvim_path%                                                                                                                                                      	        ^
     -c ":autocmd VimEnter * if filereadable(expand(\"%session_path%\")) | :source %session_path% | else | :args source/*.* | :set filetype=cpp | :e todo.txt | | endif"   ^
     -c ":autocmd VimLeave * mksession! %session_path%"

popd

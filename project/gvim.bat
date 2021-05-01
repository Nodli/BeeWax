@echo off

pushd %cd%

set session_path=%cd%\session.vim
set gvim_path="C:\Program Files (x86)\Vim\vim82\gvim.exe"

start /MAX "" /D "." %gvim_path%                                                                                                                                                      	        ^
     -c ":autocmd VimEnter * if filereadable(expand(\"%session_path%\")) | :source %session_path% | else | :args **\*.h **\*.cpp **\*.inl **\*.txt | :set filetype=cpp | :b todo.txt | endif"   ^
     -c ":autocmd VimLeave * mksession! %session_path%"

popd

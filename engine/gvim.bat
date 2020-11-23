@echo off

pushd %cd%\source

set executable_path=%cd%\..\win_x64\make.bat
set session_path=session.vim
set vimrc_path=%cd%\..\.vimrc
set gvim_path="C:\Program Files (x86)\Vim\vim82\gvim.exe"

start /MAX "" /D "." %gvim_path%                                                                                                                                                      	                                                        ^
     -c ":nnoremap <F5> :wa<CR> :!%executable_path%<CR>"                                                                                                                		                                                                ^
     -c ":nnoremap <F6> :!<CR>"                                                                                                                                                                                                                 ^
     -c ":autocmd VimEnter * if filereadable(expand(\"%session_path%\")) | :source %session_path% | :source %vimrc_path% | else | :args **\*.h **\*.cpp **\*.inl **\*.txt | :set filetype=cpp | :source %vimrc_path% | :b todo.txt | endif"     ^
     -c ":autocmd VimLeave * mksession! %session_path%"                                                                                                                                                                                         ^
     -c ":nnoremap <F7> :e %:p:s,.h$,.X123X,:s,.cpp$,.h,:s,.X123X$,.cpp,<CR>"

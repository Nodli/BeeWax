" highlight search
set hlsearch

" disable line breaking
set nowrap

" show line numbers
set number

" always show the status line
set laststatus=2
" show column numbers
" %f : file
" %= : switch to the right side
" %c : column number
" %l : total lines
set statusline=%f%=%c\ \|\ %L

" enable backspace removal of previous text
set backspace=indent,eol,start

" insert mode shortcuts
inoremap jk <ESC>

" updating find path to current path
set path=$PWD/**

" display matches when auto completing file names in the status bar
set wildmenu

" remove trailing white spaces / tabs when saving
" e avoids showing an error message when no trailing white spaces / tabs is found
autocmd BufWritePost * %s/\s\+$//e

" allows buffers to be hidden without saving
set hidden

" copy / cut / paste to system clipboard
if has('win64') || has('win32')
    set clipboard=unnamed
elseif has('unix')
    set clipboard=unnamedplus
endif


" inserts spaces instead of a tabulation when Tab is pressed
set expandtab
" cases <Tab> to be equivalent to 4 spaces
set tabstop=4
" causes <Tab> and <Backspace> to insert and delete the equivalent number of spaces
set softtabstop=4
" causes ==, << and >> to indent using 4 spaces
set shiftwidth=4

" autocompletion with TAB and cycling with Shift-J / Shift-K
function! InsertTabWrapper()
    let col = col('.') - 1
    if !col || getline('.')[col - 1] !~ '\k'
        return "\<TAB>"
    elseif !pumvisible()
        return "\<C-N>"
    else
        return "\<C-E>"
    endif
endfunction
inoremap <expr> <TAB> InsertTabWrapper()
inoremap <expr> <S-J> pumvisible() ? "\<C-N>" : "\<S-J>"
inoremap <expr> <S-K> pumvisible() ? "\<C-P>" : "\<S-K>"

" netrw shortcut
nnoremap <S-TAB> :Explore<CR>
" ignoring *.swp files in netrw
let g:netrw_list_hide= '.*\.swp$'

" grep word under cursor shortcut
nnoremap gr :silent vimgrep /<C-R><C-W>/j  **/*.h **/*.cpp **/*.inl <CR> :copen<CR> :wincmd L<CR>

" activate wrapping in quickfix windows only
autocmd FileType qf setlocal wrap
" jump in new window when in quickfix
function! QuickfixCRWrapper()
    if &buftype ==# 'quickfix'
        return "\<C-W>\<CR>:ccl\<CR>"
    else
        return "\<CR>"
endfunction
nnoremap <expr> <CR> QuickfixCRWrapper()

" NOTE(hugo) & TODO(hugo)
inoremap <F1> NOTE(hugo):
inoremap <F2> TODO(hugo):

" gui settings
if has('gui_running')
    " source the syntax highlighting and colorscheme
    source colorscheme.vim

    " disable the windows menu bar
    set guioptions=

    " override the default font
    if has('win64') || has('win32')
        set guifont=ProggyCleanTT:h12
    elseif has('unix')
        set guifont=ProggyCleanTT\ 12
    endif

    " autosave all files on lost focus (ignores complaints for untitled buffers)
    autocmd FocusLost * wa

    " automatically resize buffer windows to match the gvim window width and height
    autocmd VimResized * wincmd =
endif

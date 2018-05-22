let s:tests_root = fnamemodify(expand('<sfile>'), ':p:h')
let &l:makeprg = 'make -C '.s:tests_root
unlet s:tests_root

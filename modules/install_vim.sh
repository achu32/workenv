#!/bin/bash
# ackack's workenv 

function light_print() {
    echo -e "\033[1;37m$1\033[0m"
}
function red_print() {
    echo -e "\033[1;31m$1\033[0m"
}
function green_print() {
    echo -e "\033[1;32m$1\033[0m"
}
function yellow_print() {
    echo -e "\033[1;33m$1\033[0m"
}


if [ -f $HOME/.vimrc ]; then
    mv -f $HOME/.vimrc $HOME/.vimrc_bak
fi
if [ -d $HOME/.vim ]; then
    rm -rf $HOME/.vim_bak
    mv -f $HOME/.vim $HOME/.vim_bak
fi
cp -rf vim/.vim $HOME
cp -rf vim/.vimrc $HOME
green_print "SYS: install vim successfully"

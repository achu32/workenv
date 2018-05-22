#!/bin/bash
CUR=$PWD
cd modules/
source install_vim.sh "autoinstall"
source install_env.sh "autoinstall"
cd $CUR

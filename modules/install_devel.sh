#!/usr/bin/env bash

YUM_PACKAGES=" python python-setuptools gcc zlib-devel openssl   \
               openssl-devel mysql-devel readline readline-devel \
               readline-static openssl-static sqlite-devel       \
               bzip2-devel bzip2-libs"

DPKG_PACKAGES=" gcc build-essential zlib1g-dev python            \ 
                python-setuptools libssl-dev openssl-dev         \ 
                mysql-dev "

OS="none"
auto=$1

function detect_os() {
    isMac=`uname`
    if [[ -f /etc/redhat-release ]]; then
        OS="CentOS"
    elif [[ -f /etc/lsb-release ]]; then
        grep "Ubuntu" /etc/lsb-release > /dev/null 2>&1
        if [ $? -eq "0" ]; then
            OS="Ubuntu"
        fi
    elif [[ $isMac == "Darwin" ]]; then
        OS="OSX"
    fi
}
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

function check_procedure () {
    if [[ $auto == "autoinstall" ]]; then
        yellow_print "INFO: install $1 packages"
        $2
        return
    fi
    light_print "SYS: Do you want to install $1? [Y/n]"
    read input_key
    if [[ $input_key == "N" ]] || [[ $input_key == "n" ]]; then
        yellow_print "INFO: skip $1 packages" 
    else
        yellow_print "INFO: install $1 packages"
        $2
    fi
}


function install_devel () {
    return_status=999
    case $OS in
        "CentOS")
            if [ $USER = "root" ]; then
                yum install -y $(echo $YUM_PACKAGES)
            else 
                red_print "WARNING:The user has no privileges to install yum packages, sudo yum install"
                sudo yum install -y $(echo $YUM_PACKAGES)
            fi
            return_status=$?
            echo 'CentOS'
            ;;
        "Ubuntu")
            light_print "SYS: sudo apt-get install Ubuntu related python packages" 
            sudo apt-get install $(echo $DPKG_PACKAGES)
            return_status=$?
            echo 'Ubuntu'
            ;;
        "OSX")
            brew list > /dev/null 2>&1
            if [[ $? -eq "127" ]]; then
                #http://brew.sh/
                ruby -e "$(curl -fsSL https://raw.github.com/mxcl/homebrew/go)" 
            fi
            return_status=$?
            ;;
    esac
    if [[ $return_status -ne "0" ]]; then
        red_print "ERROR: devel does not install or install incorrectly"
    else
        green_print "INFO: devel install successfully"
    fi
}

function install_vifm () {
    return_status=999
    case $OS in
        "CentOS")
            if [ $USER = "root" ]; then
                yum install -y ncurses-libs ncurses-devel ncurses-base ncurses autoconf
            else 
                red_print "WARNING:The user has no privileges to install yum packages, sudo yum install"
                sudo yum install -y ncurses-libs ncurses-devel ncurses-base ncurses autoconf
            fi
            current=$PWD
            cd devel/vifm-0.9
            ./configure > /dev/null 2>&1
            autoreconf -ivf > /dev/null 2>&1
            make > /dev/null 2>&1
            return_status=$?
            make install > /dev/null 2>&1
            make clean > /dev/null 2>&1
            make distclean > /dev/null 2>&1
            cd $current
            ;;
        "Ubuntu")
            light_print "SYS: sudo apt-get install Ubuntu related packages" 
            sudo apt-get install vifm
            return_status=$?
            ;;
        "OSX")
            brew list > /dev/null 2>&1
            if [[ $? -eq "127" ]]; then
                ruby -e "$(curl -fsSL https://raw.github.com/mxcl/homebrew/go)" 
            fi
            brew install vifm
            return_status=$?
            ;;
    esac

    if [[ $return_status -ne "0" ]]; then
        red_print "ERROR: vifm does not install or install incorrectly"
    else
        green_print "INFO: vifm install successfully"
    fi
}


# Main Install Procedure
detect_os
check_procedure 'devel-packages' install_devel
check_procedure 'vifm' install_vifm
green_print "SYS: finish installing devel packages."


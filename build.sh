#! /bin/bash

cur_path=`pwd`
cd src
files=`ls`
function action()
{
    opera=$1
    for file in $files; do
        if [[ -d $file ]]; then
            cd $file
            if [[ -f 'SConstruct' ]]; then
                eval $opera
                if [[ $? -ne 0 ]]; then
                    echo "build " $file "failed" >> ${cur_path}/build.log
                fi
            fi
            cd ..
        fi
    done 
}

function build()
{
    action 'scons'
}

function clean()
{
    action 'scons -c'
}

if [[ $1 == '-b' ]]; then
    build
elif [[ $1 == '-c' ]]; then
    clean
else
    echo 'parameter incorrect'
fi

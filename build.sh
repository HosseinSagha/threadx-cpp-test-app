#!/bin/bash
# input paramter:
# empty for release build
# "clean" for cleaning build folder
# "init" for initialising and updating submodules
# "update" for just updating submodules
# "master" for checking out master branch of submodules
# "pull" for pulling submodules

if [[ $1 == "clean" ]]; then
    echo "Cleaning build folder ..."
    rm -rf build
    exit
elif [[ $1 == "init" ]]; then
    echo "Initialising and updating submodules ..."
    git submodule foreach 'git stash --keep-index'
    git submodule update --init --recursive
    exit
elif [[ $1 == "update" ]]; then
    echo "updating submodules ..."
    git submodule foreach 'git stash --keep-index'
    git submodule update --recursive
    exit
elif [[ $1 == "master" ]]; then
    echo "Checking out master for submodules ..."
    git submodule foreach --recursive 'git checkout master'
    exit
elif [[ $1 == "pull" ]]; then
    echo "Pulling submodules ..."
    git submodule foreach --recursive 'git pull'
    exit
elif [[ "$1" == "" ]];then
    build_folder=./_build/Release
else
    echo "Wrong build option! ./build.sh [clean | init | update | master | pull]"
    exit
fi

# build
echo "Generating build system files ..."
cmake -G Ninja -Wno-dev -DCMAKE_VERBOSE_MAKEFILE=ON -B$build_folder
echo "Building project ..."
cmake --build $build_folder

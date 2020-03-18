#!/usr/bin/env bash

cd "`dirname "$0"`"

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PWD/lib64
cd bin && exec ./spookyghost "$@"

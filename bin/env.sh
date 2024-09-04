#!/bin/sh

ulimit -c unlimited
export LD_LIBRARY_PATH=$(pwd)/lib:$LD_LIBRARY_PATH


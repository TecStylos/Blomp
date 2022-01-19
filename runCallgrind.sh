#!/bin/bash

echo "Removing previous callgrind files..."
rm callgrind.out.*

echo "Building application..."
./build.sh RelWithDebInfo

echo "Running callgrind..."
valgrind --tool=callgrind --dump-instr=yes --collect-jumps=yes ./bin/RelWithDebInfo/Blomp enc sea.jpg
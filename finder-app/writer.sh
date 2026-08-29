#!/bin/bash

writefile=$1
writestr=$2

if [ -z "$writefile" ] || [ -z "$writestr" ]; then
    echo "Error: Parameters not specified."
    exit 1
fi

dirpath=$(dirname "$writefile")

mkdir -p "$dirpath"
if [ $? -ne 0 ]; then
    echo "Error: Could not create directory $dirpath"
    exit 1
fi

echo "$writestr" > "$writefile"
if [ $? -ne 0 ]; then
    echo "Error: Could not write to file $writefile"
    exit 1
fi
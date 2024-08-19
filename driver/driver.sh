#!/bin/bash

for file in $(ls *.ko)
do
filename=$(basename "$file" .ko)
if [ -z "`lsmod | grep $filename`" ]; then
insmod "$file"
echo "insmod $file"
else
rmmod "$filename"
echo "rmmod $filename"
fi
done
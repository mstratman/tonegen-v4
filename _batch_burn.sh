#!/bin/bash

while true
do
    echo "Press enter..."
    read foo
    ~/RP2040/_upload_swd.sh ./build/tonegen-v4.elf
    echo -e "\a"
    sleep 1
    echo -e "\a"
    sleep 1
    echo -e "\a"
    sleep 1
    echo -e "\a"
done

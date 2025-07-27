#!/bin/bash

bin=$1

if [ -z "$bin" ]; then
    echo "Usage: $0 <binary_file>"
    exit 1
fi

echo -n "[+] Shellcode: "
objdump -d "$bin" | \
grep '[0-9a-f]:' | \
grep -oP '\s\K[0-9a-f]{2}(?=\s)' | \
tr -d '\n' | \
sed 's/../\\x&/g'
echo

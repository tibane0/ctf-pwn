#!/usr/bin/env bash

while true;
do
	for f in /tmp/tmp.*--gctf-qemu; do
		
		if [ "${f}" == "/tmp/tmp.*--gctf-qemu" ]
		then
			echo "[+] Nothing to clean"
			continue
		fi

		out=`ps aux | grep ${f} | grep -v grep`
		if [ -z "${out}" ]
		then
			echo "[+] Cleaning up ${f} as its dangling"
			rm -rf ${f}
		else
			echo "[+] Not cleaning up ${f} as its being used"
		fi
	done

	sleep 10
done

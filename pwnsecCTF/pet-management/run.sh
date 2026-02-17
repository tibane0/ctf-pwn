#!/bin/sh
set -eu

DATA_DIR="/srv/app"

RAND_NAME="$(tr -dc 'A-Za-z0-9' </dev/urandom | head -c 32)"
FLAG_PATH="$DATA_DIR/$RAND_NAME"

printf "%s" "${FLAG:?FLAG not set}" > "$FLAG_PATH"
chmod 0444 "$FLAG_PATH"
unset FLAG

exec socat TCP-LISTEN:5050,reuseaddr,fork,keepalive EXEC:"/usr/bin/timeout -k 1 70 /srv/app/chall",stderr,setpgid


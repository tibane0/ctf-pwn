#!/usr/bin/env bash

if ! /app/pow; then
    exit 1
fi

echo -e "Press [ENTER] to start the instance"
read -n1

# Generate temporary directory per connection
DIR=$(mktemp -d --suffix=--gctf-qemu)
cd ${DIR}

echo -e "\e[1;34m[+] Generating personal access key...\e[0m"
echo ""

ssh-keygen -t ed25519 -C "player@gctf" -q -N "" -f key
cat key

echo ""
echo -e "\e[1;34m[+] 1. Paste the key to a file 'key'\e[0m"
echo -e "\e[1;34m[+] 2. Fix key permissions with 'chmod 600 key'\e[0m"

# Copy the rootfs and kernel
cp /app/rootfs.qcow2 .
cp /app/Image .
cp /app/flag.txt .

exec 100>/tmp/port.lock || (echo "Could not spawn instance, please contact an administrator!"; exit -1)
flock -x -w 10 100 
CURRENT_PORT=`cat /tmp/port`
expr \( $CURRENT_PORT + 1 \) % \( $PUBPORTEND - $PUBPORTSTART \) > /tmp/port
flock -u 100

PORT=`expr $CURRENT_PORT + $PUBPORTSTART`

qemu-system-aarch64 \
	-machine virt -cpu cortex-a57	                                                         \
	-monitor none                                                                          \
	-kernel ${DIR}/Image                                                                   \
	-drive file=${DIR}/rootfs.qcow2,format=qcow2                                           \
	-drive file=${DIR}/key.pub,format=file                                                 \
	-drive file=${DIR}/flag.txt,format=file                                                \
	-net nic -net user,hostfwd=tcp::${PORT}-:22                                            \
	-nographic                                                                             \
	-pidfile qemupid                                                                       \
	-smp cores=1                                                                           \
	-m 1G																																									 \
	-append "root=/dev/vda console=ttyAMA0 rw rootwait quiet loglevel=0 pti=on slab_nomerge init_on_alloc=1 init_on_free=1 page_alloc.shuffle=1 vsyscall=none printk.devkmsg=off sysctl.kernel.dmesg_restrict=1 sysctl.kernel.unprivileged_userns_clone=0 io_uring.disabled=1 sysctl.kernel.unprivileged_userns_clone=0 sysctl.user.max_user_namespaces=0 sysctl.kernel.unprivileged_bpf_disabled=1 sysctl.kernel.kptr_restrict=2 sysctl.kernel.perf_event_paranoid=3 sysctl.kernel.yama.ptrace_scope=2 sysctl.fs.suid_dumpable=0 lsm=confidentiality,yama,landlock,apparmor,bpf oops=panic page_poison=on panic=-1 panic_on_warn=1" \
  2>&1 >/dev/null &

echo -e "\e[1;34m[+] 3. Wait 10-30 seconds for the system to boot\e[0m"
echo -e "\e[1;34m[+] 4. Clean known_hosts with ssh-keygen -R \"[${DOMAIN}]:${PORT}\"\e[0m"
echo -e "\e[1;34m[+] 5. Connect with 'ssh -p${PORT} -i key user@${DOMAIN}'\e[0m"
echo -e "\e[1;34m[+] 6. Copy exploit files with 'scp -P ${PORT} -i key ./LOCALEXPLOIT user@${DOMAIN}:~/'\e[0m"

echo ""

echo -e "\e[1;34m[+] You have ${TIMEOUT} seconds to solve it. Avoid timeouts by running it locally.\e[0m"

echo ""
echo -e "Press [ENTER] to stop the instance"
read -n1

PID=`cat qemupid`
kill -s 9 ${PID} 2>&1 >/dev/null
rm -rf ${DIR} 2>&1 >/dev/null

echo -e "Instance stopped"

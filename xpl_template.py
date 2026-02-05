#!/usr/bin/env python3
from pwn import *
import sys, argparse, os, sys, shlex

elf = libc = rop = io = gs =  None
args = binary = None
#argv, env = None
REMOTE = []

r, ra, rl, ru, rr, cl = (lambda *a, **k: io.recv(*a, **k),
	lambda *a, **k: io.recvall(*a, **k),                    
	lambda *a, **k: io.recvline(*a, **k),
	lambda *a, **k: io.recvuntil(*a, **k),
	lambda *a, **k: io.recvregex(*a, **k),
	lambda *a, **k: io.clean(*a, **k)
)

s, sa, st, sl, sla, slt, ia = (
	lambda *a, **k: io.send(*a, **k),
	lambda *a, **k: io.sendafter(*a, **k),
	lambda *a, **k: io.sendthen(*a, **k),
	lambda *a, **k: io.sendline(*a, **k),
	lambda *a, **k: io.sendlineafter(*a, **k),
	lambda *a, **k: io.sendlinethen(*a, **k),
	lambda *a, **k: io.interactive(*a, **k)
)
	

class Init:
	def __init__(self):
		pass
		
	def parse_args(self):
		p = argparse.ArgumentParser(description='Exploit skeleton')
		p.add_argument('mode', choices=['local','remote','gdb','debug'], nargs='?', default='local')
		p.add_argument("--binary-args", help="Arguments for the binary")
		p.add_argument("--host", help="Remote host")
		p.add_argument("--port", type=int, help="Remote port")
		return p.parse_args()

	def build_cmd(self, binary):
		"""Return (argv, env) tuple for process() depending on CUSTOM_LIBC/LD"""
		argv = [binary]
		env = {}
		if args.binary_args:
			argv += shlex.split(args.binary_args)

		return argv, env
	
	def start(self):
		if binary == None or binary == "./":
			log.failure('Binary executable to found')
			sys.exit(0)

		global elf, rop, libc
		elf = context.binary = ELF(binary, checksec=False)
		libc = elf.libc
		rop = ROP(elf)
		
		#self.build_cmd(elf.path)
		argv, env = self.build_cmd(elf.path)

		if args.mode in ('gdb','debug'):
			return gdb.debug(argv, env=env, gdbscript=gs, api=True)

		if args.mode == 'remote':
			host = args.host or (REMOTE[0] if REMOTE else None)
			port = args.port or (REMOTE[1] if len(REMOTE) > 1 else None)
			if not (host and port):
				log.error("Remote mode selected but no host/port specified!")
				log.error("Use --host/--port or set REMOTE variable")
				sys.exit(1)
			return remote(host, port)


		return process(argv, env=env)

	def setup(self):
		"""Initialize pwntools context"""
		context(os='linux')
		#context.terminal = ["terminator", "--new-tab", "-e"] # new tab
		context.terminal = ["remotinator", "vsplit", "-x"] # vertical split
		context.log_level = 'info'
		context.timeout = 3


def print_leak(description, addr):
	"""Helper to leak and log addresses"""
	log.info(f"{description} @ {hex(addr)}")
	return addr

def exploit():
	##################################################################### 
	######################## EXPLOIT CODE ###############################
	#####################################################################
	pass
	
if __name__ == "__main__":
	# === per-challenge config ===
	binary = None
	REMOTE = []
	
	gs = """
	continue
	"""
	# ============================
	init = Init()
	init.setup()
	args = init.parse_args()

	io = init.start()
	print(" === "*15)
	exploit()
	print(" === "*15)
	ia()





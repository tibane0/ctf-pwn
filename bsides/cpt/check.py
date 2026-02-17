import base64, hashlib, sys
b64 = sys.stdin.read()
ct = base64.b64decode(b64)
key = "CPT{The.Paint.Will.Thin}"#"CPT{DECLASS_A0_01_PRIMER}"  # replace with last flag of the task
ks = hashlib.sha256(key.encode()).digest()
pt = bytes([c ^ ks[i % len(ks)] for i,c in enumerate(ct)])
print(pt)
print(pt.decode("latin"))
print(pt.decode("utf-8"))
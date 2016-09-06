import sys

in_cert = False
for line in sys.stdin:
    if line.startswith("-----BEGIN"):
        in_cert = True
    if in_cert:
        sys.stdout.write(line)
    if line.startswith("-----END"):
        in_cert = False
            

# Generate test key

    openssl genrsa -des3 -out test.pem 2048

Passphrase must be "fortestingonly" (or 1.cpp has to be modified).

# Export public key

    openssl rsa -in test.pem -pubout -out test.pub.pem

# Convert private key to PKCS#8

    openssl pkcs8 -topk8 -inform PEM -outform PEM -in test.pem -out test.pkcs8.pem

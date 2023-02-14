#!/bin/bash
cd /app || exit
apt update && apt install -y openssl
# Copy cert and hash folder for test case CaCertificateLinux_SSLCerts
mkdir -p /etc/ssl/certs/
cp ca.crt /etc/ssl/certs/
openssl rehash -compat -v /etc/ssl/certs/
# Execute Tests
./TestCaCertificate

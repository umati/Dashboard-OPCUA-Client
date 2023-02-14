#!/bin/bash
rm -r -f certs
mkdir -p certs
rm -r -f certs_server
mkdir -p certs_server
# Generate root CA
openssl req  -nodes -new -x509  -keyout certs/ca.key -out certs/ca.crt -subj "/C=DE/O=UmatiApp/OU=DevOps"
# Generate Server cert
openssl genrsa -out certs_server/server.key 2048
openssl req -new -key certs_server/server.key -out certs_server/server.csr  -subj "/C=DE/O=UmatiApp/OU=DevOps/CN=mqtt"
openssl x509 -req -in certs_server/server.csr -CA certs/ca.crt -CAkey certs/ca.key -CAcreateserial -out certs_server/server.crt -days 365 -sha256

cp certs/ca.crt certs_server

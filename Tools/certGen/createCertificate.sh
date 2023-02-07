#!/bin/bash
usage()
{
  echo "Usage: $0   -e | --environment <ENV> 
                                [ -o | --outdir  <DIR>] "
  exit 2;
}
## Read args
getopt -o e:o: --long environment:,outdir: -- "$@"
if [[ $? -ne 0 ]]; then
    echo 'Error in command line parsing' >&2
    usage
fi
while true; do
    case "$1" in
        -e | --environment)
            ENV=$2;
            shift 2
            ;;
        -o | --outdir)
            OUT=$2;
            shift 2
            ;;
        *) shift; break ;;
    esac
done

if [ -z "${ENV}" ]; then
    echo 'Requires --environment'
    usage
fi

## Set vars
TEMPLATE_OPENSSL_CONF=openssl.cnf.template
if ! test -f "$TEMPLATE_OPENSSL_CONF"; then
    echo "Could not find openssl.cnf.template"
    exit 2
fi
TARGET_DIR=$ENV
if [ -n "${OUT}" ]; then
    TARGET_DIR=$OUT
fi
TARGET_OPENSSL_CONF=${TARGET_DIR}/openssl.cnf
echo "Environment: $ENV"
echo "TEMPLATE_OPENSSL_CONF: $TEMPLATE_OPENSSL_CONF"
echo "TARGET_DIR: $TARGET_DIR"

## Create target folder
mkdir -p "$TARGET_DIR"
cp $TEMPLATE_OPENSSL_CONF "$TARGET_OPENSSL_CONF"


## Load Settings
#default settings

declare -A SETTINGS=(["Hostnames"]="localhost" ["IPs"]="127.0.0.1" ["URI"]="http://dashboard.umati.app/OPCUA_DataClient" ["Keysize"]=2048 ["Days"]=365 ["Subject"]="/C=DE/O=SampleOrganization/CN=UmatiDashboardClient@localhost")
CONF_FILE=${ENV}.conf
if test -f "$CONF_FILE"; then
    echo "Load config file $CONF_FILE"
    while IFS='=' read -e -r name value || [[ -n "$name" ]]
    do
        SETTINGS[$name]="$value"
    done < "$CONF_FILE"
else
    echo "Use default config"
fi
SETTINGS_BACKUP=${TARGET_DIR}/${ENV}.conf.bak
touch "$SETTINGS_BACKUP"
true > "$SETTINGS_BACKUP"
for i in "${!SETTINGS[@]}"
do
  echo "$i=${SETTINGS[$i]}" >> "$SETTINGS_BACKUP"
done


#Generate openssl.cnf
IPS=("${SETTINGS[IPs]}")
IPS_STRING=""
for (( i=0; i<=$((${#IPS[*]} -1 ));i++))
do
   IPS_STRING="${IPS_STRING}IP.$i = ${IPS[$i]}\n"
done

HOSTNAMES=("${SETTINGS[Hostnames]}")
HOSTNAMES_STRING=""
for (( i=0; i<=$((${#HOSTNAMES[*]} -1 ));i++))
do
   HOSTNAMES_STRING="${HOSTNAMES_STRING}DNS.$i = ${HOSTNAMES[$i]}\n"
done

sed -i "s#{{URI}}#${SETTINGS[URI]}#g" "$TARGET_DIR/openssl.cnf"
sed -i "s#{{IP}}#$IPS_STRING#g" "$TARGET_DIR/openssl.cnf"
sed -i "s#{{HOSTNAME}}#$HOSTNAMES_STRING#g" "$TARGET_DIR/openssl.cnf"

outCrt="$TARGET_DIR/${ENV}.crt"
outKey="$TARGET_DIR/${ENV}.key"
outCert="$TARGET_DIR/client_cert.der"
outKeyDer="$TARGET_DIR/client_key.der"

openssl req \
     -config "${TARGET_OPENSSL_CONF}" \
     -new \
     -nodes \
     -x509 -sha256  \
     -newkey "rsa:${SETTINGS[Keysize]}" \
     -keyout "$outKey" -days "${SETTINGS[Days]}" \
     -subj "${SETTINGS[Subject]}" \
     -out "$outCrt"

openssl x509 -in "$outCrt" -outform der -out "$outCert"
openssl rsa -inform PEM -in "$outKey" -outform DER -out "$outKeyDer"

echo "Use the generated ${outCert} and ${outKeyDer} for the client."
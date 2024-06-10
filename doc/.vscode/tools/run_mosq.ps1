$ROOT_DIR=Get-Location
docker run -d --rm -p 31884:1884 -p 31883:1883 --name mosquitto -v "${ROOT_DIR}/.vscode/tools/mosq.conf:/mosquitto/config/mosquitto.conf:ro" eclipse-mosquitto:2

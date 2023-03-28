#!/bin/bash
# Copyright 2021 (c) Moritz Walker, ISW University of Stuttagart (for umati and VDW e.V.)

docker build -t opcuaclient -f Dockerfile.debian . --no-cache
docker container rm -f opcuaclient
docker run -it --rm -v /workspace/Dashboard-OPCUA-Client/configuration.json:/app/configuration.json --name=opcuaclient dashboardopcuaclient

#!/bin/bash
NEXT_WAITTIME=0
WAITTIME_LIMIT_SEC=120
while [[ "$(docker inspect  cacert_test_test_1 --format='{{.State.Status}}')" != "exited" || "$NEXT_WAITTIME" == "$WAITTIME_LIMIT_SEC" ]]
do 
    echo "Waiting for test container to become ready since ${NEXT_WAITTIME}s..."
    sleep 1
    NEXT_WAITTIME=$(($NEXT_WAITTIME+1))
done
docker compose logs
exit "$(docker inspect  cacert_test_test_1 --format='{{.State.ExitCode}}')"

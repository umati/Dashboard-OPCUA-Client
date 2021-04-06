
function buildPahoC (){
    printf "pahoc\n1: $1\n2: $2\n3: $3\n4: $4\n5: $5\n"
    cmakeDir=$2/paho.mqtt.c
    instDir=$3
    buildConf={$4:-Debug}
	cacheFile=$5
    mkdir -p $cmakeDir
    pushd $cmakeDir
    cmake -C $cacheFile -DCMAKE_INSTALL_PREFIX:STRING=$instDir -DPAHO_WITH_SSL:BOOL=1 -DPAHO_BUILD_STATIC:BOOL=0 -DPAHO_ENABLE_TESTING:BOOL=0 $1/paho.mqtt.c
    cmake --build . --target install --config $buildConf -- -j 2
    popd
}
"$@"
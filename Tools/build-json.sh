
function buildJson (){
    cmakeDir=$2/json
    instDir=$3
    buildConf={$4:-Debug}
	cacheFile=$5
    mkdir -p $cmakeDir
    pushd $cmakeDir
    cmake -C $cacheFile -DCMAKE_INSTALL_PREFIX:STRING=$instDir $1/json
    cmake --build . --target install --config $buildConf -- -j 2
    popd
}
"$@"


function buildEasyLoggingpp (){
    cmakeDir=$2/easyloggingpp
    instDir=$3
    buildConf={$4:-Debug}
	  cacheFile=$5
    mkdir -p $cmakeDir
    pushd $cmakeDir
    cmake -C $cacheFile -DCMAKE_INSTALL_PREFIX:STRING=$instDir -Dbuild_static_lib:BOOL=1 $1/easyloggingpp
    cmake --build . --target install --config $buildConf -- -j 2
    popd
}
"$@"

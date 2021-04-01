
function buildGoogleTest (){
    cmakeDir=$2/googletest
    instDir=$3
    buildConf={$4:-Debug}	
	cacheFile=$5
    mkdir -p $cmakeDir
    pushd $cmakeDir
    cmake -C $cacheFile -DCMAKE_INSTALL_PREFIX:STRING=$instDir -DINSTALL_GMOCK:BOOL=1 -DINSTALL_GTEST:BOOL=1 $1/googletest
    cmake --build . --target install --config $buildConf -- -j 2
    popd
}
"$@"

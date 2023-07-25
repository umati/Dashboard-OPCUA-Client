#!/bin/bash
# MIT License
# 
# Copyright (c) 2022 Götz Görisch, VDW - Verein Deutscher Werkzeugmaschinenfabriken e.V.
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

BASEDIR=$(pwd)
LIBCPP_VERSION="$(dpkg -s libstdc++6 | grep Version | awk '{match($0,"[0-9]+.[0-9].[0-9]",a)}END{print a[0]}')" # yamllint disable-line rule:line-length

mkdir -p build
cd build || exit
cmake ../.github/ -DCMAKE_INSTALL_PREFIX:PATH="$BASEDIR/install" \
    -DCMAKE_BUILD_TYPE=Debug -DPAHO_WITH_SSL=1 -DBUILD_DEB_PACKAGE:BOOL=1 -DDEB_PACKAGE_LIBCPP_VERSION="$LIBCPP_VERSION"
cmake --build . -j 1 --parallel 2
cd Dashboard-Client-build || exit
ctest -V -C Debug
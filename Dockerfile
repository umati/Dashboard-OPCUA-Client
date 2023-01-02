FROM alpine:3.17.0 as build-env

RUN apk --no-cache add \
      bash=5.2.15-r0 \
      cmake=3.24.3-r0 \
      gcc=12.2.1_git20220924-r4 \
      g++=12.2.1_git20220924-r4 \
      git=2.38.2-r0 \
      make=4.3-r1  \
      python3=3.10.9-r1 \
      patch=2.7.6-r8 \
      linux-headers=5.19.5-r0 && \
    mkdir /install

ARG BUILD_TYPE=Debug

COPY . /src/DashboardOpcUaClient

WORKDIR /build
RUN cmake /src/DashboardOpcUaClient/.github/ \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -DPAHO_WITH_SSL=1 \
      -DCMAKE_INSTALL_PREFIX:PATH=/install /build &&\
    cmake --build .

FROM alpine:3.17.0 as runtime
RUN apk --no-cache add \
      libstdc++=12.2.1_git20220924-r4 \
      ca-certificates=20220614-r3

COPY --from=build-env /install/bin /app
COPY --from=build-env /install/lib /usr/lib

WORKDIR /app

EXPOSE 4840

ENTRYPOINT ["/app/DashboardOpcUaClient"]

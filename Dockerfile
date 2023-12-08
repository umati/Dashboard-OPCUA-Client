FROM alpine:3.19.0 as build-env

RUN apk --no-cache add \
      bash=5.2.21-r0 \
      cmake=3.27.8-r0 \
      gcc=13.2.1_git20231014-r0 \
      g++=13.2.1_git20231014-r0\
      git=2.43.0-r0 \
      make=4.4.1-r2  \
      python3=3.11.6-r1 \
      patch=2.7.6-r10 \
      linux-headers=6.5-r0 && \
    mkdir /install

ARG BUILD_TYPE=Debug

COPY . /src/DashboardOpcUaClient

WORKDIR /build
RUN cmake /src/DashboardOpcUaClient/.github/ \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -DPAHO_WITH_SSL=1 \
      -DCMAKE_INSTALL_PREFIX:PATH=/install /build &&\
    cmake --build .

FROM alpine:3.19.0 as runtime
RUN apk --no-cache add \
      libstdc++=13.2.1_git20231014-r0 \
      ca-certificates=20230506-r0

COPY --from=build-env /install/bin /app
COPY --from=build-env /install/lib /usr/lib

WORKDIR /app

EXPOSE 4840

ENTRYPOINT ["/app/DashboardOpcUaClient"]

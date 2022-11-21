FROM alpine:3.16.2 as build-env

RUN apk --no-cache add \
      bash=5.1.16-r2 \
      cmake=3.23.1-r0 \
      gcc=11.2.1_git20220219-r2 \
      g++=11.2.1_git20220219-r2 \
      git=2.36.3-r0 \
      make=4.3-r0 \
      python3=3.10.8-r0 \
      patch=2.7.6-r7 && \
    mkdir /install

ARG BUILD_TYPE=Debug

COPY . /src/DashboardOpcUaClient

WORKDIR /build
RUN cmake /src/DashboardOpcUaClient/.github/ \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -DPAHO_WITH_SSL=1 \
      -DCMAKE_INSTALL_PREFIX:PATH=/install /build &&\
    cmake --build .

FROM alpine:3.16.2 as runtime
RUN apk --no-cache add \
      libstdc++=11.2.1_git20220219-r2 \
      ca-certificates=20220614-r0

COPY --from=build-env /install/bin /app
COPY --from=build-env /install/lib /usr/lib

WORKDIR /app

EXPOSE 4840

ENTRYPOINT ["/app/DashboardOpcUaClient"]

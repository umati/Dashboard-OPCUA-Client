FROM alpine:3.21.0 AS build-env

RUN apk --no-cache add \
      bash=5.2.37-r0 \
      cmake=3.31.1-r0 \
      gcc=14.2.0-r4 \
      g++=14.2.0-r4 \
      git=2.47.1-r0 \
      make=4.4.1-r2  \
      python3=3.12.8-r1 \
      patch=2.7.6-r10 \
      linux-headers=6.6-r1 && \
    mkdir /install

ARG BUILD_TYPE=Debug

COPY . /src/DashboardOpcUaClient

WORKDIR /build
RUN cmake /src/DashboardOpcUaClient/.github/ \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -DPAHO_WITH_SSL=1 \
      -DCMAKE_INSTALL_PREFIX:PATH=/install /build &&\
    cmake --build .

FROM alpine:3.21.0 AS runtime
RUN apk --no-cache add \
      libstdc++=14.2.0-r4 \
      ca-certificates=20241121-r1 \
      tzdata=2024b-r1

COPY --from=build-env /install/bin /app
COPY --from=build-env /install/lib /usr/lib

RUN ln -s /usr/share/zoneinfo/Europe/Berlin /etc/localtime

WORKDIR /app

EXPOSE 4840

ENTRYPOINT ["/app/DashboardOpcUaClient"]

FROM alpine:3.20.1 AS build-env

RUN apk --no-cache add \
      bash=5.2.26-r0 \
      cmake=3.29.3-r0 \
      gcc=13.2.1_git20240309-r0 \
      g++=13.2.1_git20240309-r0 \
      git=2.45.2-r0 \
      make=4.4.1-r2  \
      python3=3.12.3-r2 \
      patch=2.7.6-r10 \
      linux-headers=6.6-r0 && \
    mkdir /install

ARG BUILD_TYPE=Debug

COPY . /src/DashboardOpcUaClient

WORKDIR /build
RUN cmake /src/DashboardOpcUaClient/.github/ \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -DPAHO_WITH_SSL=1 \
      -DCMAKE_INSTALL_PREFIX:PATH=/install /build &&\
    cmake --build .

FROM alpine:3.20.1 AS runtime
RUN apk --no-cache add \
      libstdc++=13.2.1_git20240309-r0 \
      ca-certificates=20240705-r0

COPY --from=build-env /install/bin /app
COPY --from=build-env /install/lib /usr/lib

WORKDIR /app

EXPOSE 4840

ENTRYPOINT ["/app/DashboardOpcUaClient"]

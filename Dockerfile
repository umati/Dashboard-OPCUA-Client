FROM alpine:3.19.0 as build-env

RUN apk --no-cache add \
      bash=5.2.15-r5 \
      cmake=3.26.5-r0 \
      gcc=12.2.1_git20220924-r10 \
      g++=12.2.1_git20220924-r10\
      git=2.40.1-r0 \
      make=4.4.1-r1  \
      python3=3.11.6-r0 \
      patch=2.7.6-r10 \
      linux-headers=6.3-r0 && \
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
      libstdc++=12.2.1_git20220924-r10 \
      ca-certificates=20230506-r0

COPY --from=build-env /install/bin /app
COPY --from=build-env /install/lib /usr/lib

WORKDIR /app

EXPOSE 4840

ENTRYPOINT ["/app/DashboardOpcUaClient"]

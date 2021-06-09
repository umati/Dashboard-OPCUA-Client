FROM alpine:3 as build-env

RUN apk --no-cache add \
      bash=~5.1.0 \
      cmake=~3.18.4 \
      gcc=~10.2.1 \
      g++=~10.2.1 \
      git=~2.30.2\
      make=~4.3 \
      python3=~3.8.10-r0 && \
    mkdir /install

ARG BUILD_TYPE=Debug

COPY . /src/DashboardOpcUaClient

WORKDIR /build
RUN cmake /src/DashboardOpcUaClient/.github/ \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -DCMAKE_INSTALL_PREFIX:PATH=/install /build &&\
    cmake --build .

FROM alpine:3 as runtime
RUN apk --no-cache add \
      libstdc++=~10.2.1 \
       python3=~3.8.10-r0 \
       py3-pip=~ 20.3.4-r1 \
       openssl=~1.1.1k-r0
RUN pip install --no-cache-dir netifaces

COPY --from=build-env /install/bin /app
WORKDIR /app

EXPOSE 4840

ENTRYPOINT ["/app/DashboardOpcUaClient"]

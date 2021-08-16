FROM alpine:3 as build-env

RUN apk --no-cache add \
      bash=~5.1.4-r0 \
      cmake=~3.20.3-r0 \
      gcc=~10.3.1_git20210424-r2 \
      g++=~10.3.1_git20210424-r2 \
      git=~2.32.0-r0\
      make=~4.3 \
      python3=~3.9.5-r1 \
      patch=~2.7.6-r7 && \
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
      libstdc++=~10.3.1_git20210424-r2 \
       python3=~3.9.5-r1 \
       py3-pip=~20.3.4-r1 \
       openssl=~1.1.1k-r0
RUN pip install --no-cache-dir jinja2

COPY --from=build-env /install/bin /app
COPY --from=build-env /install/lib /usr/lib

WORKDIR /app

EXPOSE 4840

ENTRYPOINT ["/app/DashboardOpcUaClient"]

FROM ubuntu:24.04 AS build

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        cmake \
        ninja-build \
        pkg-config \
        libdrogon-dev \
        libjsoncpp-dev \
        uuid-dev \
        zlib1g-dev \
        libssl-dev \
        libpq-dev \
        libsqlite3-dev \
        libmariadb-dev \
        libbrotli-dev \
        libhiredis-dev \
        libyaml-cpp-dev \
        libc-ares-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . .

RUN cmake -S /src -B /build -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DDROGULAR_BUILD_TESTS=OFF \
        -DDROGULAR_BUILD_EXAMPLES=ON \
    && cmake --build /build --target system_monitor_pwa --parallel

FROM ubuntu:24.04 AS runtime

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        ca-certificates \
        libdrogon1t64 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=build /build/examples/system_monitor_pwa/system_monitor_pwa ./system_monitor_pwa
COPY --from=build /src/examples/system_monitor_pwa/templates ./examples/system_monitor_pwa/templates
COPY --from=build /src/examples/system_monitor_pwa/public ./examples/system_monitor_pwa/public

EXPOSE 8080

CMD ["./system_monitor_pwa"]
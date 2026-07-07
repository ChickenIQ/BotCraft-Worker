FROM debian:stable-slim AS builder

RUN apt-get update && apt-get install -y --no-install-recommends \
  build-essential \
  ca-certificates \
  netbase \
  cmake \
  git \
  && rm -rf /var/lib/apt/lists/*

WORKDIR /build
COPY . .

RUN cmake -B build \
  -DCMAKE_BUILD_TYPE=MinSizeRel \
  -DBOTCRAFT_CACHED_CREDENTIALS_PATH="/state/auth.json" 
    
RUN cmake --build build --config MinSizeRel -j$(nproc)

FROM debian:stable-slim AS runtime

RUN apt-get update && apt-get install -y --no-install-recommends \
  ca-certificates \
  netbase \
  && rm -rf /var/lib/apt/lists/*

COPY --from=builder /build/build/BotCraft-Worker /app/
COPY --from=builder /build/bin/ /app/

RUN groupadd -g 1000 botcraft-worker \
  && useradd -u 1000 -g botcraft-worker -m -s /usr/sbin/nologin botcraft-worker \
  && mkdir -p /state \
  && chown -R botcraft-worker:botcraft-worker /state

ENV LD_LIBRARY_PATH=/app
USER botcraft-worker
WORKDIR /app

ENTRYPOINT ["/app/BotCraft-Worker"]

FROM debian:stable-slim AS builder

RUN apt update && apt install -y build-essential ca-certificates cmake git 

WORKDIR /build
COPY . .

RUN cmake -B build \
  -DCMAKE_BUILD_TYPE=MinSizeRel \
  -DBOTCRAFT_CACHED_CREDENTIALS_PATH="/state/auth.json" 
    
RUN cmake --build build --config MinSizeRel -j$(nproc)

FROM debian:stable-slim AS runtime

RUN apt update && apt install -y ca-certificates && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /build/build/BotCraft-Worker /app/
COPY --from=builder /build/bin/ /app/

ENV LD_LIBRARY_PATH=/app

ENTRYPOINT ["./BotCraft-Worker"]

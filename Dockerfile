FROM alpine:latest AS BUILD

RUN apk add build-base

WORKDIR /build
COPY http-infinite-zero.cpp /build/

RUN g++ -o http-infinite-zero -DPORT="80" -DBUFFER_SIZE="4*1024*1024" -O3 -march=tigerlake http-infinite-zero.cpp

FROM alpine:latest

RUN apk update && apk add --no-cache libgcc libstdc++ && rm -rf /var/cache/apk/*

WORKDIR /app
COPY --from=BUILD /build/http-infinite-zero /app/

CMD ["/app/http-infinite-zero"]

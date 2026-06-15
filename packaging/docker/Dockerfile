FROM ubuntu:22.04

ARG TARGETARCH
ARG VERSION
ARG REVISION

LABEL org.opencontainers.image.title="Aria2 Next"
LABEL org.opencontainers.image.description="Maintained aria2-compatible download engine."
LABEL org.opencontainers.image.source="https://github.com/AnInsomniacy/aria2-next"
LABEL org.opencontainers.image.url="https://github.com/AnInsomniacy/aria2-next"
LABEL org.opencontainers.image.version="${VERSION}"
LABEL org.opencontainers.image.revision="${REVISION}"
LABEL org.opencontainers.image.licenses="GPL-2.0-only"

RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
      ca-certificates \
      libssl3 \
      libstdc++6 && \
    rm -rf /var/lib/apt/lists/*

COPY "linux-${TARGETARCH}/aria2-next" /usr/local/bin/aria2-next

RUN chmod 755 /usr/local/bin/aria2-next
RUN /usr/local/bin/aria2-next --version >/dev/null

WORKDIR /downloads
EXPOSE 6800
ENTRYPOINT ["aria2-next"]

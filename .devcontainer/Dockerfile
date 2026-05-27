FROM mcr.microsoft.com/devcontainers/javascript-node:18

ENV PLAYWRIGHT_BROWSERS_PATH=/ms-playwright

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
    build-essential \
    file \
    gawk \
    gettext \
    libncurses5-dev \
    libssl-dev \
    make \
    python3 \
    python3-pip \
    rsync \
    socat \
    unzip \
    wget \
    xz-utils \
    && rm -rf /var/lib/apt/lists/*

RUN mkdir -p /ms-playwright \
    && npx --yes playwright@1.54.2 install --with-deps chromium \
    && chown -R node:node /ms-playwright

RUN npm install -g yarn@1

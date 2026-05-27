FROM mcr.microsoft.com/devcontainers/javascript-node:18

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
    unzip \
    wget \
    xz-utils \
    && rm -rf /var/lib/apt/lists/*

RUN npm install -g yarn@1

FROM mcr.microsoft.com/devcontainers/cpp:ubuntu-22.04

RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get install -y \
       build-essential \
       cmake \
       gdb \
       valgrind \
       manpages-dev \
       manpages-posix-dev \
       strace \
       ltrace \
       iputils-ping \
       net-tools \
       tcpdump \
       iproute2 \
       procps \
       nano \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

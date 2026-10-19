# Reproducible Linux build / test / demo environment for Beijing_Fushengji.
#   docker build -t fushengji-dev -f docker/Dockerfile docker
# Everything needed to compile, run the unit tests and run the game headlessly
# (Xvfb + Mesa software OpenGL) to capture screenshots.
FROM ubuntu:24.04
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential cmake ninja-build git ca-certificates pkg-config clang-format \
        libsdl2-dev libgl-dev libcurl4-openssl-dev nlohmann-json3-dev libgtest-dev \
        libgl1-mesa-dri libglx-mesa0 \
        xvfb xdotool x11-utils imagemagick ffmpeg python3 \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /src

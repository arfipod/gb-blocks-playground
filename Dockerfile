FROM ubuntu:24.04

ARG DEBIAN_FRONTEND=noninteractive
ARG GBDK_VERSION=4.5.0
ARG RGBDS_VERSION=v1.0.1
ARG TARGETARCH=amd64
ARG USERNAME=gbdev
ARG USER_UID=1000
ARG USER_GID=1000

SHELL ["/bin/bash", "-o", "pipefail", "-c"]

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        bison \
        build-essential \
        ca-certificates \
        cmake \
        curl \
        default-jre \
        file \
        flex \
        git \
        libgl1 \
        libpng-dev \
        libsdl2-2.0-0 \
        make \
        pkg-config \
        python3 \
        python3-pip \
        python3-venv \
        ripgrep \
        tar \
        unzip \
        wget \
        xdg-utils \
        xz-utils && \
    rm -rf /var/lib/apt/lists/*

RUN mkdir -p /opt/gbdk /opt/rgbds && \
    if [ "${TARGETARCH}" != "amd64" ]; then \
        echo "This Dockerfile currently supports linux/amd64 only because the pinned RGBDS binary release is x86_64."; \
        exit 1; \
    fi && \
    curl -fsSL \
        "https://github.com/gbdk-2020/gbdk-2020/releases/download/${GBDK_VERSION}/gbdk-linux64.tar.gz" \
        -o /tmp/gbdk-linux64.tar.gz && \
    tar -xzf /tmp/gbdk-linux64.tar.gz -C /opt/gbdk --strip-components=1 && \
    curl -fsSL \
        "https://github.com/gbdev/rgbds/releases/download/${RGBDS_VERSION}/rgbds-linux-x86_64.tar.xz" \
        -o /tmp/rgbds-linux-x86_64.tar.xz && \
    tar -xJf /tmp/rgbds-linux-x86_64.tar.xz -C /opt/rgbds && \
    rm -f /tmp/gbdk-linux64.tar.gz /tmp/rgbds-linux-x86_64.tar.xz

RUN python3 -m venv /opt/pyboy-venv && \
    /opt/pyboy-venv/bin/python -m pip install --no-cache-dir --upgrade pip && \
    /opt/pyboy-venv/bin/python -m pip install --no-cache-dir pyboy pillow && \
    ln -s /opt/pyboy-venv/bin/pyboy /usr/local/bin/pyboy

RUN groupadd --gid "${USER_GID}" "${USERNAME}" && \
    useradd --uid "${USER_UID}" --gid "${USER_GID}" --create-home --shell /bin/bash "${USERNAME}"

ENV GBDK_HOME=/opt/gbdk
ENV RGBDS_HOME=/opt/rgbds
ENV PYBOY=/usr/local/bin/pyboy
ENV PYBOY_ENV="SDL_AUDIODRIVER=dummy LIBGL_ALWAYS_SOFTWARE=1 MESA_LOADER_DRIVER_OVERRIDE=llvmpipe"
ENV SDL_AUDIODRIVER=dummy
ENV LIBGL_ALWAYS_SOFTWARE=1
ENV MESA_LOADER_DRIVER_OVERRIDE=llvmpipe
ENV PATH="/opt/gbdk/bin:/opt/rgbds:/opt/pyboy-venv/bin:${PATH}"

WORKDIR /workspace/gb-eyenaut-adventures
USER ${USERNAME}

CMD ["bash"]

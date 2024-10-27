FROM ubuntu:22.04

# adapted from https://openwrt.org/docs/guide-developer/toolchain/install-buildsystem
# and https://openwrt.org/docs/guide-developer/toolchain/use-buildsystem
# and https://openwrt.org/docs/guide-user/additional-software/saving_space

RUN apt-get update && apt-get upgrade -y
RUN apt-get install -y \
    build-essential clang flex bison g++ gawk \
    gcc-multilib g++-multilib gettext git libncurses-dev libssl-dev \
    python3-distutils python3-setuptools rsync swig unzip zlib1g-dev file wget \
    python2 vim

RUN useradd ubuntu
USER ubuntu
WORKDIR /home/ubuntu

RUN git clone https://git.openwrt.org/openwrt/openwrt.git

WORKDIR /home/ubuntu/openwrt

RUN git checkout v19.07.10
RUN ./scripts/feeds update -a
RUN ./scripts/feeds install -a

COPY --chown=ubuntu:ubuntu .config .
RUN make defconfig

RUN make -j$(nproc) download
RUN make -j$(nproc)

FROM ubuntu:latest

# adapted from https://clifford.at/icestorm

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get -y upgrade
RUN apt-get install -y git build-essential clang bison flex libreadline-dev \
                       gawk tcl-dev libffi-dev git mercurial graphviz \
                       xdot pkg-config python3 python-is-python3 libftdi-dev \
                       python3-dev libboost-all-dev cmake libeigen3-dev

RUN git clone https://github.com/YosysHQ/icestorm.git
RUN cd icestorm && make -j$(nproc) && make install

RUN git clone https://github.com/YosysHQ/nextpnr && cd nextpnr
RUN cd nextpnr && cmake -DARCH=ice40 -DCMAKE_INSTALL_PREFIX=/usr/local .
RUN cd nextpnr && make -j$(nproc) && make install

RUN git clone https://github.com/YosysHQ/yosys.git
RUN cd yosys && make -j$(nproc) && make install

WORKDIR /project

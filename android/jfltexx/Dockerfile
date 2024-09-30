FROM ubuntu:24.04

# adapted from https://wiki.lineageos.org/devices/jfltexx/build

RUN apt-get update && apt-get upgrade -y
RUN apt-get install -y \
    wget unzip python-is-python3 \
    bc bison build-essential ccache curl flex g++-multilib gcc-multilib \
    git git-lfs gnupg gperf imagemagick lib32readline-dev lib32z1-dev \
    libelf-dev liblz4-tool libsdl1.2-dev libssl-dev libxml2 libxml2-utils \
    lzop pngcrush rsync schedtool squashfs-tools xsltproc zip zlib1g-dev
RUN wget http://archive.ubuntu.com/ubuntu/pool/universe/n/ncurses/libtinfo5_6.4-2_amd64.deb && \
    dpkg -i libtinfo5_6.4-2_amd64.deb && rm -f libtinfo5_6.4-2_amd64.deb
RUN wget http://archive.ubuntu.com/ubuntu/pool/universe/n/ncurses/libncurses5_6.4-2_amd64.deb && \
    dpkg -i libncurses5_6.4-2_amd64.deb && rm -f libncurses5_6.4-2_amd64.deb

USER ubuntu
WORKDIR /home/ubuntu

RUN wget https://dl.google.com/android/repository/platform-tools-latest-linux.zip && \
    unzip platform-tools-latest-linux.zip && rm platform-tools-latest-linux.zip && \
    echo "export PATH=\$HOME/platform-tools:\$PATH" >> ~/.profile

RUN mkdir ~/bin && wget -O ~/bin/repo https://storage.googleapis.com/git-repo-downloads/repo && \
    chmod a+x ~/bin/repo

RUN git config --global user.email "matthewlamtran@berkeley.edu" && \
    git config --global user.name "Matthew Tran" && \
    git lfs install && \
    git config --global trailer.changeid.key "Change-Id"

RUN echo "export USE_CCACHE=1" >> ~/.profile && \
    echo "export CCACHE_EXEC=/usr/bin/ccache" >> ~/.profile && \
    echo "export CCACHE_DIR=$HOME/android/ccache" >> ~/.profile && \
    echo "export CCACHE_MAXSIZE=25G" >> ~/.profile && \
    echo "export CCACHE_COMPRESS=true" >> ~/.profile

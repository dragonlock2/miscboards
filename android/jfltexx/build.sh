#!/bin/sh

docker run --rm -v ./android:/home/ubuntu/android jfltexx /bin/bash -lc "
    cd ~/android;
    source build/envsetup.sh;
    breakfast jfltexx;
    croot;
    brunch jfltexx;
"

# TODO breakfast needed?

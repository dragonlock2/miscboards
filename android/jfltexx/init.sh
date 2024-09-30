#!/bin/sh

docker build -t jfltexx .

mkdir -p android && cd android
git lfs install
repo init -u https://github.com/LineageOS/android.git -b lineage-18.1 --git-lfs
repo sync
cd ..

# from prior run of ./extract-files.sh
cp -Tr vendor/samsung android/vendor/samsung

docker run --rm -v ./android:/home/ubuntu/android jfltexx /bin/bash -lc "
    cd ~/android;
    source build/envsetup.sh;
    breakfast jfltexx;
"

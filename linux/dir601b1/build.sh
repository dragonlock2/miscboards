#!/bin/sh

docker build -t dir601b1 .

id=$(docker create dir601b1)
docker cp $id:/home/ubuntu/openwrt/bin/targets/ar71xx/tiny/openwrt-ar71xx-tiny-dir-601-b1-squashfs-factory.bin .
docker cp $id:/home/ubuntu/openwrt/bin/targets/ar71xx/tiny/openwrt-ar71xx-tiny-dir-601-b1-squashfs-sysupgrade.bin .
docker rm -v $id

#!/bin/bash

# create network interface
cd /sys/kernel/config/usb_gadget
mkdir g1
cd g1

echo 0x1d6b > idVendor
echo 0x0103 > idProduct
echo 0x0200 > bcdUSB
echo 0x0100 > bcdDevice
echo 0x00 > bDeviceClass
echo 0x00 > bDeviceSubClass
echo 0x00 > bDeviceProtocol
echo 0x40 > bMaxPacketSize0

mkdir strings/0x409
echo "Raspberry Pi" > strings/0x409/manufacturer
echo "piusb" > strings/0x409/product
echo "69420" > strings/0x409/serialnumber

mkdir functions/ncm.usb0

mkdir configs/c.1
mkdir configs/c.1/strings/0x409
echo "piusb config" > configs/c.1/strings/0x409/configuration
echo 250 > configs/c.1/MaxPower
ln -s functions/ncm.usb0 configs/c.1

# works on Pi Zero W, may need to adjust for others
echo "20980000.usb" > UDC

# enable network interface
ip link set usb0 up
ip addr add 169.254.69.42/16 dev usb0
dhclient -nw usb0

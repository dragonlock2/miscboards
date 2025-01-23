# piusb

Instructions to get a USB Ethernet gadget working on a Pi. Bookworm and `g_ether` haven't been working recently. Building `g_ncm` is an option, but it wouldn't survive updates. Instead we can use `configfs`. There's a better way to do this, but I don't have time to figure that out right now.

1. Install Raspberry Pi OS Bookworm. Make sure to enable SSH.
2. Add `dtoverlay=dwc2` and `enable_uart=1` to the end of `config.txt`. Add `modules-load=dwc2,libcomposite` after `rootwait` in `cmdline.txt`.
3. Copy over `setup_usb.service` and `setup_usb.sh` to the boot partition.
4. Connect using UART and run the following. Reboot and enjoy!
```
sudo cp /boot/firmware/setup_usb.service /etc/systemd/system
sudo cp /boot/firmware/setup_usb.sh /opt
sudo systemctl enable setup_usb.service
```

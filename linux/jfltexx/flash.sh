#!/bin/sh

# helper to update from same LineageOS version

adb -d root
wget https://raw.githubusercontent.com/LineageOS/android_packages_apps_Updater/lineage-18.1/push-update.sh
chmod +x push-update.sh
./push-update.sh $(ls -t android/out/target/product/jfltexx/lineage-*-UNOFFICIAL-jfltexx.zip | head -1)
rm push-update.sh
echo "install update from Settings > System > System updates"

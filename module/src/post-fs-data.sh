#!/system/bin/sh

MODDIR=${0%/*}
chmod +x $MODDIR/bin -R
#  must usr &
./bin/zygiskd d63138f231 &


#!/system/bin/sh


MODDIR=${0%/*}
if [ "$ZYGISK_ENABLED" ]; then
  exit 0
fi

cd "$MODDIR"

if [ "$(which magisk)" ]; then
  for file in ../*; do
    if [ -d "$file" ] && [ -d "$file/zygisk" ] && ! [ -f "$file/disable" ]; then
      if [ -f "$file/post-fs-data.sh" ]; then
        cd "$file"
        log -p i -t "zygisk-sh" "Manually trigger post-fs-data.sh for $file"
        sh "$(realpath ./post-fs-data.sh)"
        cd "$MODDIR"
      fi
    fi
  done
fi

cd "$MODDIR" || exit

chmod +x $MODDIR/bin -R


#  must usr &
./bin/zygiskd unix_socket d63138f231 &
#./bin/adi -m -c   $MODDIR/zygisk3.json &
./bin/adi  -m  -p 1 --exec /system/bin/app_process64  --injectSoPath /data/adb/modules/zygiskADI/lib/arm64-v8a/libzygisk.so --injectFunSym entry --injectFunArg d63138f231 --monitorCount 10&


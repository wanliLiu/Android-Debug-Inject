#!/system/bin/sh

MODDIR=${0%/*}
cd "$MODDIR" || exit

chmod +x $MODDIR/bin -R

create_sys_perm() {
  mkdir -p $1
  chmod 555 $1
  chcon u:object_r:system_file:s0 $1
}


export TMP_PATH=/apex/com.android.syzuel

create_sys_perm $TMP_PATH
mount -t tmpfs tmpfs $TMP_PATH

if [ -f $MODDIR/lib/arm64-v8a/libzygisk.so ];then
  create_sys_perm $TMP_PATH/lib64
  cp $MODDIR/lib/arm64-v8a/libzygisk.so $TMP_PATH/lib64/libsyzuel.so
  chcon u:object_r:system_file:s0 $TMP_PATH/lib64/libsyzuel.so
fi


#  must usr &
./bin/zygiskd unix_socket d63138f231 &
./bin/adi  $MODDIR/zygisk3.json &


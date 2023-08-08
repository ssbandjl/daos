
dmg pool create sxb -z 4g
dmg pool list -v
dmg pool query sxb

daos container create sxb --type POSIX sxb
daos cont list sxb --verbose; daos cont query sxb sxb

umount /mnt/sxb
dfuse -m /mnt/sxb --pool sxb --cont sxb

# gdb attach `ps aux|grep dfuse|grep -v grep|awk '{print$2}'`

cd xb
./write

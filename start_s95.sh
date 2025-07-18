# modprobe xpmem
# lsmod|grep xpmem

mkdir -p  /var/run/daos_agent/
pkill daos_agent
# chmod 0400 /root/project/stor/daos/daos_ca/etc/daos/certs/agent.key
daos_agent &

# chmod 0400 /root/project/stor/daos/daos_ca/etc/daos/certs/server.key

mkdir -p  /var/run/daos_server
# mkdir -p /usr/share/spdk/scripts/
# cp /root/project/stor/daos/install/prereq/debug/spdk/share/spdk/scripts/setup.sh /usr/share/spdk/scripts/setup.sh
# umount /mnt/daos/1
daos_server start &

# dmg storage format

# dmg system query -v


# daos_create_pool_cont_mount_dfuse
# dmg pool create sxb -z 4g; dmg pool list --verbose

root@s95:~/project/stor/daos# dmg pool create sxb -z 4g; dmg pool list --verbose
Creating DAOS pool with automatic storage allocation: 4.0 GB total, 6.38% ratio
Pool created with 6.00%,94.00% storage tier ratio
-------------------------------------------------
  UUID                 : 17464d14-b8ab-42c9-adac-b454e4477a0f
  Service Leader       : 1                                   
  Service Ranks        : [0-2]                               
  Storage Ranks        : [0-2]                               
  Total Size           : 4.0 GB                              
  Storage tier 0 (SCM) : 240 MB (80 MB / rank)               
  Storage tier 1 (NVMe): 3.8 GB (1.3 GB / rank)              

Label UUID                                 State SvcReps SCM Size SCM Used SCM Imbalance NVME Size NVME Used NVME Imbalance Disabled UpgradeNeeded? Rebuild State 
----- ----                                 ----- ------- -------- -------- ------------- --------- --------- -------------- -------- -------------- ------------- 
sxb   17464d14-b8ab-42c9-adac-b454e4477a0f Ready [0-2]   240 MB   201 kB   0%            3.8 GB    126 MB    0%             0/3      None           idle   

# # EC, 2+1 EC 对象，具有一个冗余组, 
# # daos container create sxb --oclass=EC_2P1G1 --type POSIX sxb

# #3副本
# #daos container create sxb --oclass=RP_3GX --properties=rf:1 --type POSIX sxb
# daos container create sxb --type POSIX sxb

root@s95:~/project/stor/daos# daos container create sxb --type POSIX sxb
Successfully created container 0d0a1cf6-5538-4a88-9c0d-c5950d873e4b
  Container UUID : 0d0a1cf6-5538-4a88-9c0d-c5950d873e4b
  Container Label: sxb                                 
  Container Type : POSIX  


# daos container query sxb sxb --verbose; daos cont get-prop sxb sxb

# mkdir -p /tmp/sxb; dfuse --mountpoint=/tmp/sxb --pool=sxb --cont=sxb; df -h
#cd /tmp/sxb

fio --name=global --bs=1M --direct=1 --directory=/tmp/sxb --group_reporting=1 --iodepth=16 --ioengine=libaio --rw=rw --size=1G --thread=2 --verify=crc64 --name=test --numjobs=2
root@s95:/tmp/sxb# fio --name=global --bs=1M --direct=1 --directory=/tmp/sxb --group_reporting=1 --iodepth=16 --ioengine=libaio --rw=rw --size=1G --thread=2 --verify=crc64 --name=test --numjobs=2
test: (g=0): rw=rw, bs=(R) 1024KiB-1024KiB, (W) 1024KiB-1024KiB, (T) 1024KiB-1024KiB, ioengine=libaio, iodepth=16
...
fio-3.16
Starting 2 threads
test: Laying out IO file (1 file / 1024MiB)
test: Laying out IO file (1 file / 1024MiB)
Jobs: 2 (f=2): [V(2)][-.-%][r=755MiB/s][r=754 IOPS][eta 00m:00s]                 
test: (groupid=0, jobs=2): err= 0: pid=26881: Fri Jul 18 10:34:10 2025
  read: IOPS=658, BW=659MiB/s (691MB/s)(2048MiB/3108msec)
    slat (usec): min=12, max=1763, avg=28.96, stdev=58.80
    clat (usec): min=2002, max=49545, avg=31266.81, stdev=9691.07
     lat (usec): min=2019, max=50227, avg=31295.92, stdev=9689.19
    clat percentiles (usec):
     |  1.00th=[ 9110],  5.00th=[15008], 10.00th=[17957], 20.00th=[21365],
     | 30.00th=[24249], 40.00th=[28181], 50.00th=[36439], 60.00th=[39060],
     | 70.00th=[39584], 80.00th=[40109], 90.00th=[40109], 95.00th=[40633],
     | 99.00th=[42730], 99.50th=[43779], 99.90th=[47449], 99.95th=[47973],
     | 99.99th=[49546]
   bw (  KiB/s): min=227328, max=669696, per=74.49%, avg=502651.00, stdev=91813.58, samples=8
   iops        : min=  222, max=  654, avg=490.75, stdev=89.65, samples=8
  write: IOPS=633, BW=633MiB/s (664MB/s)(1066MiB/1683msec); 0 zone resets
    slat (usec): min=2543, max=4462, avg=2888.16, stdev=304.33
    clat (usec): min=1818, max=47803, avg=24370.06, stdev=5931.63
     lat (usec): min=4536, max=51799, avg=27258.39, stdev=6023.09
    clat percentiles (usec):
     |  1.00th=[ 9372],  5.00th=[15139], 10.00th=[17433], 20.00th=[19792],
     | 30.00th=[21365], 40.00th=[23200], 50.00th=[24249], 60.00th=[25560],
     | 70.00th=[27132], 80.00th=[29230], 90.00th=[31851], 95.00th=[33817],
     | 99.00th=[39584], 99.50th=[42730], 99.90th=[43779], 99.95th=[47973],
     | 99.99th=[47973]
   bw (  KiB/s): min=221184, max=683369, per=84.12%, avg=545626.25, stdev=103138.34, samples=8
   iops        : min=  216, max=  667, avg=532.75, stdev=100.69, samples=8
  lat (msec)   : 2=0.03%, 4=0.22%, 10=1.32%, 20=15.38%, 50=83.04%
  cpu          : usr=98.27%, sys=1.41%, ctx=655, majf=0, minf=8347
  IO depths    : 1=0.1%, 2=0.3%, 4=0.5%, 8=1.0%, 16=98.1%, 32=0.0%, >=64=0.0%
     submit    : 0=0.0%, 4=100.0%, 8=0.0%, 16=0.0%, 32=0.0%, 64=0.0%, >=64=0.0%
     complete  : 0=0.0%, 4=99.9%, 8=0.0%, 16=0.1%, 32=0.0%, 64=0.0%, >=64=0.0%
     issued rwts: total=2048,1066,0,0 short=0,0,0,0 dropped=0,0,0,0
     latency   : target=0, window=0, percentile=100.00%, depth=16

Run status group 0 (all jobs):
   READ: bw=659MiB/s (691MB/s), 659MiB/s-659MiB/s (691MB/s-691MB/s), io=2048MiB (2147MB), run=3108-3108msec
  WRITE: bw=633MiB/s (664MB/s), 633MiB/s-633MiB/s (664MB/s-664MB/s), io=1066MiB (1118MB), run=1683-1683msec




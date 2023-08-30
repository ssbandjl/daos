gdb attach `ps aux|grep '/opt/daos/bin/daos_engine' |grep -v grep|awk '{print$2}'`
b dss_engine_metrics_init
c


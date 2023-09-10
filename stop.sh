echo -e "stop"

# source /root/.bashrc
# run_all "pkill daos_agent;pkill daos_server"

source /root/.bashrc
umount /mnt/daos
df -h|grep '/mnt/daos'

pkill daos_agent 
pkill daos_server

rm -f /tmp/daos*.log
rm -f /tmp/.daos_engine.0.log.swp
rm -f /tmp/daos_engine*

#export FI_LOG_LEVEL=debug
#export FI_LOG_LEVEL=warn

#export HG_LOG_LEVEL=debug
#export HG_LOG_LEVEL=warn

# echo -e "dmg storage format"

# count=0
# while true;do
#     joined_num=`dmg sys query -v|grep Joined|wc -l`
#     if [[ $joined_num -eq 3 ]];then
#         break
#     fi
#     echo -e "wait all rank join, $count times"
#     count=$((count+1))
#     sleep 1
# done

# centos7
# cp -r /home/xb/project/stor/daos/main/daos/daosCA/certs /etc/daos/

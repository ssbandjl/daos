echo -e "start"

# source /root/.bashrc
# run_all "pkill daos_agent;pkill daos_server"

source /root/.bashrc
pkill daos_agent 
pkill daos_server
rm -f /tmp/daos*.log
rm -f /tmp/.daos_engine.0.log.swp
rm -f /tmp/daos_engine*
umount /mnt/daos

#export FI_LOG_LEVEL=debug
#export FI_LOG_LEVEL=warn

#export HG_LOG_LEVEL=debug
#export HG_LOG_LEVEL=warn

daos_agent &
# echo -e "dmg storage format"
daos_server start &

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

sleep 5
dmg storage format


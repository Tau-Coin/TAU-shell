#!/bin/bash
DATE=$(date +%Y%m%d_%H)

NUM=1
LOG="./data/log_"

for ((i=0;i<${NUM};i++))
do
    LOG_FILE=${LOG}${i}
    tar -zcvf ${LOG_FILE}_${DATE}.tar.gz ${LOG_FILE}
done

RMDATE=$(date "+%Y%m%d_%H" -d "-1 days")

TAR_BAK="./log_*_$RMDATE.tar.gz"

rm -rf $TAR_BAK

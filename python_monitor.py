#!/usr/bin/env python3
import os, sys, time
from time import strftime

count = 0 
while True:
    flag = 0 
    try:
        ret = os.popen('ps aux|grep "./config/config"').readlines()
        #print("tau-shell worker num: ", len(ret))
        for r in ret:
            str_cmd = r.split(" ")
            for key_word in str_cmd:
                if("./build/main"==key_word):
                    flag = 1 
        print(flag)
        if(flag==0):
            cmd = "nohup ./run_shell.sh &"
            count += 1 
            os.system(cmd)
    except Exception:
        print(strftime("%Y-%m-%r %H:%M"))

    time.sleep(5.0)

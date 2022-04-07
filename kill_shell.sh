ps -aux|grep main|awk '{print $2}'|xargs kill -9

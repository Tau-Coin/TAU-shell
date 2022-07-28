###  compile
```
   0. libTAU build，详见libTAU/doc/building.md
   
	1.  环境变量设置(和build libTAU一致)
    	 $ENV{OPENSSL_ROOT}/include 
    	 $ENV{BOOST_ROOT} 
    	 $ENV{SQLITE_ROOT}/include
   
     2. mkdir build && cd build && cmake ../ && make
```

### Run
#### run.sh 
 	两个配置，一个配置线上config.txt，一个配置测试网络config_dev.txt
 config参数解析
 ```
           f49126ba43138eedeb6b51996e8281e1     //device_id
			null    //seed，给null，tau-shell会随机生成
            tau://83024767468B8BF8DB868F336596C63561265D553833E5C0BF3E4767659B826B@13.229.53.249:6882 // bs节点
            ./pid.txt
            ./error.txt
            ./debug.txt
            6882     //libTAU监听端口
            8080     //rpc端口，后续和rpc.sh cmd端口对应
            /data/TAU_SHELL/TAU_TEST //tau-shell数据存储目录
            .libTAU_test    //libTAU目录
```
#### rpc cmd使用
./rpc.sh
```
#curl -H "Content-Type: application/json"   --user tau-shell:tester    -X POST  --data '{"method":"get-block-by-hash", "arguments":{"chain_id": "15701c56ad4a8dbd54657374436861696e", "block_hash": "15701c56ad4a8dbd54657374436861696e"}}' http://localhost:8080/rpc ;


--user tau-shell:tester  //rpc用户密码，可以在main函数中修改

 "arguments":{"chain_id": "15701c56ad4a8dbd54657374436861696e", "block_hash": "15701c56ad4a8dbd54657374436861696e"}}' //参数传递
 
 http://localhost:8080/rpc ;   //8080是config中的rpc端口
```

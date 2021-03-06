#curl -H "Content-Type: application/json" --user tau-shell:tester -X GET  --data '{"method":"session-stats"}' http://localhost:8080/rpc

#curl -H "Content-Type: application/json" -X POST  --data '{"method":"set-loop-time-interval", "arguments":{"time-interval": 5000}}' http://localhost:8080/rpc


#curl -H "Content-Type: application/json" -X POST  --data '{"method":"update-friend-info", "arguments":{"friend": "63ec42130442c91e23d56dc73708e06eb164883ab74c9813764c3fd0e2042dc4", "info": "name-tester"}}' http://localhost:8080/rpc ;

#curl -H "Content-Type: application/json" -X POST  --data '{"method":"get-friend-info", "arguments":{"friend": "63ec42130442c91e23d56dc73708e06eb164883ab74c9813764c3fd0e2042dc4"}}' http://localhost:8080/rpc ;

#curl -H "Content-Type: application/json" --user tau-shell:tester -X POST  --data '{"method":"add-new-friend", "arguments":{"friend": "809df518ee450ded0a659aeb4bc5bec636e2cff012fc88d343b7419af974bb81"}}' http://localhost:8080/rpc ;

#curl -H "Content-Type: application/json" --user tau-shell:tester -X POST  --data '{"method":"add-new-message", "arguments":{"sender": "3e87c35d2079858d88dcb113edadaf1b339fcd4f74c539faa9a9bd59e787f124", "receiver": "809df518ee450ded0a659aeb4bc5bec636e2cff012fc88d343b7419af974bb81", "payload": "TAU, Hello"}}' http://localhost:8080/rpc ;

#curl -H "Content-Type: application/json" --user tau-shell:tester -X POST  --data '{"method":"send-data", "arguments":{"receiver": "809df518ee450ded0a659aeb4bc5bec636e2cff012fc88d343b7419af974bb81", "payload": "TAU, Hello", "alpha": 1, "beta": 3, "invoke_limit": 3}}' http://localhost:8080/rpc ;

#curl -H "Content-Type: application/json" --user tau-shell:tester -X POST  --data '{"method":"create-chain-id", "arguments":{"community_name": "TestChain"}}' http://localhost:8080/rpc ;

#curl -H "Content-Type: application/json" --user tau-shell:tester -X POST  --data '{"method":"follow-chain", "arguments":{"chain_id": "15701c56ad4a8dbd2f", "peers":[{"peer_key": "63ec42130442c91e23d56dc73708e06eb164883ab74c9813764c3fd0e2042dc4"}, {"peer_key": "95cd9f12598163a604c01f746bb6f80235c0a1938d70d50c72b7eef3fc158e0c"}]}}' http://localhost:8080/rpc ;

#curl -H "Content-Type: application/json" --user tau-shell:tester -X POST  --data '{"method":"follow-chain-mobile", "arguments":{"chain_id": "00b9611816a07e54test", "peers":[{"peer_key": "6f9c37105222b4554f67e9897fb9375d66d362ed7f1480cef101f717deb1303f"}]}}' http://localhost:9090/rpc ;

#curl -H "Content-Type: application/json" --user tau-shell:tester -X POST  --data '{"method":"unfollow-chain-mobile", "arguments":{"chain_id": "516ace9c42281b09t2"}}' http://localhost:8080/rpc ;

#curl -H "Content-Type: application/json" --user tau-shell:tester -X POST  --data '{"method":"get-chain-state", "arguments":{"chain_id": "edcc7b81962237c3qwe"}}' http://localhost:8080/rpc ;

#curl -H "Content-Type: application/json" -X POST  --data '{"method":"unfollow-chain", "arguments":{"chain_id": "15701c56ad4a8dbd54657374436861696e"}}' http://localhost:8080/rpc ;

#curl -H "Content-Type: application/json" --user tau-shell:tester -X POST  --data '{"method":"get-account-info", "arguments":{"chain_id": "00b9611816a07e54test", "pubkey": "6f9c37105222b4554f67e9897fb9375d66d362ed7f1480cef101f717deb1303f"}}' http://localhost:9090/rpc ;

#curl -H "Content-Type: application/json" -X POST  --data '{"method":"get-median-tx-fee", "arguments":{"chain_id": "15701c56ad4a8dbd54657374436861696e"}}' http://localhost:8080/rpc ;

#curl -H "Content-Type: application/json" -X POST  --data '{"method":"get-top-tip-block", "arguments":{"chain_id": "15701c56ad4a8dbd54657374436861696e", "number": 1}}' http://localhost:8080/rpc ;

#curl -H "Content-Type: application/json" -X POST  --data '{"method":"get-block-by-number", "arguments":{"chain_id": "15701c56ad4a8dbd54657374436861696e", "number": 1}}' http://localhost:8080/rpc ;

#curl -H "Content-Type: application/json" -X POST  --data '{"method":"get-block-by-hash", "arguments":{"chain_id": "15701c56ad4a8dbd54657374436861696e", "block_hash": "15701c56ad4a8dbd54657374436861696e"}}' http://localhost:8080/rpc ;

#curl -H "Content-Type: application/json" --user tau-shell:tester -X POST  --data '{"method":"submit-transaction", "arguments":{"chain_id": "00b9611816a07e54test", "sender": "3e87c35d2079858d88dcb113edadaf1b339fcd4f74c539faa9a9bd59e787f124", "receiver": "809df518ee450ded0a659aeb4bc5bec636e2cff012fc88d343b7419af974bb81", "amount": 1000, "fee": 10, "payload": "sumbit tx test"}}' http://localhost:9191/rpc ;

#curl -H "Content-Type: application/json" --user tau-shell:tester -X POST  --data '{"method":"submit-transaction", "arguments":{"chain_id": "00b9611816a07e54test", "sender": "3e87c35d2079858d88dcb113edadaf1b339fcd4f74c539faa9a9bd59e787f124", "receiver": "809df518ee450ded0a659aeb4bc5bec636e2cff012fc88d343b7419af974bb81", "amount": 10000000, "fee": 3000, "payload": "sumbit tx test"}}' http://localhost:9090/rpc ;

#curl -H "Content-Type: application/json" --user tau-shell:tester -X POST  --data '{"method":"submit-transaction", "arguments":{"chain_id": "00b9611816a07e54test", "sender": "3e87c35d2079858d88dcb113edadaf1b339fcd4f74c539faa9a9bd59e787f124", "receiver": "df69fc8c934e0f2c44172ab5d300afc9c55dea5d6f94e08b1e7815d4ed0b6cf1", "amount": 1000000000, "fee": 2000, "payload": "sumbit tx test"}}' http://localhost:9090/rpc ;

curl -H "Content-Type: application/json" --user tau-shell:tester -X POST  --data '{"method":"submit-note-transaction", "arguments":{"chain_id": "00b9611816a07e54test", "sender": "63ec42130442c91e23d56dc73708e06eb164883ab74c9813764c3fd0e2042dc4", "fee": 0, "payload": "sumbit note tx test"}}' http://localhost:9090/rpc ;

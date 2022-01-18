curl -H "Content-Type: application/json" --user tau-shell:tester -X GET  --data '{"method":"session-stats"}' http://localhost:8080/rpc

#curl -H "Content-Type: application/json" -X POST  --data '{"method":"set-loop-time-interval", "arguments":{"time-interval": 5000}}' http://localhost:8080/rpc

#curl -H "Content-Type: application/json" -X POST  --data '{"method":"add-new-friend", "arguments":{"friend": "63ec42130442c91e23d56dc73708e06eb164883ab74c9813764c3fd0e2042dc4"}}' http://localhost:8080/rpc ;

#curl -H "Content-Type: application/json" -X POST  --data '{"method":"update-friend-info", "arguments":{"friend": "63ec42130442c91e23d56dc73708e06eb164883ab74c9813764c3fd0e2042dc4", "info": "name-tester"}}' http://localhost:8080/rpc ;

#curl -H "Content-Type: application/json" -X POST  --data '{"method":"get-friend-info", "arguments":{"friend": "63ec42130442c91e23d56dc73708e06eb164883ab74c9813764c3fd0e2042dc4"}}' http://localhost:8080/rpc ;

#curl -H "Content-Type: application/json" -X POST  --data '{"method":"add-new-message", "arguments":{"sender": "63ec42130442c91e23d56dc73708e06eb164883ab74c9813764c3fd0e2042dc4", "receiver": "63ec42130442c91e23d56dc73708e06eb164883ab74c9813764c3fd0e2042dc4", "payload": "TAU, Hello"}}' http://localhost:8080/rpc ;


#curl -H "Content-Type: application/json" -X POST  --data '{"method":"create-chain-id", "arguments":{"community_name": "TestChain"}}' http://localhost:8080/rpc ;

#curl -H "Content-Type: application/json" -X POST  --data '{"method":"follow-chain", "arguments":{"chain_id": "15701c56ad4a8dbd54657374436861696e", "peers":[{"peer_key": "63ec42130442c91e23d56dc73708e06eb164883ab74c9813764c3fd0e2042dc4"}, {"peer_key": "95cd9f12598163a604c01f746bb6f80235c0a1938d70d50c72b7eef3fc158e0c"}]}}' http://localhost:8080/rpc ;

#curl -H "Content-Type: application/json" -X POST  --data '{"method":"unfollow-chain", "arguments":{"chain_id": "15701c56ad4a8dbd54657374436861696e"}}' http://localhost:8080/rpc ;

#curl -H "Content-Type: application/json" -X POST  --data '{"method":"get-account-info", "arguments":{"chain_id": "15701c56ad4a8dbd54657374436861696e", "pubkey": "63ec42130442c91e23d56dc73708e06eb164883ab74c9813764c3fd0e2042dc4"}}' http://localhost:8080/rpc ;

#curl -H "Content-Type: application/json" -X POST  --data '{"method":"get-median-tx-fee", "arguments":{"chain_id": "15701c56ad4a8dbd54657374436861696e"}}' http://localhost:8080/rpc ;

#curl -H "Content-Type: application/json" -X POST  --data '{"method":"get-top-tip-block", "arguments":{"chain_id": "15701c56ad4a8dbd54657374436861696e", "number": 1}}' http://localhost:8080/rpc ;

#curl -H "Content-Type: application/json" -X POST  --data '{"method":"get-block-by-number", "arguments":{"chain_id": "15701c56ad4a8dbd54657374436861696e", "number": 1}}' http://localhost:8080/rpc ;

#curl -H "Content-Type: application/json" -X POST  --data '{"method":"get-block-by-hash", "arguments":{"chain_id": "15701c56ad4a8dbd54657374436861696e", "block_hash": "15701c56ad4a8dbd54657374436861696e"}}' http://localhost:8080/rpc ;

#curl -H "Content-Type: application/json" -X POST  --data '{"method":"submit-transaction", "arguments":{"chain_id": "15701c56ad4a8dbd54657374436861696e", "sender": "15701c56ad4a8dbd54657374436861696e", "receiver": "63ec42130442c91e23d56dc73708e06eb164883ab74c9813764c3fd0e2042dc4", "amount": 1000, "fee": 100, "payload": "sumbit tx test"}}' http://localhost:8080/rpc ;

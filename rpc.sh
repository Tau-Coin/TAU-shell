#curl -H "Content-Type: application/json" -X GET  --data '{"method":"session-stats"}' http://localhost:8080/rpc

#curl -H "Content-Type: application/json" -X POST  --data '{"method":"set-loop-time-interval", "arguments":{"time-interval": 5000}}' http://localhost:8080/rpc

#curl -H "Content-Type: application/json" -X POST  --data '{"method":"add-new-friend", "arguments":{"friend": "63ec42130442c91e23d56dc73708e06eb164883ab74c9813764c3fd0e2042dc4"}}' http://localhost:8080/rpc ;

#curl -H "Content-Type: application/json" -X POST  --data '{"method":"update-friend-info", "arguments":{"friend": "63ec42130442c91e23d56dc73708e06eb164883ab74c9813764c3fd0e2042dc4", "info": "name-tester"}}' http://localhost:8080/rpc ;

#curl -H "Content-Type: application/json" -X POST  --data '{"method":"get-friend-info", "arguments":{"friend": "63ec42130442c91e23d56dc73708e06eb164883ab74c9813764c3fd0e2042dc4"}}' http://localhost:8080/rpc ;

curl -H "Content-Type: application/json" -X POST  --data '{"method":"add-new-message", "arguments":{"sender": "63ec42130442c91e23d56dc73708e06eb164883ab74c9813764c3fd0e2042dc4", "receiver": "63ec42130442c91e23d56dc73708e06eb164883ab74c9813764c3fd0e2042dc4", "payload": "TAU, Hello"}}' http://localhost:8080/rpc ;

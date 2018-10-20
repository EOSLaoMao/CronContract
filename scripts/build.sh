#!/bin/bash

docker ps | grep croncontract-eos-dev
if [ $? -ne 0 ]; then
    echo "Run eos dev env "
    docker run --name croncontract-eos-dev -dit --rm -v  `(pwd)`:/croncontract eoslaomao/eos-dev:1.2.3
fi

docker exec croncontract-eos-dev eosiocpp -g /croncontract/croncontract.abi /croncontract/croncontract.cpp
docker exec croncontract-eos-dev eosiocpp -o /croncontract/croncontract.wast /croncontract/croncontract.cpp 
##docker exec eos-dev cleos -u http://$HOST:8888 --wallet-url http://$HOST:8900 set contract croncontract ../croncontract -p croncontract@active
docker cp ../croncontract nodeosd:/

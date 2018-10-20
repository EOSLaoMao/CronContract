#!/bin/bash

ACCOUNT=$1
PKEY=$2
CRON=$3
API=${4:-http://localhost:8888}

cleos set account permission $ACCOUNT cronperm '{"threshold": 1,"keys": [{"key": "'$PKEY'","weight": 1}],"accounts": [{"permission":{"actor":"croncontract","permission":"eosio.code"},"weight":1}]}'  "active" -p $ACCOUNT@active
cleos set action permission $ACCOUNT $ACCOUNT $CRON cronperm -p $ACCOUNT@active

#!/bin/bash

echo "Starting clients..."

for i in {1..10}
do
    ( ./client.py & )
done



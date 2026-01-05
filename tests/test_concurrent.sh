#!/bin/bash
for i in {1..5}; do
  curl -x localhost:8888 http://httpbin.org/ip &
done
wait

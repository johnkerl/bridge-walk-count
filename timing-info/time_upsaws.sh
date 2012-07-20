#!/bin/bash

for n in `jot 1 30`; do
	time count_walks UPSAW $n
done

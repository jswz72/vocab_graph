#!/usr/bin/env bash

BASE="$1"
BEGIN="${BASE}_beg_pos.bin"
CSR="${BASE}_csr.bin"
WEIGHT="${BASE}_weight.bin"
echo $BEGIN


./graph_loader.bin $BEGIN $CSR $WEIGHT $2


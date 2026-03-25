#!/bin/sh

echo "APPLYING PATCHES"
git apply ./Drivers/Embedded-Base/patches/03072026_threadx_tx_queue_max_inc.patch
git apply ./Drivers/Embedded-Base/patches/0322026_nx_stm32_query_status.patch 

#!/bin/bash
#
# elfc.bash <image>
#
option=""
outdir="../binaries"
outext="elf"
shares="oz_kernel.elf"
echo oz_util_ldelf32 $option $outdir/$1.$outext ../objects/$1.r $shares
ln -s -f ../binaries/oz_kernel.elf oz_kernel.elf
../linux/oz_util_ldelf32 $option $outdir/$1.$outext ../objects/$1.r $shares > ../objects/$1.map
rm -f oz_kernel.elf

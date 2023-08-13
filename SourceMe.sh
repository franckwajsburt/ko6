# ------------------------------------------------------------------------------
#    _     ___    __
#   | |__ /'v'\  / /    \date        2023-08-12
#   | / /(     )/ _ \   \copyright   2021-2023 Sorbonne University
#   |_\_\ \___/ \___/                https://opensource.org/licenses/MIT
#
# ------------------------------------------------------------------------------

# Main directory and SOC, you can change them
# --------------------------------------------
export kO6=$HOME/ko6
export SOCLIB=/users/outil/soc/soclib

export CPU=mips
export SOC=almo1-$CPU

export CPU=riscv
export SOC=qemu-virt-$CPU

echo "  _     ___    __   "
echo " | |__ /'v'\  / /   SOC = $SOC"
echo " | / /(     )/ _ \  CPU = $CPU"
echo " |_\_\ \___/ \___/  edit $kO6/SourceMe.sh to change default SOC"
echo ""

# Do not change the line after, unless you know what you are doing
# ----------------------------------------------------------------
export GCC=7.1.0
export CCTOOLS=$kO6/bin/gcc.$GCC
export PATH=$PATH:$kO6/bin:$CCTOOLS/bin:$SOCLIB/utils/bin
export LD_LIBRARY_PATH=$CCTOOLS/lib64:$LD_LIBRARY_PATH
# shopt -s globstar   # allow the ** symbol which replace any pathname

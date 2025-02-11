# ------------------------------------------------------------------------------
#    _     ___    __
#   | |__ /'v'\  / /    \date        2023-08-14
#   | / /(     )/ _ \   \copyright   2021-2023 Sorbonne University
#   |_\_\ \___/ \___/                https://opensource.org/licenses/MIT
#
# ------------------------------------------------------------------------------

# Main directory and SOC, you can change them
# --------------------------------------------
export ko6=$HOME/ko6
export SOCLIB=/users/outil/soc/soclib
export ko6VER=`git show -s --format=%ci`

# uncomment the good CPU and SOC
# ------------------------------
export CPU=riscv
export CPU=mips
export SOC=qemu-virt-$CPU
export SOC=almo1-$CPU

echo "  _     ___    __   "
echo " | |__ /'v'\  / /   CPU: $CPU / SOC: $SOC"
echo " | / /(     )/ _ \  edit $ko6/SourceMe.sh to change default CPU & SOC"
echo " |_\_\ \___/ \___/  GIT: $ko6VER"
echo ""

# Do not change the line after, unless you know what you are doing
# ----------------------------------------------------------------
export GCC=7.1.0
export CCTOOLS=$ko6/bin/gcc.$GCC
export PATH=$ko6/bin:$CCTOOLS/bin:$SOCLIB/utils/bin:$PATH
export LD_LIBRARY_PATH=/usr/lib64:$CCTOOLS/lib64:$LD_LIBRARY_PATH
# shopt -s globstar   # allow the ** symbol which replace any pathname

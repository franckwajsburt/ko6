# ------------------------------------------------------------------------------
#    _     ___    __
#   | |__ /'v'\  / /    \date 2025-05-01
#   | / /(     )/ _ \   Copyright (c) 2021 Sorbonne University
#   |_\_\ x___x \___/   SPDX-License-Identifier: MIT
#
# ------------------------------------------------------------------------------

# Main directory and SOC, you can change them
# --------------------------------------------
export ko6=$HOME/ko6
export SOCLIB=/users/outil/soc/soclib
export SOCLIB=$HOME/soc/soclib
export KO6VER="`git show -s --format=%ci`"

# uncomment the good CPU and SOC
# ------------------------------
export CPU=riscv
export CPU=mips
export SOC=qemu-virt-$CPU
export SOC=almo1-$CPU
export MKSOC=src/soft/hal/soc/$SOC/Makefile

echo "  _     ___    __   "
echo " | |__ /'v'\  / /   SoC: $SOC"
echo " | / /(     )/ _ \  edit $ko6/SourceMe.sh to change default CPU & SOC"
echo " |_\_\ x___x \___/  GIT: $KO6VER"
echo ""

# Do not change the line after, unless you know what you are doing
# ----------------------------------------------------------------
export GCC=7.1.0
export CCTOOLS=$ko6/bin/gcc.$GCC

case ":$PATH:" in
  *":$ko6/bin:"*) ;;
  *) _OLD_PATH="$PATH"
    _OLD_LD_LIBRARY_PATH="$LD_LIBRARY_PATH"
     export PATH=$ko6/bin:$CCTOOLS/bin:$SOCLIB/utils/bin:$PATH;
     export LD_LIBRARY_PATH=/usr/lib64:$CCTOOLS/lib64:$LD_LIBRARY_PATH;;
esac

shopt -s globstar   # allow the ** symbol which replace any pathname
if [[ ! "$PS1" == *ko6* ]]; then
    _OLD_PS1="${PS1:-}"
    PS1='(ko6) '"${PS1:-}"
export PS1
fi

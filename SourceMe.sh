# ------------------------------------------------------------------------------
#    _     ___    __
#   | |__ /'v'\  / /    \date        2022-07-13
#   | / /(     )/ _ \   \copyright   2021 Sorbonne University
#   |_\_\ \___/ \___/                https://opensource.org/licenses/MIT
#
# ------------------------------------------------------------------------------

# Main directory, you can change it
# ---------------------------------
export kO6=$HOME/Documents/ko6
export SOCLIB=/users/outil/soc/soclib
export SOC=almo1-mips
export SOC=qemu-virt-riscv

echo $kO6

# Do not change the line after, unless you know what you are doing
# ----------------------------------------------------------------
export GCC=7.1.0
export CCTOOLS=$kO6/bin/gcc.$GCC
export PATH=$kO6/bin:$CCTOOLS/bin:$SOCLIB/utils/bin:$PATH
export LD_LIBRARY_PATH=$CCTOOLS/lib64:$LD_LIBRARY_PATH
shopt -s globstar   # allow the ** symbol which replace any pathname

# those lines are sometimes useful because to copy files may change the rigths
chmod u+x $kO6/bin/gcc.$GCC/mipsel-unknown-elf/bin/*
chmod u+x $kO6/bin/gcc.$GCC/bin/*
chmod u+x $kO6/bin/gcc.$GCC/libexec/gcc/mipsel-unknown-elf/$GCC/*
chmod u+x $kO6/bin/$SOC.x
chmod u+x $kO6/bin/tracelog

SOURCE="${BASH_SOURCE[0]}"
SOURCE_DIR=`dirname $SOURCE`
export LIQUIBOOK_ROOT=`readlink -f $SOURCE_DIR`
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$LIQUIBOOK_ROOT/lib
# CIAO is not used, set so MPC does not give warning
export CIAO_ROOT=/dev/null

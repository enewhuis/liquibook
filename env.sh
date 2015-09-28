SOURCE="${BASH_SOURCE[0]}"
SOURCE_DIR=`dirname $SOURCE`
if [ -z "$LIQUIBOOK_ROOT" ]; then
    READLINK='readlink'
    $READLINK --version >/dev/null 2>/dev/null
    if (( $? != 0 )); then
	echo "readlink does not exist or it does not support --version"
	echo "maybe it is not GNU readlink but BSD"
	echo "trying with greadlink..."
	READLINK='greadlink'
    fi
    $READLINK --version >/dev/null 2>/dev/null
    if (( $? != 0 )); then
	echo "greadlink does not exist or an error occurred"
	UNAME=`uname`
	if [[ $UNAME == "Darwin" ]]; then
            echo "You are running on a Mac OSX system."
            echo "Consider installing homebrew."
            echo "Then install coreutils."
            echo "# brew install coreutils"
	fi
    else
	echo "$READLINK found at `which $READLINK`."
    fi
    $READLINK -f $SOURCE_DIR
    if (( $? != 0 )); then
	echo "trying exporting LIQUIBOOK_ROOT by pwd."
	export LIQUIBOOK_ROOT=`pwd`
	echo "LIQUIBOOK_ROOT = $LIQUIBOOK_ROOT"
    else
	export LIQUIBOOK_ROOT=`$READLINK -f $SOURCE_DIR`
    fi
fi
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$LIQUIBOOK_ROOT/lib
# CIAO is not used, set so MPC does not give warning
export CIAO_ROOT=/dev/null

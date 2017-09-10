SOURCE="${BASH_SOURCE[0]}"
SOURCE_DIR=`dirname $SOURCE`

if test "$LIQUIBOOK_ROOT" = ""; then
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

if test "$MPC_ROOT" == ""; then
    echo Set MPC_ROOT variable
fi

if test "$QUICKFAST_ROOT" == "";  then
    echo QuickFAST support disabled
    export QUICKFAST_ROOT=`pwd`/noQuickFAST
    echo Set QUICKFAST_ROOT variable to configure QuickFAST
else
    LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$QUICKFAST_ROOT/lib
fi

if test "$BOOST_VERSION" = ""; then
    export BOOST_VERSION=""
fi
if test "$BOOST_ROOT" = ""; then
  export BOOST_ROOT=/usr
fi
if test "$BOOST_ROOT_LIB" = ""; then
  export BOOST_ROOT_LIB=$BOOST_ROOT/lib/x86_64-linux-gnu
fi
if test "$BOOST_CFG" = ""; then
  export BOOST_CFG=
fi
if test "$BOOST_STATIC_LIB_PREFIX" = ""; then
  export BOOST_STATIC_LIB_PREFIX=
fi

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$LIQUIBOOK_ROOT/lib
# CIAO is not used, set so MPC does not give warning
export CIAO_ROOT=/dev/null

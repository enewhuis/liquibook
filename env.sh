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

if test "$QUICKFAST_ROOT" == "";  then
  export QUICKFAST_ROOT=`pwd`/noQuickFAST
  echo QuickFAST support disabled
fi

if test "$BOOST_VERSION" = ""; then
  echo Please export BOOST_VERSION, and BOOST_CFG
  echo you can also set BOOST_ROOT if it is not /usr/boost/BOOST_VERSION
else
  if test "$BOOST_ROOT" = ""; then
    export BOOST_ROOT=/usr/boost/$BOOST_VERSION
  fi
  if test "$BOOST_ROOT_LIB" = ""; then
    export BOOST_ROOT_LIB=$BOOST_ROOT/lib
  fi
  if test "$BOOST_CFG" = ""; then  
    export BOOST_CFG=-gcc62-mt-1_63
  fi
  if test "$BOOST_STATIC_LIB_PREFIX" = ""; then
    export BOOST_STATIC_LIB_PREFIX=
  fi
fi

LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$LIQUIBOOK_ROOT/lib
# CIAO is not used, set so MPC does not give warning
export CIAO_ROOT=/dev/null

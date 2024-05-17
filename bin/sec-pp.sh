#!/bin/bash

case `uname -a` in
     CYGWIN* | cygwin* | Cygwin* )
        CTESK_HOME=`cygpath "$CTESK_HOME"`
    ;;
esac

gcc -xc -E -D__SEC__ "-I$CTESK_HOME/include" -include "$CTESK_HOME/include/ts/ts.h" "$@"


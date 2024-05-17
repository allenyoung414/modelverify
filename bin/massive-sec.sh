#!/bin/bash

###############################################################################
# Usage
###
this=`basename "$0"`

usage () {
    cat <<EOF
Massive CTesK launcher
Usage:
  $this [ options ] infile [ actions ]
  -- processes each file from the given with the given set of actions

     Arguments:
        infile - filename should be prefixed with @ symbol
     Options:
        --debug-info      -- generate debug info
        --help | -h       -- this message
EOF
}


###############################################################################
# Process parameters
###
if [ -z "$CTESK_HOME" ]; then
    echo $0: CTESK_HOME environment variable is not set
    exit 1
fi

for i in "$@"
do
  case $i in
    --help | -h )
      usage
      exit 0
    ;;
    --debug-info )
        debug_info="-J-Dsec.generate.debugger_info=enabled"
    ;;
  esac
done

###############################################################################
# Call SEC Translator
###

echo Massive CTESK processor launched ... `date`
"$CTESK_HOME/bin/secc" $debug_info "-J-Dsec.check_usage=false" "-J-Dsec.properties=$CTESK_HOME/etc/.sec.gcc.properties" $@

exit_code=$?


###############################################################################
# Unset auxiliary variables
###
unset debug_info

exit $exit_code


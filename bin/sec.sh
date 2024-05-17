#!/bin/bash

###############################################################################
# Usage
###
this=`basename "$0"`

usage () {
    cat <<EOF
Usage:
  $this [options] secfile cfile
     translate secfile to cfile

     Options:
        --sei <filename>  -- use <filename> for preprocessor output
        --keep-sei        -- keep .sei file after processing, do not erase it
        --debug-info      -- generate debug info
        --help | -h       -- this message
        other options are passed to preprocessor
EOF
}


###############################################################################
# Process parameters
###

preprocessor_options=

argc=$(( $#-2 ))

for(( i = 0; $i < $argc; i++ ))
do
  case $1 in
    --help | -h )
      usage
      exit 0
    ;;
    --sei )
      if [[ $i == $(( $argc - 1 )) ]]
      then
        echo "--sei option should be followed by seifile"
        echo ""
        usage
        exit 10
      fi
      seifile=$2
      i=$(( $i + 1 ))
      shift
    ;;
    --keep-sei )
        keep_sei=yes
    ;;
    --debug-info )
        debug_info="--debug-info"
    ;;
    * )
      preprocessor_options="$preprocessor_options $1"
  esac
  shift
done

if [[ $# != 2 ]]
then
    echo "You must specify secfile & cfile"
    usage
    exit 10
fi

secfile=$1
cfile=$2

if [ -z "$seifile" ]
then
  case $secfile in
    *.sec )
      seifile=`basename "$secfile" .sec`.sei
    ;;
    *.seh )
      seifile=`basename "$secfile" .seh`.sei
    ;;
    * )
      seifile=`basename "$secfile"`.sei
    ;;
  esac
fi

function clean_sei {
    if [ -z "$keep_sei" ]
    then
        rm "$seifile" > /dev/null 2>&1
        unset keep_sei
    fi
}


###############################################################################
# Preprocess SEC file
###
"$CTESK_HOME/bin/sec-pp.sh" $preprocessor_options "$secfile" > "$seifile"
exit_code=$?

if [ $exit_code -eq 0 ]
then
  success=1
else
  echo Preprocessor failed
  clean_sei
  exit 1
fi


###############################################################################
# Translate SEC file
###
"$CTESK_HOME/bin/sec-c.sh" $debug_info "$seifile" "$cfile"
exit_code=$?
clean_sei

if [ $exit_code -eq 0 ]
then
  success=2
else
  exit 2
fi


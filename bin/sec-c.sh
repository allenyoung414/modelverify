#!/bin/bash

###############################################################################
# Usage
###
this=`basename "$0"`

usage () {
    cat <<EOF
Usage:
  $this [ options ] infile [ outfile ] -- translate infile to outfile
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
    -* )
      echo Unknown option: $i
      usage
      exit 1
    ;;
    * )
      if [ -z "$infile" ]
      then
        infile=$i
      elif [ -z "$outfile" ]
      then
        outfile=$i
      else  
        echo Unknown argument: $i
        usage
        exit 1
      fi 
  esac
done

if [ -z "$infile" ]
then
  echo You must specify infile
  usage
  exit 1
fi

if [ -z "$outfile" ]
then
    case $infile in
      *.sec )
        outfile=`basename "$infile" .sec`.c
      ;;
      *.seh )
        outfile=`basename "$infile" .seh`.c
      ;;
      *.sei )
        outfile=`basename "$infile" .sei`.c
      ;;
      * )
        outfile=$infile.c
      ;;
    esac
fi

case `uname -a` in
    CYGWIN* | cygwin* | Cygwin* )
        CTESK_HOME=`cygpath -m "$CTESK_HOME"`
        outfile=`cygpath -m "$outfile"`
        infile=`cygpath -m "$infile"`
    ;;
esac


###############################################################################
# Call SEC Translator
###
"$CTESK_HOME/bin/secc" $debug_info "-J-Dsec.properties=$CTESK_HOME/etc/.sec.gcc.properties" "$infile" generate "-J-Dsec.generate.output.file=$outfile"

exit_code=$?


###############################################################################
# Unset auxiliary variables
###
unset infile
unset outfile
unset debug_info

exit $exit_code


#!/bin/sh
if command -v gsed >/dev/null
then
  sed_cmd=gsed
else
  sed_cmd=sed
fi
for source_file in $(grep '"file":' $1 | ${sed_cmd} -e 's/"file"://' | ${sed_cmd} -e 's/[^"]*"\([^"]*\)"/\1/' | grep -v 'pb.h' | grep -v 'pb.cc')
do
  source_dir=$(dirname ${source_file})
  for head in $(${sed_cmd} -n -e 's/^\s*#include[^"]*"\([^"]*\)"/\1/p' ${source_file})
  do
    if test -f "${source_dir}/${head}"
    then
      realpath "${source_dir}/${head}"
    fi
  done
  if test -f "${source_file}"
  then
    realpath "${source_file}"
  fi
done
  
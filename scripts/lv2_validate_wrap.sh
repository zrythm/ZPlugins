#!/usr/bin/env bash
# $1: lv2_validate
# $2: plugin build dir
# $3: plugin name
# $4: plugin URI

set -o xtrace
set -e

LV2_VALIDATE_BIN=$1
PL_BUILD_DIR=$2
PL_NAME=$3
PL_URI=$4
LIBEXT=$5

tmpdir=$(mktemp -d /tmp/lv2_validate_wrap.XXXXXXXXX)
tmp_plugin_dir="$tmpdir/$PL_NAME"
mkdir -p "$tmp_plugin_dir"
cp $PL_BUILD_DIR/$PL_NAME.ttl \
  $PL_BUILD_DIR/${PL_NAME}_dsp$LIBEXT $tmp_plugin_dir/
if [ -e $PL_BUILD_DIR/${PL_NAME}_ui$LIBEXT ] ; then
  cp $PL_BUILD_DIR/${PL_NAME}_ui$LIBEXT $tmp_plugin_dir/
fi
cp $PL_BUILD_DIR/$3_manifest.ttl $tmp_plugin_dir/manifest.ttl

LV2_PATH="$tmpdir" env $LV2_VALIDATE_BIN $PL_URI

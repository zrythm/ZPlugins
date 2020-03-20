#!/usr/bin/env bash
# $1: lv2lint
# $2: plugin build dir
# $3: plugin name
# $4: plugin URI

set -o xtrace

LV2_LINT_BIN=$1
PL_BUILD_DIR=$2
PL_NAME=$3
PL_URI=$4

tmpdir=$(mktemp -d /tmp/lv2lint_wrap.XXXXXXXXX)
tmp_plugin_dir="$tmpdir/$PL_NAME"
mkdir -p "$tmp_plugin_dir"
cp $PL_BUILD_DIR/$PL_NAME.ttl \
  $PL_BUILD_DIR/${PL_NAME}_dsp.so $tmp_plugin_dir/
cp $PL_BUILD_DIR/$3_manifest.ttl $tmp_plugin_dir/manifest.ttl

LV2_PATH="$tmpdir" env $LV2_LINT_BIN -d \
  -I $tmp_plugin_dir/ $PL_URI

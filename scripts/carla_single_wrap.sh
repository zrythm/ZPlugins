#!/usr/bin/env bash
# $1: carla-single

set -o xtrace
set -e

carla_single_bin=$1

tmpdir=$(mktemp -d /tmp/carla_single_wrap.XXXXXXXXX)
tmp_plugin_dir="$tmpdir/$PL_NAME"
mkdir -p "$tmp_plugin_dir"
cp $PL_BUILD_DIR/$PL_NAME.ttl \
  $PL_BUILD_DIR/${PL_NAME}_dsp$LIBEXT $tmp_plugin_dir/
if [ -e $PL_BUILD_DIR/${PL_NAME}_ui$LIBEXT ] ; then
  cp $PL_BUILD_DIR/${PL_NAME}_ui$LIBEXT $tmp_plugin_dir/
fi
cp $PL_BUILD_DIR/${PL_NAME}_manifest.ttl $tmp_plugin_dir/manifest.ttl
for lv2dir in atom.lv2 buf-size.lv2 core.lv2 data-access.lv2 \
  dynmanifest.lv2 event.lv2 instance-access.lv2 log.lv2 midi.lv2 \
  morph.lv2 options.lv2 parameters.lv2 patch.lv2 port-groups.lv2 \
  port-props.lv2 presets.lv2 resize-port.lv2 schemas.lv2 \
  state.lv2 time.lv2 ui.lv2 units.lv2 urid.lv2 uri-map.lv2 \
  worker.lv2; do
  cp -r $LV2_DIR/$lv2dir $tmpdir/
done

LV2_PATH="$tmpdir" \
  CARLA_BRIDGE_DUMMY=1 \
  CARLA_BRIDGE_TESTING=native \
  env $carla_single_bin $PL_URI

rm -rf $tmpdir

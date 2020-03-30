#!/usr/bin/env bash

cd @CURRENT_SOURCE_DIR@
@GUILE@ -s manifest_gen.scm "$@"

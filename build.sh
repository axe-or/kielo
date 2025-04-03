#!/usr/bin/env sh

cc=gcc
cflags='-O0 -std=c17 -fPIC -fno-strict-aliasing -fwrapv -g'
ldflags=''

Run(){ echo "$@"; $@; }

set -eu
Run $cc $cflags -o kielo.exe main.c base/base.c $ldflags


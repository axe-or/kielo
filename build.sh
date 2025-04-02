#!/usr/bin/env sh

cc=clang
cflags='-O1 -std=c17 -fPIC -fno-strict-aliasing -fwrapv'
ldflags=''

Run(){ echo "$@"; $@; }

set -eu
Run $cc $cflags -o kielo.exe main.c base/base.c $ldflags


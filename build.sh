#!/usr/bin/env sh

cc=clang
cflags='-O2 -std=c17 -fPIC -fno-strict-aliasing -fwrapv -fsanitize=address -g'
ldflags=''

set -eu
$cc $cflags -o kielo.exe main.c base/base.c $ldflags


#!/bin/sh

set -v
set -e

if test -d .testdir; then
   rm -r .testdir
fi

mkdir -p .testdir/dev
printf '%i\n' 50 > .testdir/dev/max_brightness
printf '%i\n' 25 > .testdir/dev/brightness

test $(./test -ga) = 50.00%
test $(./test -g dev) = 50.00%

./test -s +30 dev
test $(./test -ga) = 100.00%

./test -s 5 dev
test $(./test -ga) = 10.00%

./test -s -1 dev
test $(./test -ga) = 8.00%

./test -s +100%% dev
test $(./test -ga) = 16.00%

./test -s -50%% dev
test $(./test -ga) = 8.00%

./test -s 200%% dev
test $(./test -ga) = 16.00%

./test -s 50% dev
test $(./test -ga) = 50.00%

./test -s -10% dev
test $(./test -ga) = 40.00%

./test -s +20% dev
test $(./test -ga) = 60.00%

mkdir .testdir/acpi_videox
printf '%i\n' 50 > .testdir/acpi_videox/max_brightness
printf '%i\n' 25 > .testdir/acpi_videox/brightness

test $(./test -g) = 50.00%

rm -r .testdir

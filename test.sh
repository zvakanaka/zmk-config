#!/usr/bin/env bash
# Run the simulator tests in tests/ against this repo's ZMK module, inside the ZMK build container.
# ./build.sh must have run once first so .zmk/ has ZMK and Zephyr.
#
#   ./test.sh            run all tests
#   ./test.sh --accept   rewrite the expected keycode snapshots from the current output
set -euo pipefail

cd "$(dirname "$0")"
IMAGE=${ZMK_IMAGE:-docker.io/zmkfirmware/zmk-build-arm:stable}
accept=
[ "${1:-}" = --accept ] && accept=y

podman run --rm -i \
  -v "$PWD/.zmk:/ws:z" \
  -v "$PWD/config:/ws/config:z" \
  -v "$PWD:/zmk-config:z" \
  -e ZMK_TESTS_AUTO_ACCEPT="$accept" \
  -w /ws "$IMAGE" bash -s <<'SH'
set -euo pipefail
west zephyr-export >/dev/null
cd zmk/app
ZMK_EXTRA_MODULES=/zmk-config ZMK_BUILD_DIR=/ws/testbuild J=$(nproc) ./run-test.sh /zmk-config/tests
SH

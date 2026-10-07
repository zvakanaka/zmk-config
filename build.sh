#!/usr/bin/env bash
# Build every target in build.yaml inside the ZMK build container (podman).
#
#   ./build.sh            build all targets
#   ./build.sh corne_left build only targets whose artifact-name contains "corne_left"
#   ./build.sh --update   refresh ZMK/Zephyr sources (west update) before building
#   ./build.sh --pristine force clean builds
#
# The west workspace (ZMK, Zephyr, modules) lives in .zmk/ and is reused between runs.
# Firmware is written to firmware/<artifact-name>.uf2.
set -euo pipefail

cd "$(dirname "$0")"
IMAGE=${ZMK_IMAGE:-docker.io/zmkfirmware/zmk-build-arm:stable}

update=0 pristine=auto filter=
for arg; do
  case $arg in
    --update) update=1 ;;
    --pristine) pristine=always ;;
    -h|--help) sed -n '2,10p' "$0"; exit ;;
    *) filter=$arg ;;
  esac
done

mkdir -p .zmk firmware

podman run --rm -i \
  -v "$PWD/.zmk:/ws:z" \
  -v "$PWD/config:/ws/config:z" \
  -v "$PWD:/zmk-config:z" \
  -e UPDATE="$update" -e PRISTINE="$pristine" -e FILTER="$filter" \
  -w /ws "$IMAGE" bash -s <<'EOF'
set -euo pipefail
if [ ! -d .west ]; then
  west init -l config
  UPDATE=1
fi
if [ "$UPDATE" = 1 ] || [ ! -d zmk ]; then
  west update --fetch-opt=--filter=tree:0
fi
west zephyr-export >/dev/null

python3 - <<'PY' > /tmp/targets
import os, shlex, yaml
for t in yaml.safe_load(open("/zmk-config/build.yaml"))["include"]:
    shield = t.get("shield", "")
    name = t.get("artifact-name") or (shield.split()[0] + "-" if shield else "") + t["board"].replace("/", "_")
    if os.environ["FILTER"] not in name:
        continue
    print(shlex.join([name, t["board"], shield, t.get("snippet", ""), t.get("cmake-args", "")]))
PY

[ -s /tmp/targets ] || { echo "no targets match '$FILTER'" >&2; exit 1; }
while read -r line <&3; do
  eval "set -- $line"
  name=$1 board=$2 shield=$3 snippet=$4 cmake_args=$5
  echo "=== $name: $board ${shield:+($shield)}"
  # shellcheck disable=SC2086
  west build -s zmk/app -d "build/$name" -b "$board" -p "$PRISTINE" ${snippet:+-S "$snippet"} -- \
    -DZMK_CONFIG=/ws/config -DZMK_EXTRA_MODULES=/zmk-config ${shield:+-DSHIELD="$shield"} $cmake_args
  cp "build/$name/zephyr/zmk.uf2" "/zmk-config/firmware/$name.uf2"
done 3< /tmp/targets
EOF

ls -l firmware/*.uf2

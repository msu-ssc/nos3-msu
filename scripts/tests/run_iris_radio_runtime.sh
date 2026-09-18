#!/usr/bin/env bash
set -euo pipefail
nos3_root=$(cd "$(dirname "$0")/../.." && pwd)
docker run --rm --network none \
    --sysctl fs.mqueue.msg_max=10000 --ulimit rtprio=99 --cap-add=sys_nice \
    --add-host nos-engine-server:127.0.0.1 \
    --add-host sc01-nos-engine-server:127.0.0.1 \
    --add-host nos-fsw:127.0.0.1 --add-host radio-sim:127.0.0.1 \
    --add-host cryptolib:127.0.0.1 --add-host cosmos:127.0.0.1 \
    -v "$nos3_root:$nos3_root:ro" \
    ivvitc/nos3-64:20260619 python3 "$nos3_root/scripts/tests/iris_radio_runtime.py"

# IRIS transmit power

The cFS IRIS radio app can set the simulator's transmit-power setting to an
integer from 0 to 100 mW. This setting does not yet affect RF propagation,
forwarding, or link availability. The existing target name is `GENERIC_RADIO`.

## Use it

After pulling the changes and initializing the submodules, run `make config`
and rebuild flight software, simulators, and ground software. Restart NOS3 and
COSMOS to load the rebuilt app and packet definitions. This update grows radio
housekeeping from 33 to 38 bytes; flight and ground updates must be deployed
together.

In COSMOS Command Sender:

1. Select target `GENERIC_RADIO` on the debug interface (or
   `GENERIC_RADIO_RADIO` on the radio interface).
2. Select `IRIS_RADIO_SET_POWER`.
3. Set `MILLIWATTS` to `50`, then send.
4. Send `GENERIC_RADIO_REQ_HK`, or wait for scheduled housekeeping.
5. In Packet Viewer, select the same target and `GENERIC_RADIO_HK_TLM`.
   Expect `TRANSMIT_POWER_MW = 50` and `POWER_VALID = 1` in a fresh packet.

The component also provides a `power` screen in Telemetry Viewer. The cFS event
on a successful send says that confirmation is pending: only returned device
housekeeping confirms the setting. Invalid values increment `CMD_ERR_COUNT`.
Failed device reads increment `DEVICE_ERR_COUNT`, retain the last power value,
and clear `POWER_VALID`. Check packet freshness, since a stopped telemetry
stream cannot update even the validity field.

## Implementation

- cFS command MID `0x1930`, function code `4`, 32-bit native-order mW payload.
- Device command `02` sets power using a big-endian 32-bit payload.
- Device command `03` returns the 20-byte extended housekeeping response.
- The cFS app appends `TRANSMIT_POWER_MW` and `POWER_VALID` to its ground
  housekeeping packet. The original device command `00` still returns legacy
  16-byte housekeeping for other clients.
- The COSMOS/OpenC3 definitions, COSMOS screen, and Yamcs XTCE match the new
  packet. The separate F Prime application is outside this cFS integration.

The NOS hwlib UDP sender also needed a fix: its IPv4 enum value is `0`, while
POSIX requires `AF_INET` (`2`) in `sockaddr_in.sin_family`. The correction is in
`fsw/apps/hwlib/sim/src/libsocket.c`. Without it, runtime tests reported UDP
send errors even though all builds passed.

## Verification

cFS and simulator builds passed. The live test ran the actual cFS radio app,
command ingest, telemetry output, NOS engine/time driver, and radio simulator
inside an isolated container. It used a static radio provider and the UDP
debug interface, without 42, CryptoLib routing, or the COSMOS desktop.

It verified 0, 50, and 100 mW, rejection of 101 and `UINT32_MAX`, incorrect
command length, device loss, and recovery. COSMOS's own packet parser separately
verified command bytes, input limits, and telemetry decoding. Yamcs XML and
field references were checked; a Yamcs server was not launched.

From the NOS3 root, after building:

```sh
bash components/iris_radio/fsw/shared/tests/run.sh
bash scripts/tests/run_iris_radio_runtime.sh
docker run --rm --network none --entrypoint ruby \
  -v "$PWD/components/iris_radio:/radio:ro" \
  ballaerospace/cosmos:4.5.0 /radio/gsw/tests/power_packets_test.rb
```

## Commit and publish

There are now **three repositories** with related changes: IRIS radio, hwlib,
and NOS3. Review `git diff --cached` in each repository before committing.

First publish the radio changes:

```sh
cd components/iris_radio
git add README.md fsw/cfs/src/generic_radio_app.c \
  fsw/cfs/src/generic_radio_msg.h fsw/cfs/src/generic_radio_events.h gsw/
git diff --cached --stat
git commit -m "Add IRIS power command and housekeeping telemetry"
git push -u origin HEAD
cd ../..
```

The hwlib origin now points to `https://github.com/msu-ssc/hwlib.git`.
Publish its fix next. Creating a branch also handles a detached submodule
checkout:

```sh
git -C fsw/apps/hwlib switch -c iris-udp-fix
git -C fsw/apps/hwlib add sim/src/libsocket.c
git -C fsw/apps/hwlib commit -m "Fix IPv4 UDP destination address family"
git -C fsw/apps/hwlib push -u origin iris-udp-fix
```

Only after both submodule commits are published, save the parent references:

```sh
git add .gitmodules components/iris_radio fsw/apps/hwlib \
  scripts/tests/iris_radio_runtime.py scripts/tests/run_iris_radio_runtime.sh \
  docs/wiki/IRIS_Radio_Power.md
git diff --cached --stat
git commit -m "Integrate IRIS power control and corrected UDP transport"
git push
```

Changes are uncommitted and have not been pushed. The hwlib submodule URL and
local origin now point to the MSU fork.

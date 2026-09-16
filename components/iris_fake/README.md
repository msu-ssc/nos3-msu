# IRIS_FAKE: a GPIO learning example

This is not an Iris protocol implementation. GENERIC_RADIO still carries normal
commands and telemetry. This second, fake radio only drives CDH GPIO1.

## Follow the signal through the code

1. `gsw/IRIS_FAKE_SIM_CMD.txt`: COSMOS sends `GPO1=0` or `GPO1=1` to
   `iris-fake-command` through SIM_CMDBUS_BRIDGE. These are simulator commands,
   not flight commands.
2. `sim/iris_fake_sim.cpp`: the NOS3 command callback forwards the command to
   `sim/iris_fake.py` over localhost TCP port 12021. Python owns GPO1's state
   and replies with its level. C++ reflects that level by overwriting one ASCII
   byte in `/tmp/gpio-fake/gpio1/value`. There is no SPI or dynamics provider.
3. `scripts/fsw/fsw_cfs_launch.sh`: creates that file initially low and mounts
   the same spacecraft-specific directory into the simulator and flight
   containers. It starts Python, which opens its listening socket before
   launching the C++ adapter as a child process in the same container.
   The app treats GPIO1 as an input; only the simulator drives it.
4. `fsw/cfs/src/iris_fake_app.c`: reads GPIO1 on each 10 Hz scheduler request.
   The first valid sample establishes a baseline. Each subsequent change
   increments the counter and emits event 1. Repeated levels do not count.
5. Once per second another scheduler request publishes `IRIS_HK`, with the
   standard cFS header followed by UINT32 `IRIS_PIN_TRANSITION_COUNT` and UINT8
   `IRIS_PIN_CURRENT_STATE`. The counter resets when the app starts.
6. The existing TO_LAB and TO subscription tables forward this packet to the
   debug and radio interfaces. COSMOS decodes the same packet as IRIS_FAKE or
   IRIS_FAKE_RADIO, respectively.

The two scheduler request IDs and telemetry ID are in
`fsw/cfs/platform_inc/iris_fake_msgids.h`. Scheduler message entries 120/121
and unused schedule slots implement 10 Hz polling and 1 Hz housekeeping.
The app is always included in the default cFS/COSMOS configuration for this
exercise; there is no new GUI enable option. Alternate launchers/ground systems
are outside this example.

The private Python/C++ protocol is deliberately small: one connection per
command, with a newline-terminated `GPO1=0` or `GPO1=1` request and a
newline-terminated `0` or `1` response. Python prints each successful drive in
the Iris simulator terminal, including repeated levels. C++ waits for the reply
in its command callback. There are no reconnects, timeouts, unsolicited output
updates, or simulation-time synchronization in this learning example.

## Build and try it

Stop the current simulation before rebuilding/relaunching:

```sh
make stop
make config
make
make launch
```

Open COSMOS from the launcher. In Packet Viewer select target `IRIS_FAKE`,
packet `IRIS_HK`. Wait for its received count to increase. In Command Sender,
select target `SIM_CMDBUS_BRIDGE` and command `IRIS_SIM_GPO1_DRIVE`. Set
`GPO1_VALUE` as shown below, waiting at least one second between commands
(or until the expected telemetry arrives).

| Action | GPO1_VALUE | State | Transition count |
| --- | ---: | ---: | ---: |
| Initial startup | — | 0 | 0 |
| Drive high | 1 | 1 | 1 |
| Drive high again | 1 | 1 | 1 |
| Drive low | 0 | 0 | 2 |
| Drive high | 1 | 1 | 3 |

Transition events also appear in the FSW console. To view the packet through
GENERIC_RADIO, enable the normal radio downlink with `CFS TO_ENABLE_OUTPUT`,
`DEST_IP=radio-sim`, `DEST_PORT=5011`, and select `IRIS_FAKE_RADIO / IRIS_HK`.

Polling counts observed level changes, not every possible physical edge. A
pulse that starts and ends between polls can be missed. Failed reads leave the
last state/count unchanged. This example does not implement interrupts,
debouncing, counter persistence, or the real Iris ICD.

## Reading the diff

The new component contains the behavior. The surrounding changes register its
build/startup, scheduler requests, simulator, telemetry routes, and COSMOS targets.
The GPIO helper change prevents input initialization from driving a level and
makes reads consume exactly one valid ASCII byte.

`gsw/cosmos` and `fsw/apps/hwlib` are existing Git submodules. View their actual
file diffs with `git -C gsw/cosmos diff` and `git -C fsw/apps/hwlib diff`;
the parent repository only reports them as dirty. Generated files under
`cfg/build`, `fsw/build`, and `sims/build` are not source changes.

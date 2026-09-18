"""Live smoke test of the built cFS app, NOS time/engine, and IRIS simulator.

Run via run_iris_radio_runtime.sh, inside an isolated Docker network namespace.
The test copies build artifacts to /tmp and uses a minimal startup file and
static radio data provider; it does not start 42 or the COSMOS GUI. UDP commands
use the same packet layout separately tested by COSMOS's own packet parser.
The mission files in the host checkout are mounted read-only.
"""
import os, pathlib, shutil, subprocess, time, socket, struct, json
root = pathlib.Path(__file__).resolve().parents[2]
import tempfile
work = pathlib.Path(tempfile.mkdtemp(prefix='iris-runtime-'))
shutil.copytree(root / 'fsw/build/exe/cpu1', work / 'fsw', dirs_exist_ok=True)
p = work / 'fsw/cf/cfe_es_startup.scr'
lines = p.read_text().splitlines()
keep = {'hwlib', 'io_lib', 'sch', 'ci_lab', 'to_lab', 'generic_radio'}
p.write_text('\n'.join((l for l in lines if l.startswith(('CFE_APP', 'CFE_LIB')) and l.split(',')[1].strip() in keep)) + '\n!\n')
simdir = root / 'sims/build/bin'
config = (root / 'cfg/build/sims/nos3-simulator.xml').read_text().replace('</fdw-port>', '</fwd-port>')
import xml.etree.ElementTree as ET
import re
config = re.sub('<!--.*?-->', '', config, flags=re.S)
common = re.search('<common>.*?</common>', config, re.S).group()
blocks = re.findall('<simulator>.*?</simulator>', config, re.S)
blocks = [b for b in blocks if re.search('<name>(time|generic-radio-sim|cmdbus-bridge)</name>', b)]
xml = ET.fromstring('<nos3-configuration>' + common + '<simulators>' + ''.join(blocks) + '</simulators></nos3-configuration>')
for node in list(xml.find('simulators')):
    if node.findtext('name') not in ('time', 'generic-radio-sim', 'cmdbus-bridge'):
        xml.find('simulators').remove(node)
for node in xml.iter('nos-connection-string-override'):
    node.text = 'tcp://nos-engine-server:12001'
radio = next((n for n in xml.find('simulators') if n.findtext('name') == 'generic-radio-sim'))
dp = radio.find('hardware-model/data-provider')
dp.clear()
ET.SubElement(dp, 'type').text = 'GENERIC_RADIO_PROVIDER'
xml.find('common/log-config-file').text = str(simdir / 'sim_log_config.xml')
ET.ElementTree(xml).write(work / 'sim.xml')
processes = []
logs = []

def launch(name, args, cwd=work, env=None):
    f = open(work / (name + '.log'), 'w')
    logs.append(f)
    p = subprocess.Popen(args, cwd=cwd, stdin=subprocess.PIPE, stdout=f, stderr=subprocess.STDOUT, env=env)
    processes.append(p)
    return p
try:
    launch('engine', ['/usr/bin/nos_engine_server_standalone', '-f', str(simdir / 'nos_engine_server_config.json')])
    time.sleep(1)
    env = os.environ.copy()
    env.update(GCOV_PREFIX='/tmp/iris-gcov', TERM='xterm', TCP_GROUND='0', MULTI_GDS='0', LD_LIBRARY_PATH=str(root / 'sims/build/lib') + ':/usr/local/lib:/usr/lib')
    radio_proc = launch('radio', [str(simdir / 'nos3-single-simulator'), '-f', str(work / 'sim.xml'), 'generic-radio-sim'], env=env)
    launch('bridge', [str(simdir / 'nos3-sim-cmdbus-bridge'), '-f', str(work / 'sim.xml')], env=env)
    launch('time', [str(simdir / 'nos3-single-simulator'), '-f', str(work / 'sim.xml'), 'time'], env=env)
    env['LD_LIBRARY_PATH'] = str(work / 'fsw') + ':' + env['LD_LIBRARY_PATH']
    launch('cfs', ['./core-cpu1', '-R', 'PO'], cwd=work / 'fsw', env=env)
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(('127.0.0.1', 5013))
    sock.settimeout(0.5)
    seq = 0

    def send(mid, fc, payload=b''):
        global seq
        seq += 1
        packet = bytearray(struct.pack('>HHHBB', mid, 49152 | seq & 16383, len(payload) + 1, fc, 0) + payload)
        check = 255
        for b in packet:
            check ^= b
        packet[7] = check
        sock.sendto(packet, ('127.0.0.1', 5012))

    def hk(timeout=15):
        # The device read waits 250 ms. Avoid overwhelming its command pipe,
        # and discard older telemetry before requesting a fresh sample.
        time.sleep(0.75)
        sock.setblocking(False)
        try:
            while True:
                sock.recv(4096)
        except BlockingIOError:
            pass
        finally:
            sock.settimeout(0.5)
        end = time.monotonic() + timeout
        while time.monotonic() < end:
            send(6449, 0)
            try:
                while True:
                    b = sock.recv(4096)
                    if len(b) >= 6 and struct.unpack('>H', b[:2])[0] == 2352:
                        assert len(b) == 38, (len(b), b.hex())
                        return {'errors': b[16], 'commands': b[17], 'device_errors': b[18], 'power': struct.unpack_from('<I', b, 33)[0], 'valid': b[37]}
            except socket.timeout:
                pass
        raise AssertionError('No radio housekeeping received')
    time.sleep(8)
    send(0x18e8, 2, b'127.0.0.1\x00'.ljust(16, b'\x00') + struct.pack('<H', 5013))
    print('Initial', hk(), flush=True)
    for power in (0, 50, 100):
        send(6448, 4, struct.pack('<I', power))
        end = time.monotonic() + 10
        while True:
            result = hk()
            if result['power'] == power and result['valid'] == 1:
                print('Set', power, result, flush=True)
                break
            assert time.monotonic() < end

    def await_hk(predicate):
        deadline = time.monotonic() + 10
        while time.monotonic() < deadline:
            result = hk()
            if predicate(result):
                return result
        raise AssertionError(('Telemetry condition not met', result))
    for invalid in (101, 4294967295):
        before = hk()
        send(6448, 4, struct.pack('<I', invalid))
        after = await_hk(lambda h: h['errors'] == (before['errors'] + 1) % 256)
        assert after['power'] == 100 and after['valid'] == 1 and (after['commands'] == before['commands']), (before, after)
        print('Invalid power rejected:', invalid, after, flush=True)
    before = hk()
    send(6448, 4)
    after = await_hk(lambda h: h['errors'] == (before['errors'] + 1) % 256)
    assert after['power'] == 100 and after['commands'] == before['commands'], (before, after)
    print('Bad command length rejected:', after, flush=True)
    # Use the supported failure injection: killing a NOS time-bus client can
    # disrupt the engine's clock and prevent cFS from completing its delay.
    bridge = socket.create_connection(('127.0.0.1', 12020), timeout=5)
    bridge.sendall((json.dumps({'node': 'radio-sim-command-node', 'cmd': 'DISABLE'})+'\n').encode())
    result = await_hk(lambda h: h['valid'] == 0)
    assert result['power'] == 100, result
    print('Device loss marks last value invalid:', result, flush=True)
    bridge.sendall((json.dumps({'node': 'radio-sim-command-node', 'cmd': 'ENABLE'})+'\n').encode())
    result = await_hk(lambda h: h['valid'] == 1 and h['power'] == 0)
    print('Device recovery reports reset power:', result, flush=True)
    print('LIVE IRIS END-TO-END TEST PASSED', flush=True)
finally:
    for p in reversed(processes):
        if p.poll() is None:
            p.terminate()
            try:
                p.wait(timeout=3)
            except subprocess.TimeoutExpired:
                p.kill()
                p.wait()
    for f in logs:
        f.close()
    import sys
    if sys.exc_info()[0] is not None:
        for name in ['cfs', 'radio', 'engine', 'time']:
            p = work / (name + '.log')
            print('\n' + name + ' log tail:\n' + p.read_text(errors='replace')[-2200:], flush=True)

"""Toy Iris model. Run with the NOS3 adapter command as arguments."""

import socketserver
import subprocess
import sys


class IrisRequest(socketserver.StreamRequestHandler):
    def handle(self):
        command = self.rfile.readline().decode().strip()
        if command not in ("GPO1=0", "GPO1=1"):
            self.wfile.write(b"Expected GPO1=0 or GPO1=1\n")
            return

        self.server.gpo1 = int(command[-1])
        print(f"Python Iris: drove GPO1 {'HIGH' if self.server.gpo1 else 'LOW'}",
              flush=True)
        self.wfile.write(f"{self.server.gpo1}\n".encode())


if __name__ == "__main__":
    # Bind before starting C++ so its first command can connect immediately.
    with socketserver.TCPServer(("127.0.0.1", 12021), IrisRequest) as server:
        server.gpo1 = 0
        server.timeout = 0.5
        print("Python Iris: GPO1 initially LOW; listening on localhost:12021",
              flush=True)
        adapter = subprocess.Popen(sys.argv[1:])
        try:
            while adapter.poll() is None:
                server.handle_request()
        finally:
            if adapter.poll() is None:
                adapter.terminate()
            adapter.wait()
        sys.exit(adapter.returncode)

import os
import sys
import time
import socket
import random
import subprocess
import threading
from pathlib import Path

BUILD_VERSION = 1
PREFERRED_PORT = 3000

def find_free_port(start=3000):
    def is_free(port):
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            try:
                s.bind(("", port))
                return True
            except OSError:
                return False

    if is_free(start):
        return start

    for _ in range(20):
        port = random.randint(3001, 9999)
        if is_free(port):
            return port

    return None

def get_mtimes():
    mtimes = {}
    for p in Path("src").rglob("*.pysx"):
        mtimes[str(p)] = p.stat().st_mtime
    return mtimes

def serve(port):
    from http.server import SimpleHTTPRequestHandler
    from socketserver import TCPServer

    class QuietHandler(SimpleHTTPRequestHandler):
        def log_message(self, format, *args):
            pass

        def do_GET(self):
            if self.path == '/version':
                self.send_response(200)
                self.send_header('Content-type', 'text/plain')
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
                self.wfile.write(str(BUILD_VERSION).encode())
            else:
                super().do_GET()

    TCPServer.allow_reuse_address = True
    try:
        httpd = TCPServer(("", port), QuietHandler)
        print(f"🌐 Live Server running at http://localhost:{port}/public/index.html")
        httpd.serve_forever()
    except OSError as e:
        print(f"❌ Could not start dev server: {e}")
        sys.exit(1)

def main():
    global BUILD_VERSION

    port = find_free_port(PREFERRED_PORT)

    if port is None:
        print("❌ Could not find a free port. Please free up a port and try again.")
        sys.exit(1)

    if port != PREFERRED_PORT:
        print(f"⚠️  Port {PREFERRED_PORT} is already in use.")
        print(f"   Found free port: {port}")
        try:
            answer = input(f"   Continue with port {port}? [Y/n] ").strip().lower()
        except (KeyboardInterrupt, EOFError):
            print("\n🛑 Cancelled.")
            sys.exit(0)

        if answer in ("n", "no"):
            print("🛑 Exiting. Free up port 3000 and try again.")
            sys.exit(0)

    print("🚀 Starting ReactXPy Live Development Server...")
    subprocess.run([sys.executable, "build.py"], check=False)

    server_thread = threading.Thread(target=serve, args=(port,), daemon=True)
    server_thread.start()

    print("👀 Watching for file changes in src/ directory...")
    last_mtimes = get_mtimes()

    try:
        while True:
            time.sleep(0.5)
            current_mtimes = get_mtimes()

            if current_mtimes != last_mtimes:
                print("\n✨ File change detected! Rebuilding...")
                subprocess.run([sys.executable, "build.py"], check=False)
                last_mtimes = current_mtimes
                BUILD_VERSION += 1

    except KeyboardInterrupt:
        print("\n🛑 Stopping dev server...")
        sys.exit(0)

if __name__ == "__main__":
    main()

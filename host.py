#!/usr/bin/python3
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import sys

class InfiniteZeroHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        self.send_response(200)
        self.send_header('Content-Type','application/octet-stream')
        self.end_headers()
        try:
            while True:
                self.wfile.write(b'\0' * 524288)
        except BrokenPipeError:
            pass
        except Exception as e:
            print("Connection error:", e, file=sys.stderr)

server = ThreadingHTTPServer(('0.0.0.0', 80), InfiniteZeroHandler)
print("Serving infinite zeros on 0.0.0.0:80...")
try:
    server.serve_forever()
except KeyboardInterrupt:
    print("\nShutting down...")
    server.server_close()

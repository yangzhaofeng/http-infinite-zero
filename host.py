#!/usr/bin/python3
from http.server import BaseHTTPRequestHandler, HTTPServer
import sys

class InfiniteZeroHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        self.send_response(200)
        self.send_header('Content-Type','application/octet-stream')
        self.end_headers()
        while True:
            self.wfile.write(b'\0' * 524288)

server = HTTPServer(('0.0.0.0', 80), InfiniteZeroHandler)
print("Serving infinite zeros on 0.0.0.0:80...")
server.serve_forever()

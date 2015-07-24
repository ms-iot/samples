""" Copyright (c) Microsoft. All rights reserved."""

import http.server
import socketserver
import _wingpio as gpio

led_pin = 5
led_status = gpio.HIGH

gpio.setup(led_pin, gpio.OUT, gpio.PUD_OFF, led_status)

class BlinkyRequestHandler(http.server.BaseHTTPRequestHandler):
    def do_HEAD(self):
        self.send_response(200)
        self.send_header("Content-type", "text/plain")
        self.end_headers()
    def do_GET(self):
        global led_status
        if led_status == gpio.LOW:
            self.wfile.write(b"Setting pin to HIGH")
            print('Setting pin to HIGH')
            led_status = gpio.HIGH
        else:
            self.wfile.write(b"Setting pin to LOW")
            print('Setting pin to LOW')
            led_status = gpio.LOW
        gpio.output(led_pin, led_status)        

httpd = http.server.HTTPServer(("", 8000), BlinkyRequestHandler)
print('Started web server on port %d' % httpd.server_address[1])
httpd.serve_forever()
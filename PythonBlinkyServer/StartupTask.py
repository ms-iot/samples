"""
    Copyright(c) Microsoft Open Technologies, Inc. All rights reserved.

    The MIT License(MIT)

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files(the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions :

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
"""
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
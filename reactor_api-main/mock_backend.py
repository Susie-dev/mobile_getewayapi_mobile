from http.server import HTTPServer, BaseHTTPRequestHandler
import sys

class SimpleHTTPRequestHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        self.send_response(200)
        self.send_header('Content-type', 'application/json')
        self.end_headers()
        
        # 根据端口号区分不同的服务
        port = self.server.server_port
        if port == 8081:
            response = '{"code": 0, "msg": "User Service Node 1 Response", "port": 8081}'
        elif port == 8082:
            response = '{"code": 0, "msg": "User Service Node 2 Response", "port": 8082}'
        elif port == 9091:
            response = '{"code": 0, "msg": "Order Service Response", "port": 9091}'
        else:
            response = '{"code": 0, "msg": "Unknown Service"}'
            
        self.wfile.write(response.encode())

def run(port):
    server_address = ('127.0.0.1', port)
    httpd = HTTPServer(server_address, SimpleHTTPRequestHandler)
    print(f"Mock Backend Service running on port {port}...")
    httpd.serve_forever()

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Usage: python3 mock_backend.py <port>")
        sys.exit(1)
    port = int(sys.argv[1])
    run(port)

import socket
import json
from PyQt5.QtCore import QThread, pyqtSignal, QRunnable, QThreadPool

class Worker(QRunnable):
    def __init__(self, function, *args, **kwargs):
        super(Worker, self).__init__()
        self.function = function
        self.args = args
        self.kwargs = kwargs

    def run(self):
        self.function(*self.args, **self.kwargs)

class ListenerThread(QThread):
    connected_signal = pyqtSignal()
    received_signal = pyqtSignal(dict)
    disconnected_signal = pyqtSignal()

    def __init__(self, ip, port=1803):
        super().__init__()
        self.ip = ip
        self.port = port
        self.is_running = True
        self.threadpool = QThreadPool()
        self.i = 0

    def run(self):
        # Start TCP handling in its own thread
        self.threadpool.start(Worker(self.handle_tcp))

        # Start UDP handling in its own thread
        self.threadpool.start(Worker(self.handle_udp))

    def handle_tcp(self):
        self.s_tcp = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  
        try:
            self.s_tcp.connect((self.ip, self.port))
            self.s_tcp.sendall(b"WIPADRETRO_CONNECT")
            self.s_tcp.settimeout(2.0)
            print(f"Connected to {self.ip}:{self.port}")

            while self.is_running:
                try:
                    data_tcp = self.s_tcp.recv(1024)
                    if data_tcp:
                        self.handle_tcp_data(data_tcp)
                except socket.timeout:
                    pass  # No TCP data received

        except Exception as e:
            print(f"TCP Connection closed or error: {e}")
            self.disconnected_signal.emit()

        finally:
            if self.s_tcp: self.s_tcp.close()

    def handle_udp(self):
        self.s_udp = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        try:
            self.s_udp.bind(('0.0.0.0', self.port))
            self.s_udp.settimeout(0.01)  # Short timeout for non-blocking behavior

            while self.is_running:
                try:
                    data_udp, addr = self.s_udp.recvfrom(1024)
                    if data_udp:
                        self.handle_udp_data(data_udp)
                except socket.timeout:
                    pass  # No UDP data received

        except Exception as e:
            print(self.ip, self.port)
            print(f"UDP Connection closed or error: {e}")

        finally:
            print("Trying to close UDP socket")
            if self.s_udp: self.s_udp.close()

    def handle_tcp_data(self, data):
        message = data.decode('utf-8')

        # Handling connection
        if "WIPADRETRO_CONNECTED" in message:
            self.connected_signal.emit()
            return

        # Handling disconnection
        if "WIPADRETRO_DISCONNECT" in message:
            self.disconnected_signal.emit()
            return

        # Handling received input
        try:
            keyinput = json.loads(message)
            self.received_signal.emit(keyinput)
        except Exception as e:
            print(message)
            print(f"Invalid data format for TCP: {e}")
            self.disconnected_signal.emit()

    def handle_udp_data(self, data):
        message = data.decode('utf-8')
        try:
            keyinput = json.loads(message)
            self.received_signal.emit(keyinput)
            self.i += 1
            print('a', self.i)
        except Exception as e:
            print(f"Invalid data format for UDP: {e}")

    def stop(self):
        self.is_running = False  # Indicate the threads should stop
        self.close_sockets()     # Close sockets to force exit from blocking calls
        self.quit()
        self.wait()

    def close_sockets(self):
        """ Close the TCP and UDP sockets to force exit from blocking calls """
        try:
            if hasattr(self, 's_tcp') and self.s_tcp:
                self.s_tcp.shutdown(socket.SHUT_RDWR)  # Shutdown TCP socket
                self.s_tcp.close()  # Close TCP socket
        except Exception as e:
            print("Error closing TCP socket:", e)
        
        try:
            if hasattr(self, 's_udp') and self.s_udp:
                self.s_udp.shutdown(socket.SHUT_RDWR)  # Shutdown UDP socket
                self.s_udp.close()  # Close UDP socket
        except Exception as e:
            print("Error closing UDP socket:", e)

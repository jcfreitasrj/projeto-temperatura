import serial
import threading
import json
import time
import os
from datetime import datetime, timezone

class SerialReader(threading.Thread):
    def __init__(self, port=None, baudrate=9600):
        super().__init__()
        self.port = port or os.getenv('SERIAL_PORT', '/dev/ttyUSB0')
        self.baudrate = baudrate
        self.serial_conn = None
        self.latest_data = {'temperatura': None, 'umidade': None, 'timestamp': None}
        self.running = True
        self.lock = threading.Lock()
        self.daemon = True

    def run(self):
        while self.running:
            try:
                if self.serial_conn is None or not self.serial_conn.is_open:
                    self.serial_conn = serial.Serial(self.port, self.baudrate, timeout=1)
                    time.sleep(2)
                    print(f"Conectado à porta serial {self.port}")

                if self.serial_conn.in_waiting:
                    line = self.serial_conn.readline().decode('utf-8').strip()
                    if line:
                        print(f"Linha recebida: {line}")
                        try:
                            data = json.loads(line)
                            # Prioridade: termopar
                            if 'temperaturaTermopar' in data and data['temperaturaTermopar'] is not None:
                                with self.lock:
                                    self.latest_data = {
                                        'temperatura': data['temperaturaTermopar'],
                                        'umidade': None,
                                        'timestamp': datetime.now(timezone.utc).isoformat()
                                    }
                                print(f"Dados atualizados (termopar): {self.latest_data}")
                            elif 'temperaturaDHT' in data and data['temperaturaDHT'] is not None:
                                with self.lock:
                                    self.latest_data = {
                                        'temperatura': data['temperaturaDHT'],
                                        'umidade': data.get('umidade'),
                                        'timestamp': datetime.now(timezone.utc).isoformat()
                                    }
                                print(f"Dados atualizados (DHT): {self.latest_data}")
                            elif 'status' in data:
                                print(f"Comando executado: {data['status']}")
                        except json.JSONDecodeError:
                            print(f"Linha não JSON: {line}")
                time.sleep(0.1)
            except serial.SerialException as e:
                print(f"Erro na serial: {e}. Tentando reconectar...")
                if self.serial_conn:
                    self.serial_conn.close()
                    self.serial_conn = None
                time.sleep(5)
            except Exception as e:
                print(f"Erro inesperado: {e}")
                time.sleep(1)

    def stop(self):
        self.running = False
        with self.lock:
            if self.serial_conn and self.serial_conn.is_open:
                self.serial_conn.close()

    def get_latest(self):
        with self.lock:
            return self.latest_data.copy()

    def send_command(self, command):
        with self.lock:
            if self.serial_conn and self.serial_conn.is_open:
                try:
                    self.serial_conn.write(f"{command}\n".encode())
                    self.serial_conn.flush()
                    return True
                except Exception as e:
                    print(f"Erro ao enviar comando: {e}")
            return False

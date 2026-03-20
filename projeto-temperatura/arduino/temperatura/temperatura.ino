#include <max6675.h>
#include <Ethernet.h>
#include <SPI.h>

// ========== CONFIGURAÇÃO DOS PINOS ==========
// MAX6675 (Termopar) - CS alterado para pino 9
#define SO_PIN   12
#define CS_PIN   9      // Novo CS para o MAX6675 (evita conflito com Ethernet)
#define SCK_PIN  13
MAX6675 termopar(SCK_PIN, CS_PIN, SO_PIN);

// Atuador (LED, relé, etc.)
#define ACTUATOR_PIN 3

// ========== CONFIGURAÇÃO ETHERNET ==========
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  // MAC address fixo
IPAddress ip(192, 168, 0, 120);                        // IP fixo (ajuste conforme sua rede)
EthernetServer server(80);                              // Servidor web na porta 80

// ========== VARIÁVEIS GLOBAIS ==========
float ultimaTemperatura = 0;
bool atuadorEstado = false;  // false = desligado, true = ligado

// ========== SETUP ==========
void setup() {
  Serial.begin(9600);
  pinMode(ACTUATOR_PIN, OUTPUT);
  digitalWrite(ACTUATOR_PIN, LOW); // Atuador começa desligado

  // Inicia o Ethernet shield
  Ethernet.begin(mac, ip);
  server.begin();
  
  // Verifica se o Ethernet está ok
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield não encontrado!");
  } else if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Cabo Ethernet não conectado.");
  } else {
    Serial.print("Servidor web em: http://");
    Serial.println(Ethernet.localIP());
  }
  
  delay(1000);
}

// ========== LOOP PRINCIPAL ==========
void loop() {
  // 1. LEITURA DO SENSOR (a cada 2s, mas sem delay bloqueante)
  static unsigned long ultimaLeitura = 0;
  if (millis() - ultimaLeitura >= 2000) {
    ultimaLeitura = millis();
    float temperatura = termopar.readCelsius();
    
    // Armazena a última leitura válida
    if (!isnan(temperatura)) {
      ultimaTemperatura = temperatura;
    }
    
    // Envia para a serial (para o backend Flask)
    Serial.print("{\"temperaturaTermopar\":");
    if (isnan(temperatura)) {
      Serial.print("null");
    } else {
      Serial.print(temperatura, 1);
    }
    Serial.println("}");
  }

  // 2. PROCESSAMENTO DE COMANDOS VIA SERIAL (backend)
  if (Serial.available()) {
    String comando = Serial.readStringUntil('\n');
    comando.trim();
    if (comando == "LIGAR") {
      digitalWrite(ACTUATOR_PIN, HIGH);
      atuadorEstado = true;
      Serial.println("{\"status\":\"LIGADO\"}");
    } else if (comando == "DESLIGAR") {
      digitalWrite(ACTUATOR_PIN, LOW);
      atuadorEstado = false;
      Serial.println("{\"status\":\"DESLIGADO\"}");
    }
  }

  // 3. ATENDIMENTO A REQUISIÇÕES WEB
  EthernetClient client = server.available();
  if (client) {
    // Aguarda dados do cliente por até 100ms
    unsigned long timeout = millis() + 100;
    while (!client.available() && millis() < timeout) {
      delay(1);
    }
    
    // Se não houver requisição, encerra
    if (!client.available()) {
      client.stop();
      return;
    }
    
    // Lê a primeira linha da requisição (ex: "GET / HTTP/1.1")
    String request = client.readStringUntil('\r');
    client.flush();  // limpa o restante
    
    // Processa comandos via GET (ex: http://ip/?cmd=ligar)
    if (request.indexOf("GET /?cmd=ligar") >= 0) {
      digitalWrite(ACTUATOR_PIN, HIGH);
      atuadorEstado = true;
    } else if (request.indexOf("GET /?cmd=desligar") >= 0) {
      digitalWrite(ACTUATOR_PIN, LOW);
      atuadorEstado = false;
    }
    
    // Envia resposta HTML
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html; charset=UTF-8");
    client.println("Connection: close");
    client.println();
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
    client.println("<head><meta http-equiv='refresh' content='5'><title>Controle de Temperatura</title></head>");
    client.println("<body>");
    client.println("<h1>Monitoramento Local</h1>");
    client.print("<p><strong>Temperatura:</strong> ");
    client.print(ultimaTemperatura, 1);
    client.println(" °C</p>");
    client.print("<p><strong>Atuador:</strong> ");
    client.print(atuadorEstado ? "LIGADO" : "DESLIGADO");
    client.println("</p>");
    client.println("<p><a href='/?cmd=ligar'>Ligar Atuador</a></p>");
    client.println("<p><a href='/?cmd=desligar'>Desligar Atuador</a></p>");
    client.println("</body>");
    client.println("</html>");
    
    delay(10);
    client.stop();
  }
}

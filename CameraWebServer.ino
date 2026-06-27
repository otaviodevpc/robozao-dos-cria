#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include "esp_camera.h"
#include "index_html.h"   // interface tatica em PROGMEM (INDEX_HTML)

// ==========================================
// CONFIGURAÇÕES DE REDE (HOTSPOT AP)
// ==========================================
const char* ssid = "ROBO_MILITAR_AP";
const char* password = "senha_secreta";
IPAddress local_ip(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

// ==========================================
// CONFIGURAÇÕES DE HARDWARE (PINOS)
// ==========================================
// Motores (L298N)
// NOTA: GPIO 12 (MTDI) é strapping pin — se estiver HIGH no boot o ESP32-CAM
// pode não inicializar. Trocado por GPIO 2. Evite usar 12/15 em saidas que
// fiquem HIGH durante o boot.
#define MOTOR_ESQ_IN1 2
#define MOTOR_ESQ_IN2 13
#define MOTOR_DIR_IN3 14
#define MOTOR_DIR_IN4 15

// Flash LED
#define PIN_LANTERNA 4
bool lanternaLigada = false;

// Pinos da Câmera (Modelo AI-Thinker)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// ==========================================
// CONSTANTES DE TEMPO PARA GIROS (CALIBRAÇÃO)
// ==========================================
// Ajuste estes valores (em milissegundos) conforme a bateria e o piso
const int TEMPO_45_GRAUS = 400;
const int TEMPO_90_GRAUS = 800;
const int TEMPO_180_GRAUS = 1600;

// ==========================================
// INSTÂNCIAS DOS SERVIDORES
// ==========================================
WebServer server(80);           // Servidor para a página Web
WebServer streamServer(81);     // Servidor exclusivo para o Vídeo
WebSocketsServer webSocket = WebSocketsServer(82); // Servidor de Comandos

// ==========================================
// PÁGINA HTML DA INTERFACE MILITAR
// ==========================================
// O HTML/CSS/JS completo está em "index_html.h" (const INDEX_HTML[] PROGMEM).
// O JS conecta sozinho em ws://<host>:82/ e busca o video em :81/stream.

// ==========================================
// CONTROLE DOS MOTORES
// ==========================================
void pararMotores() {
  digitalWrite(MOTOR_ESQ_IN1, LOW); digitalWrite(MOTOR_ESQ_IN2, LOW);
  digitalWrite(MOTOR_DIR_IN3, LOW); digitalWrite(MOTOR_DIR_IN4, LOW);
}

void moverFrente() {
  digitalWrite(MOTOR_ESQ_IN1, HIGH); digitalWrite(MOTOR_ESQ_IN2, LOW);
  digitalWrite(MOTOR_DIR_IN3, HIGH); digitalWrite(MOTOR_DIR_IN4, LOW);
}

void moverTras() {
  digitalWrite(MOTOR_ESQ_IN1, LOW); digitalWrite(MOTOR_ESQ_IN2, HIGH);
  digitalWrite(MOTOR_DIR_IN3, LOW); digitalWrite(MOTOR_DIR_IN4, HIGH);
}

void girarEsquerda() {
  digitalWrite(MOTOR_ESQ_IN1, LOW); digitalWrite(MOTOR_ESQ_IN2, HIGH); // Esq para trás
  digitalWrite(MOTOR_DIR_IN3, HIGH); digitalWrite(MOTOR_DIR_IN4, LOW); // Dir para frente
}

void girarDireita() {
  digitalWrite(MOTOR_ESQ_IN1, HIGH); digitalWrite(MOTOR_ESQ_IN2, LOW); // Esq para frente
  digitalWrite(MOTOR_DIR_IN3, LOW); digitalWrite(MOTOR_DIR_IN4, HIGH); // Dir para trás
}

void realizarGiroSimulado(int tempo) {
  girarDireita(); // Adota giro para a direita por padrão no botão de giro rápido
  delay(tempo);
  pararMotores();
}

// ==========================================
// HANDLER DO WEBSOCKET (RECEBENDO COMANDOS)
// ==========================================
void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_TEXT) {
    String cmd = (char*)payload;
    
    // Movimentação Contínua (Joystick)
    if (cmd == "F") moverFrente();
    else if (cmd == "B") moverTras();
    else if (cmd == "L") girarEsquerda();
    else if (cmd == "R") girarDireita();
    else if (cmd == "S") pararMotores();
    
    // Funções Especiais
    else if (cmd == "LED") {
      lanternaLigada = !lanternaLigada;
      digitalWrite(PIN_LANTERNA, lanternaLigada ? HIGH : LOW);
    }
    else if (cmd == "T45") realizarGiroSimulado(TEMPO_45_GRAUS);
    else if (cmd == "T90") realizarGiroSimulado(TEMPO_90_GRAUS);
    else if (cmd == "T180") realizarGiroSimulado(TEMPO_180_GRAUS);
  }
}

// ==========================================
// HANDLER DO STREAM DE VÍDEO
// ==========================================
void stream_handler() {
  WiFiClient client = streamServer.client();

  // Cabecalho HTTP + multipart, escrito direto no socket (sem WebServer.send,
  // que quebra o boundary do MJPEG no ESP32).
  String head = "HTTP/1.1 200 OK\r\n";
  head += "Access-Control-Allow-Origin: *\r\n";
  head += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
  client.print(head);

  while (client.connected()) {
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Falha na captura da camera");
      continue;
    }

    client.print("--frame\r\n");
    client.print("Content-Type: image/jpeg\r\n");
    client.printf("Content-Length: %u\r\n\r\n", fb->len);
    client.write(fb->buf, fb->len);
    client.print("\r\n");

    esp_camera_fb_return(fb);

    // cede o processador entre frames: evita estourar o watchdog e deixa os
    // outros servidores (web/websocket) respirarem.
    yield();
  }
}

// ==========================================
// CONFIGURAÇÃO INICIAL (SETUP)
// ==========================================
void setup() {
  Serial.begin(115200);

  // Configuração dos Pinos dos Motores e LED
  pinMode(MOTOR_ESQ_IN1, OUTPUT);
  pinMode(MOTOR_ESQ_IN2, OUTPUT);
  pinMode(MOTOR_DIR_IN3, OUTPUT);
  pinMode(MOTOR_DIR_IN4, OUTPUT);
  pinMode(PIN_LANTERNA, OUTPUT);
  pararMotores();

  // Configuração da Câmera
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  // Resolução: CIF (400x296) é ideal para fluidez no modo AP. SVGA (800x600) para qualidade.
  config.frame_size = FRAMESIZE_CIF;
  config.jpeg_quality = 12; // Menor = melhor qualidade (10 a 63)
  config.fb_count = 2; // 2 buffers para maior taxa de quadros (FPS)

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Erro ao iniciar a câmera!");
    return;
  }

  // Configuração do Wi-Fi (Modo Access Point com IP Fixo)
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.softAP(ssid, password);
  Serial.print("Access Point criado! Conecte-se e acesse: http://");
  Serial.println(WiFi.softAPIP());

  // Rotas do Servidor Web (Porta 80)
  server.on("/", []() {
    server.send_P(200, "text/html", INDEX_HTML);
  });
  server.begin();

  // Rota do Servidor de Vídeo (Porta 81)
  streamServer.on("/stream", stream_handler);
  streamServer.begin();

  // Inicialização do WebSocket (Porta 82)
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
}

// ==========================================
// LOOP PRINCIPAL
// ==========================================
void loop() {
  server.handleClient();       // Mantém a página web viva
  streamServer.handleClient(); // Mantém o stream de vídeo vivo
  webSocket.loop();            // Escuta ativamente os comandos do joystick
}
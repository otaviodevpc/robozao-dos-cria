// =========================================================================
//  TESTE COMPLETO - ROBO ESTEIRA (ESP32-CAM)
//  Valida TUDO junto, mas ainda simples: CAMERA + LED + MOVIMENTO.
//
//  - Video: stream MJPEG na porta 81 (aparece no topo da pagina).
//  - Movimento: botoes FRENTE / RE / ESQ / DIR / PARAR (HTTP simples).
//  - LED (flash, GPIO 4): botao liga/desliga.
//
//  HTTP em vez de WebSocket de proposito: e mais simples e robusto pra
//  teste de campo. Segura o botao = anda; solta = para.
//
//  IMPORTANTE: arquivo todo em ASCII (sem acentos/emoji) de proposito,
//  pra nao quebrar a raw string R"HTML(...)" se o editor/OneDrive mexer
//  no encoding do arquivo.
//
//  Veja TesteBasico/INSTRUCOES.md para o passo a passo e a tabela de feedback.
// =========================================================================

#include <WiFi.h>
#include <WebServer.h>
#include "esp_camera.h"

// ---- Rede (Access Point: o ESP cria o proprio Wi-Fi) -------------------
const char* ssid     = "ROBO_TESTE";
const char* password = "12345678";     // min. 8 caracteres
IPAddress local_ip(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

// ---- Pinos da ponte H L298N (iguais ao codigo definitivo) --------------
//  ESQUERDO: IN1=14, IN2=2     DIREITO: IN3=13, IN4=15
#define MOTOR_ESQ_IN1 14
#define MOTOR_ESQ_IN2  2
#define MOTOR_DIR_IN3 13
#define MOTOR_DIR_IN4 15

// ---- LED de flash ------------------------------------------------------
#define PIN_LED 4
bool ledLigado = false;

// ---- Pinos da Camera (Modelo AI-Thinker) -------------------------------
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

WebServer server(80);         // pagina + comandos
WebServer streamServer(81);   // video MJPEG

// =========================================================================
//  MOVIMENTO
// =========================================================================
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
  digitalWrite(MOTOR_ESQ_IN1, LOW); digitalWrite(MOTOR_ESQ_IN2, HIGH);
  digitalWrite(MOTOR_DIR_IN3, HIGH); digitalWrite(MOTOR_DIR_IN4, LOW);
}
void girarDireita() {
  digitalWrite(MOTOR_ESQ_IN1, HIGH); digitalWrite(MOTOR_ESQ_IN2, LOW);
  digitalWrite(MOTOR_DIR_IN3, LOW); digitalWrite(MOTOR_DIR_IN4, HIGH);
}

// =========================================================================
//  PAGINA HTML (video + botoes grandes)
// =========================================================================
const char PAGINA[] PROGMEM = R"HTML(
<!DOCTYPE html><html lang="pt-BR"><head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no">
<title>TESTE ROBO</title>
<style>
  *{box-sizing:border-box;-webkit-tap-highlight-color:transparent;}
  body{margin:0;font-family:system-ui,Arial,sans-serif;background:#0b0f10;color:#cfe;
       text-align:center;padding:12px;user-select:none;}
  h1{font-size:17px;margin:4px 0 2px;}
  .sub{font-size:12px;color:#7a9;margin-bottom:10px;}
  #cam{width:100%;max-width:420px;aspect-ratio:4/3;background:#000;border:2px solid #16323a;
       border-radius:10px;object-fit:cover;}
  .grid{display:grid;grid-template-columns:repeat(3,1fr);gap:10px;max-width:420px;margin:12px auto 0;}
  button{font-size:19px;font-weight:700;padding:20px 0;border:none;border-radius:14px;
         background:#16323a;color:#bfe;cursor:pointer;}
  button:active{background:#2bd673;color:#000;}
  .stop{background:#5a1820;color:#fdd;}
  .stop:active{background:#ff4d4d;color:#000;}
  .wide{grid-column:1 / -1;}
  .led{background:#2a2410;color:#fd6;}
  .led.on{background:#ffaa00;color:#000;}
  #log{margin-top:10px;font-size:13px;color:#9fd;min-height:18px;}
</style></head><body>
  <h1>TESTE ROBO - CAMERA + MOVIMENTO</h1>
  <div class="sub">O video deve aparecer abaixo. Segure um botao pra andar.</div>

  <img id="cam" alt="VIDEO">

  <div class="grid">
    <div></div>
    <button ontouchstart="cmd('F')" onmousedown="cmd('F')">FRENTE</button>
    <div></div>

    <button ontouchstart="cmd('L')" onmousedown="cmd('L')">ESQ</button>
    <button class="stop" onclick="cmd('S')">PARAR</button>
    <button ontouchstart="cmd('R')" onmousedown="cmd('R')">DIR</button>

    <div></div>
    <button ontouchstart="cmd('B')" onmousedown="cmd('B')">RE</button>
    <div></div>

    <button id="btnled" class="wide led" onclick="toggleLed()">LIGAR / DESLIGAR LED</button>
  </div>

  <div id="log">conectando video...</div>

<script>
  var HOST = location.hostname || "192.168.4.1";

  // ---- video (stream MJPEG na porta 81) ----
  var cam = document.getElementById('cam');
  function startCam(){ cam.src = "http://" + HOST + ":81/stream?t=" + Date.now(); }
  cam.onload  = function(){ log('video OK'); };
  cam.onerror = function(){ log('video falhou - tentando de novo'); setTimeout(startCam, 1500); };
  startCam();

  // ---- comandos ----
  function cmd(c){
    fetch('/'+c).then(function(){ log('OK: '+c); }).catch(function(){ log('FALHOU: '+c); });
  }
  function parar(){ cmd('S'); }

  // ---- LED ----
  var ledOn = false;
  function toggleLed(){
    fetch('/LED').then(function(){
      ledOn = !ledOn;
      document.getElementById('btnled').classList.toggle('on', ledOn);
      log('LED: ' + (ledOn ? 'ON' : 'OFF'));
    }).catch(function(){ log('FALHOU: LED'); });
  }

  function log(t){ document.getElementById('log').textContent = t; }

  // ao soltar um botao de direcao, manda PARAR
  document.querySelectorAll('.grid button').forEach(function(b){
    if(b.classList.contains('stop') || b.id === 'btnled') return;
    b.addEventListener('touchend', parar);
    b.addEventListener('mouseup', parar);
    b.addEventListener('mouseleave', function(e){ if(e.buttons) parar(); });
  });
</script>
</body></html>
)HTML";

// =========================================================================
//  STREAM DE VIDEO (porta 81)
// =========================================================================
void stream_handler() {
  WiFiClient client = streamServer.client();

  String head = "HTTP/1.1 200 OK\r\n";
  head += "Access-Control-Allow-Origin: *\r\n";
  head += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
  client.print(head);

  while (client.connected()) {
    // mantem a pagina/comandos respondendo enquanto transmite video
    server.handleClient();

    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) { Serial.println("Falha na captura da camera"); delay(20); continue; }

    bool ok = true;
    ok &= client.print("--frame\r\n") > 0;
    ok &= client.print("Content-Type: image/jpeg\r\n") > 0;
    ok &= client.printf("Content-Length: %u\r\n\r\n", fb->len) > 0;
    ok &= client.write(fb->buf, fb->len) == fb->len;
    ok &= client.print("\r\n") > 0;

    esp_camera_fb_return(fb);
    if (!ok) break;
    delay(20);
  }
}

// =========================================================================
//  ROTAS DE COMANDO
// =========================================================================
void rotaComando(const char* nome, void (*acao)()) {
  acao();
  server.send(200, "text/plain", nome);
  Serial.print("CMD: "); Serial.println(nome);
}

void setup() {
  Serial.begin(115200);

  // Pinos dos motores: OUTPUT + LOW logo de cara (GPIO 2 e 15 sao strapping).
  pinMode(MOTOR_ESQ_IN1, OUTPUT);
  pinMode(MOTOR_ESQ_IN2, OUTPUT);
  pinMode(MOTOR_DIR_IN3, OUTPUT);
  pinMode(MOTOR_DIR_IN4, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  pararMotores();
  digitalWrite(PIN_LED, LOW);

  // ---- Camera (config padrao AI-Thinker, igual ao codigo definitivo) ----
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;   config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM; config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM; config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;   config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size   = FRAMESIZE_CIF;  // 400x296 - leve, bom pra AP
  config.jpeg_quality = 12;             // menor = melhor qualidade (10..63)
  config.fb_count     = 2;

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("ERRO ao iniciar a camera! (segue sem video; motores ainda funcionam)");
  } else {
    Serial.println("Camera OK.");
  }

  // ---- Wi-Fi (Access Point) ----
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.softAP(ssid, password);
  Serial.println();
  Serial.print("AP no ar! Conecte no Wi-Fi \"");
  Serial.print(ssid);
  Serial.print("\" (senha "); Serial.print(password);
  Serial.print(") e abra: http://"); Serial.println(WiFi.softAPIP());

  // ---- Rotas ----
  server.on("/", []() { server.send_P(200, "text/html", PAGINA); });
  server.on("/F", []() { rotaComando("F (frente)",   moverFrente);   });
  server.on("/B", []() { rotaComando("B (re)",        moverTras);     });
  server.on("/L", []() { rotaComando("L (esquerda)",  girarEsquerda); });
  server.on("/R", []() { rotaComando("R (direita)",   girarDireita);  });
  server.on("/S", []() { rotaComando("S (parar)",     pararMotores);  });
  server.on("/LED", []() {
    ledLigado = !ledLigado;
    digitalWrite(PIN_LED, ledLigado ? HIGH : LOW);
    server.send(200, "text/plain", ledLigado ? "LED ON" : "LED OFF");
    Serial.print("CMD: LED "); Serial.println(ledLigado ? "ON" : "OFF");
  });
  server.begin();

  streamServer.on("/stream", stream_handler);
  streamServer.begin();

  Serial.println("Servidores HTTP iniciados (pagina :80, video :81).");
}

void loop() {
  server.handleClient();
  streamServer.handleClient();
}

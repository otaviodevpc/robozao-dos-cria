#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include "esp_camera.h"
#include <pgmspace.h>

// =========================================================================
// HTML, CSS e JS (Interface Web Embutida Diretamente no Código)
// =========================================================================
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
<title>UNIT-DX9 // TACTICAL CONTROL</title>
<style>
  :root{
    --neon:#39ff14; --neon-dim:#1f8a0c; --bg:#030508; --panel:#0a1218;
    --border:#143018; --text-dim:#4a6850; --red:#ff3b3b; --amber:#ffaa00;
    --mono:'Share Tech Mono','Courier New',monospace;
  }
  *{box-sizing:border-box;margin:0;padding:0;-webkit-tap-highlight-color:transparent;}
  html,body{height:100%;}
  body{
    background:var(--bg); color:var(--neon); font-family:var(--mono);
    overflow:hidden; user-select:none; touch-action:none;
    display:flex; flex-direction:column;
  }
  body::before{
    content:''; position:fixed; inset:0; z-index:0; pointer-events:none;
    background-image:linear-gradient(rgba(57,255,20,0.04) 1px,transparent 1px),
                     linear-gradient(90deg,rgba(57,255,20,0.04) 1px,transparent 1px);
    background-size:40px 40px;
  }
  /* HEADER */
  header{
    display:flex; align-items:center; justify-content:space-between;
    padding:8px 14px; border-bottom:1px solid var(--border);
    background:rgba(3,5,8,0.9); z-index:2; font-size:11px; letter-spacing:0.15em;
  }
  .logo{font-weight:700; color:var(--neon); display:flex; align-items:center; gap:8px;}
  .logo .box{width:22px;height:22px;border:1px solid var(--neon);display:grid;place-items:center;font-size:9px;background:rgba(57,255,20,0.1);}
  .conn{display:flex; align-items:center; gap:6px; font-size:10px;}
  .dot{width:8px;height:8px;border-radius:50%;background:var(--red);box-shadow:0 0 6px var(--red);transition:.3s;}
  .dot.on{background:var(--neon);box-shadow:0 0 8px var(--neon);}
  /* MAIN */
  main{flex:1; display:flex; flex-direction:column; align-items:center; justify-content:flex-start; gap:14px; padding:14px; z-index:1; overflow-y:auto;}
  /* VIDEO VIEWPORT */
  .viewport{
    position:relative; width:100%; max-width:520px; aspect-ratio:4/3;
    border:1px solid var(--neon-dim); background:#000; overflow:hidden;
    clip-path:polygon(0 12px,12px 0,calc(100% - 12px) 0,100% 12px,100% calc(100% - 12px),calc(100% - 12px) 100%,12px 100%,0 calc(100% - 12px));
  }
  .viewport img{width:100%;height:100%;object-fit:cover;display:block;}
  .viewport .corner{position:absolute;width:24px;height:24px;border:2px solid var(--neon);filter:drop-shadow(0 0 4px var(--neon));}
  .viewport .corner.tl{top:6px;left:6px;border-right:none;border-bottom:none;}
  .viewport .corner.tr{top:6px;right:6px;border-left:none;border-bottom:none;}
  .viewport .corner.bl{bottom:6px;left:6px;border-right:none;border-top:none;}
  .viewport .corner.br{bottom:6px;right:6px;border-left:none;border-top:none;}
  .viewport .tag{position:absolute;font-size:9px;color:var(--neon);letter-spacing:.15em;background:rgba(3,5,8,.6);padding:2px 5px;}
  .viewport .tag.live{top:10px;left:10px;color:var(--red);}
  .viewport .tag.cmd{bottom:10px;right:10px;}
  .reticle{position:absolute;top:50%;left:50%;width:40px;height:40px;transform:translate(-50%,-50%);border:1px solid rgba(57,255,20,.4);pointer-events:none;}
  .reticle::before,.reticle::after{content:'';position:absolute;background:rgba(57,255,20,.4);}
  .reticle::before{top:50%;left:-10px;right:-10px;height:1px;}
  .reticle::after{left:50%;top:-10px;bottom:-10px;width:1px;}
  /* CONTROL ROW */
  .controls{width:100%;max-width:520px;display:flex;gap:14px;align-items:center;justify-content:space-between;flex-wrap:wrap;}
  /* JOYSTICK */
  .joy-wrap{display:flex;flex-direction:column;align-items:center;gap:6px;}
  #joy{
    width:170px;height:170px;border-radius:50%;
    background:radial-gradient(circle,rgba(57,255,20,.06),transparent 70%);
    border:1px solid var(--neon-dim);position:relative;touch-action:none;
  }
  #joy .ring{position:absolute;inset:0;border-radius:50%;border:1px dashed rgba(57,255,20,.15);}
  #knob{
    position:absolute;width:58px;height:58px;border-radius:50%;
    background:radial-gradient(circle,var(--neon),var(--neon-dim));
    box-shadow:0 0 14px var(--neon);left:50%;top:50%;
    transform:translate(-50%,-50%);transition:transform .05s linear;
  }
  .joy-label{font-size:9px;color:var(--text-dim);letter-spacing:.2em;}
  .dirbig{font-size:22px;font-weight:700;color:var(--neon);min-height:26px;text-shadow:0 0 10px var(--neon);}
  /* UTILITY BUTTONS */
  .utils{display:flex;flex-direction:column;gap:10px;}
  .btn{
    font-family:var(--mono);font-size:12px;letter-spacing:.1em;color:var(--neon);
    background:var(--panel);border:1px solid var(--neon-dim);padding:12px 16px;
    cursor:pointer;min-width:96px;text-align:center;transition:.12s;
    clip-path:polygon(0 0,calc(100% - 7px) 0,100% 7px,100% 100%,7px 100%,0 calc(100% - 7px));
  }
  .btn:active{background:var(--neon);color:#000;box-shadow:0 0 12px var(--neon);}
  .btn.led.on{background:var(--amber);color:#000;border-color:var(--amber);box-shadow:0 0 12px var(--amber);}
  .turn-row{display:flex;gap:8px;}
  .turn-row .btn{min-width:0;flex:1;padding:12px 8px;}
  footer{padding:6px;text-align:center;font-size:9px;color:var(--text-dim);letter-spacing:.2em;border-top:1px solid var(--border);z-index:2;}
</style>
</head>
<body>
<header>
  <div class="logo"><div class="box">DX</div>UNIT-DX9 CONTROL</div>
  <div class="conn"><span>LINK</span><span class="dot" id="dot"></span><span id="conn-txt">OFFLINE</span></div>
</header>

<main>
  <div class="viewport">
    <img id="stream" src="" alt="VIDEO FEED">
    <span class="corner tl"></span><span class="corner tr"></span>
    <span class="corner bl"></span><span class="corner br"></span>
    <div class="reticle"></div>
    <span class="tag live">● REC // LIVE</span>
    <span class="tag cmd" id="last-cmd">CMD: S</span>
  </div>

  <div class="dirbig" id="dirbig">STOP</div>

  <div class="controls">
    <div class="joy-wrap">
      <div id="joy"><div class="ring"></div><div id="knob"></div></div>
      <div class="joy-label">DRIVE CONTROL</div>
    </div>

    <div class="utils">
      <button class="btn led" id="btn-led">◉ FLASH</button>
      <div class="turn-row">
        <button class="btn" data-cmd="T45">45°</button>
        <button class="btn" data-cmd="T90">90°</button>
      </div>
      <button class="btn" data-cmd="T180">↺ 180°</button>
    </div>
  </div>
</main>

<footer>UNIT-DX9 // AP 192.168.4.1 // SEC-LVL ALPHA</footer>

<script>
(function(){
  "use strict";
  var HOST = window.location.hostname || "192.168.4.1";
  var STREAM_URL = "http://" + HOST + ":81/stream";
  var WS_URL = "ws://" + HOST + ":82/";

  // ---- VIDEO STREAM ----
  var img = document.getElementById("stream");
  img.src = STREAM_URL;
  img.onerror = function(){ setTimeout(function(){ img.src = STREAM_URL + "?t=" + Date.now(); }, 1500); };

  // ---- WEBSOCKET (robust + auto-reconnect) ----
  var ws = null, wsReady = false;
  var dot = document.getElementById("dot");
  var connTxt = document.getElementById("conn-txt");

  function connect(){
    ws = new WebSocket(WS_URL);
    ws.onopen = function(){ wsReady = true; dot.classList.add("on"); connTxt.textContent = "ONLINE"; };
    ws.onclose = function(){ wsReady = false; dot.classList.remove("on"); connTxt.textContent = "OFFLINE"; setTimeout(connect, 1500); };
    ws.onerror = function(){ try{ ws.close(); }catch(e){} };
  }
  connect();

  function send(cmd){
    if(wsReady && ws.readyState === 1){ ws.send(cmd); }
  }

  // ---- COMMAND STATE (only send on change) ----
  var lastDir = "S";
  var lastCmdEl = document.getElementById("last-cmd");
  var dirBig = document.getElementById("dirbig");
  var DIR_NAME = {F:"FORWARD", B:"BACKWARD", L:"LEFT", R:"RIGHT", S:"STOP"};

  function setDir(cmd){
    if(cmd === lastDir) return;       // <-- anti-flood: só envia quando muda
    lastDir = cmd;
    send(cmd);
    lastCmdEl.textContent = "CMD: " + cmd;
    dirBig.textContent = DIR_NAME[cmd];
  }

  // ---- VIRTUAL JOYSTICK (touch + mouse) ----
  var joy = document.getElementById("joy");
  var knob = document.getElementById("knob");
  var dragging = false;
  var DEAD_ZONE = 0.28;   // fração do raio antes de ativar
  var MAX = 60;           // deslocamento máx do knob em px

  function center(){ var r = joy.getBoundingClientRect(); return {x:r.left + r.width/2, y:r.top + r.height/2}; }

  function handle(px, py){
    var c = center();
    var dx = px - c.x, dy = py - c.y;
    var dist = Math.sqrt(dx*dx + dy*dy);
    var radius = joy.getBoundingClientRect().width/2;
    var norm = Math.min(dist / radius, 1);

    // limita o knob visualmente
    var kx = dx, ky = dy, capped = MAX;
    if(dist > capped){ var s = capped/dist; kx = dx*s; ky = dy*s; }
    knob.style.transform = "translate(calc(-50% + "+kx+"px), calc(-50% + "+ky+"px))";

    if(norm < DEAD_ZONE){ setDir("S"); return; }

    // angulo: 0 = direita, cresce horário (tela). Convertendo p/ direções
    var ang = Math.atan2(dy, dx) * 180 / Math.PI; // -180..180
    if(ang < 0) ang += 360;                        // 0..360
    // setores de 90° centrados em cada direção
    var cmd;
    if(ang >= 45 && ang < 135)      cmd = "B"; // baixo na tela = ré
    else if(ang >= 135 && ang < 225) cmd = "L";
    else if(ang >= 225 && ang < 315) cmd = "F"; // cima = frente
    else                             cmd = "R";
    setDir(cmd);
  }

  function release(){
    dragging = false;
    knob.style.transform = "translate(-50%,-50%)";
    setDir("S");
  }

  // touch
  joy.addEventListener("touchstart", function(e){ dragging = true; handle(e.touches[0].clientX, e.touches[0].clientY); e.preventDefault(); }, {passive:false});
  joy.addEventListener("touchmove", function(e){ if(dragging) handle(e.touches[0].clientX, e.touches[0].clientY); e.preventDefault(); }, {passive:false});
  joy.addEventListener("touchend", function(e){ release(); e.preventDefault(); }, {passive:false});
  joy.addEventListener("touchcancel", release, {passive:false});
  // mouse
  joy.addEventListener("mousedown", function(e){ dragging = true; handle(e.clientX, e.clientY); });
  window.addEventListener("mousemove", function(e){ if(dragging) handle(e.clientX, e.clientY); });
  window.addEventListener("mouseup", function(){ if(dragging) release(); });

  // ---- UTILITY BUTTONS ----
  var ledOn = false;
  var ledBtn = document.getElementById("btn-led");
  ledBtn.addEventListener("click", function(){ send("LED"); ledOn = !ledOn; ledBtn.classList.toggle("on", ledOn); });

  var turnBtns = document.querySelectorAll(".btn[data-cmd]");
  for(var i=0;i<turnBtns.length;i++){
    (function(b){ b.addEventListener("click", function(){ send(b.getAttribute("data-cmd")); }); })(turnBtns[i]);
  }
})();
</script>
</body>
</html>
)rawliteral";

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
/*
 * Ponte H L298N (robô lagarta) — ESP32-CAM AI-Thinker, sem cartão SD:
 *
 *   GPIO 14 → IN1 (motor esquerdo)     GPIO 15 → IN3 (motor direito)
 *   GPIO  2 → IN2 (motor esquerdo)     GPIO 13 → IN4 (motor direito)
 *   GND     → GND da L298N (comum com ESP32 e bateria dos motores)
 *
 * Evitar: GPIO 0 (câmera/upload), GPIO 4 (flash), GPIO 12 (strapping),
 *         GPIO 5,18,19,21-27,32,34-39 (câmera).
 *
 * Teste só dos motores (sem câmera/Wi-Fi): sketch RoboEsteira_Basico.
 *
 * IMPORTANTE: o motor DIREITO estava cabeado invertido — no comando "frente"
 * ele não girava e em "trás" os dois giravam. Em vez de mexer na lógica das
 * 4 funções de movimento, invertemos aqui os pinos IN3/IN4 do lado direito.
 * Assim os 4 sentidos ficam corretos de uma vez. (GPIO 15 continua sendo IN3
 * fisicamente; só a associação no software é que foi trocada.)
 */
#define MOTOR_ESQ_IN1 14
#define MOTOR_ESQ_IN2  2
#define MOTOR_DIR_IN3 13   // <- invertido (era GPIO 15) p/ corrigir cabeamento do motor direito
#define MOTOR_DIR_IN4 15   // <- invertido (era GPIO 13)

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
const int TEMPO_45_GRAUS = 400;
const int TEMPO_90_GRAUS = 800;
const int TEMPO_180_GRAUS = 1600;

// ==========================================
// INSTÂNCIAS DOS SERVIDORES
// ==========================================
WebServer server(80);           // Servidor HTTP para a página Web
WebServer streamServer(81);     // Servidor HTTP exclusivo para o Vídeo
WebSocketsServer webSocket = WebSocketsServer(82); // Servidor de Comandos (Baixa Latência)

// ==========================================
// CONTROLE DOS MOTORES (esteira / lagarta)
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
  digitalWrite(MOTOR_ESQ_IN1, LOW); digitalWrite(MOTOR_ESQ_IN2, HIGH);
  digitalWrite(MOTOR_DIR_IN3, HIGH); digitalWrite(MOTOR_DIR_IN4, LOW);
}

void girarDireita() {
  digitalWrite(MOTOR_ESQ_IN1, HIGH); digitalWrite(MOTOR_ESQ_IN2, LOW);
  digitalWrite(MOTOR_DIR_IN3, LOW); digitalWrite(MOTOR_DIR_IN4, HIGH);
}

void realizarGiroSimulado(int tempo) {
  girarDireita(); 
  delay(tempo);
  pararMotores();
}

// ==========================================
// HANDLER DO WEBSOCKET (COMANDOS)
// ==========================================
void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_DISCONNECTED) {
    pararMotores();
    return;
  }

  if (type == WStype_TEXT) {
    String cmd = (char*)payload;

    if      (cmd == "F")   moverFrente();
    else if (cmd == "B")   moverTras();
    else if (cmd == "L")   girarEsquerda();
    else if (cmd == "R")   girarDireita();
    else if (cmd == "S")   pararMotores();

    else if (cmd == "LED") {
      lanternaLigada = !lanternaLigada;
      digitalWrite(PIN_LANTERNA, lanternaLigada ? HIGH : LOW);
    }
    else if (cmd == "T45")  realizarGiroSimulado(TEMPO_45_GRAUS);
    else if (cmd == "T90")  realizarGiroSimulado(TEMPO_90_GRAUS);
    else if (cmd == "T180") realizarGiroSimulado(TEMPO_180_GRAUS);
  }
}

// ==========================================
// HANDLER DE VÍDEO (RODANDO NO LOOP PRINCIPAL)
// ==========================================
void stream_handler() {
  WiFiClient client = streamServer.client();

  String head = "HTTP/1.1 200 OK\r\n";
  head += "Access-Control-Allow-Origin: *\r\n";
  head += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
  client.print(head);

  while (client.connected()) {
    // ESSENCIAL: o stream é um loop infinito que prende o programa aqui dentro.
    // Sem rodar TODOS os servidores neste laço, o joystick (WebSocket :82) e a
    // própria página (:80) ficam sem resposta enquanto o vídeo transmite —
    // foi essa a causa do "joystick travado". webSocket.loop() em especial
    // precisa rodar a cada iteração para entregar os comandos de movimento.
    webSocket.loop();
    server.handleClient();

    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Falha na captura da camera");
      // sem 'continue' cego: cede o processador para não travar o watchdog
      // nem segurar o WebSocket caso a câmera engasgue.
      delay(20);
      continue;
    }

    bool ok = true;
    ok &= client.print("--frame\r\n") > 0;
    ok &= client.print("Content-Type: image/jpeg\r\n") > 0;
    ok &= client.printf("Content-Length: %u\r\n\r\n", fb->len) > 0;
    ok &= client.write(fb->buf, fb->len) == fb->len;
    ok &= client.print("\r\n") > 0;

    esp_camera_fb_return(fb);

    // se a escrita falhou, o cliente caiu: sai já em vez de rodar o loop todo
    if (!ok) break;

    // cede o processador entre frames (mantém WebSocket/HTTP responsivos)
    delay(20);
  }
}

// ==========================================
// CONFIGURAÇÃO INICIAL (SETUP)
// ==========================================
void setup() {
  Serial.begin(115200);

  // Pinos dos Motores como saída e já em LOW.
  // GPIO 2 e GPIO 15 são strapping pins: se ficarem flutuando/HIGH no boot
  // podem gerar ruído nos motores e até atrapalhar o upload. Definir OUTPUT +
  // LOW logo de cara deixa a ponte H em estado conhecido (parado).
  pinMode(MOTOR_ESQ_IN1, OUTPUT);
  pinMode(MOTOR_ESQ_IN2, OUTPUT);
  pinMode(MOTOR_DIR_IN3, OUTPUT);
  pinMode(MOTOR_DIR_IN4, OUTPUT);
  pinMode(PIN_LANTERNA, OUTPUT);
  pararMotores();
  digitalWrite(PIN_LANTERNA, LOW);

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
  
  // CIF é o ideal para fluidez e estabilidade em AP
  config.frame_size = FRAMESIZE_CIF;
  config.jpeg_quality = 12; 
  config.fb_count = 2; 

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Erro ao iniciar a câmera!");
    return;
  }

  // Configuração Wi-Fi (Modo AP Fixo)
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.softAP(ssid, password);
  Serial.print("AP Criado. IP da Interface: http://");
  Serial.println(WiFi.softAPIP());

  // Rotas HTTP
  server.on("/", []() {
    server.send_P(200, "text/html", INDEX_HTML);
  });
  server.begin();

  streamServer.on("/stream", stream_handler);
  streamServer.begin();

  // Websocket na Porta 82
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
}

// ==========================================
// LOOP PRINCIPAL
// ==========================================
void loop() {
  server.handleClient();       // Interface Web
  streamServer.handleClient(); // Vídeo
  webSocket.loop();            // Joystick
}
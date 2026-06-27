// =========================================================================
//  TESTE BÁSICO — ROBÔ ESTEIRA (ESP32-CAM)
//  Objetivo: isolar o problema. SEM câmera, SEM vídeo, SEM WebSocket.
//  Só Wi-Fi (Access Point) + página com botões grandes + LED de confirmação.
//
//  Como funciona:
//   - Cada botão faz uma requisição HTTP simples (GET /F, /B, /L, /R, /S...).
//     HTTP é mais "burro" e confiável que WebSocket — bom para teste de campo.
//   - A CADA comando recebido o LED (flash, GPIO 4) PISCA uma vez.
//     => Se o LED pisca quando você aperta o botão, o comando CHEGOU no ESP32.
//        Aí, se o motor não anda, o problema é na ponte H / fiação / bateria.
//     => Se o LED NÃO pisca, o comando não chegou (problema de Wi-Fi / rede).
//
//  Veja TesteBasico/INSTRUCOES.md para o passo a passo e a tabela de feedback.
// =========================================================================

#include <WiFi.h>
#include <WebServer.h>

// ---- Rede (Access Point: o ESP cria o próprio Wi-Fi) -------------------
const char* ssid     = "ROBO_TESTE";   // nome da rede que vai aparecer no celular
const char* password = "12345678";     // senha (mín. 8 caracteres)
IPAddress local_ip(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

// ---- Pinos da ponte H L298N (mesmos do código definitivo) --------------
//  ESQUERDO: IN1=14, IN2=2     DIREITO: IN3=13, IN4=15
//  (IN3/IN4 já estão na ordem corrigida do motor direito)
#define MOTOR_ESQ_IN1 14
#define MOTOR_ESQ_IN2  2
#define MOTOR_DIR_IN3 13
#define MOTOR_DIR_IN4 15

// ---- LED de flash (confirmação visual) ---------------------------------
#define PIN_LED 4

WebServer server(80);

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

// Testa UM motor de cada vez (ajuda a achar fiação invertida)
void soEsquerdoFrente() {
  digitalWrite(MOTOR_ESQ_IN1, HIGH); digitalWrite(MOTOR_ESQ_IN2, LOW);
  digitalWrite(MOTOR_DIR_IN3, LOW);  digitalWrite(MOTOR_DIR_IN4, LOW);
}
void soDireitoFrente() {
  digitalWrite(MOTOR_ESQ_IN1, LOW);  digitalWrite(MOTOR_ESQ_IN2, LOW);
  digitalWrite(MOTOR_DIR_IN3, HIGH); digitalWrite(MOTOR_DIR_IN4, LOW);
}

// Pisca o LED rápido: confirma que o comando chegou no ESP32
void piscarLed() {
  digitalWrite(PIN_LED, HIGH);
  delay(60);
  digitalWrite(PIN_LED, LOW);
}

// =========================================================================
//  PÁGINA HTML (botões grandes, leve, sem nada externo)
// =========================================================================
const char PAGINA[] PROGMEM = R"HTML(
<!DOCTYPE html><html lang="pt-BR"><head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no">
<title>TESTE ROBO</title>
<style>
  *{box-sizing:border-box;-webkit-tap-highlight-color:transparent;}
  body{margin:0;font-family:system-ui,Arial,sans-serif;background:#0b0f10;color:#cfe;
       text-align:center;padding:14px;user-select:none;}
  h1{font-size:18px;margin:6px 0 2px;}
  .sub{font-size:12px;color:#7a9;margin-bottom:14px;}
  .grid{display:grid;grid-template-columns:repeat(3,1fr);gap:10px;max-width:420px;margin:0 auto;}
  button{font-size:20px;font-weight:700;padding:22px 0;border:none;border-radius:14px;
         background:#16323a;color:#bfe;cursor:pointer;}
  button:active{background:#2bd673;color:#000;}
  .stop{background:#5a1820;color:#fdd;}
  .stop:active{background:#ff4d4d;color:#000;}
  .wide{grid-column:1 / -1;}
  .sec{margin-top:18px;font-size:12px;color:#7a9;}
  .extra button{font-size:15px;padding:16px 0;background:#23323a;}
  #log{margin-top:14px;font-size:13px;color:#9fd;min-height:20px;}
  .led{font-size:12px;color:#fa0;margin-top:6px;}
</style></head><body>
  <h1>TESTE ROBÔ — BÁSICO</h1>
  <div class="sub">Aperte um botão e observe o robô + o LED do ESP</div>

  <div class="grid">
    <div></div>
    <button ontouchstart="cmd('F')" onmousedown="cmd('F')">▲<br>FRENTE</button>
    <div></div>

    <button ontouchstart="cmd('L')" onmousedown="cmd('L')">◄<br>ESQ</button>
    <button class="stop" onclick="cmd('S')">■<br>PARAR</button>
    <button ontouchstart="cmd('R')" onmousedown="cmd('R')">►<br>DIR</button>

    <div></div>
    <button ontouchstart="cmd('B')" onmousedown="cmd('B')">▼<br>RÉ</button>
    <div></div>
  </div>

  <div class="sec">TESTES INDIVIDUAIS (achar fiação invertida)</div>
  <div class="grid extra">
    <button class="wide" onclick="cmd('ME')">SÓ MOTOR ESQUERDO (frente)</button>
    <button class="wide" onclick="cmd('MD')">SÓ MOTOR DIREITO (frente)</button>
    <button class="wide" onclick="cmd('LED')">PISCAR LED (testar conexão)</button>
  </div>

  <div id="log">pronto</div>
  <div class="led">O LED do ESP pisca a cada comando recebido</div>

<script>
  // manda o comando e, ao soltar o dedo/mouse, manda PARAR.
  function cmd(c){
    fetch('/'+c).then(()=>{ log('OK: '+c); }).catch(()=>{ log('FALHOU: '+c); });
  }
  function parar(){ cmd('S'); }
  function log(t){ document.getElementById('log').textContent = t; }
  // ao soltar qualquer botão de direção, para o robô
  document.querySelectorAll('.grid button').forEach(function(b){
    if(b.classList.contains('stop')) return;
    if(b.textContent.indexOf('MOTOR')>=0 || b.textContent.indexOf('LED')>=0) return;
    b.addEventListener('touchend', parar);
    b.addEventListener('mouseup', parar);
    b.addEventListener('mouseleave', function(e){ if(e.buttons) parar(); });
  });
</script>
</body></html>
)HTML";

// =========================================================================
//  ROTAS HTTP
// =========================================================================
void rotaComando(const char* cmd, void (*acao)()) {
  acao();
  piscarLed();                 // confirma visualmente que o comando chegou
  server.send(200, "text/plain", cmd);
  Serial.print("CMD: "); Serial.println(cmd);
}

void setup() {
  Serial.begin(115200);

  // Pinos dos motores: OUTPUT + LOW logo de cara (GPIO 2 e 15 são strapping).
  pinMode(MOTOR_ESQ_IN1, OUTPUT);
  pinMode(MOTOR_ESQ_IN2, OUTPUT);
  pinMode(MOTOR_DIR_IN3, OUTPUT);
  pinMode(MOTOR_DIR_IN4, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  pararMotores();
  digitalWrite(PIN_LED, LOW);

  // Sobe o Access Point
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.softAP(ssid, password);
  Serial.println();
  Serial.print("AP no ar! Conecte no Wi-Fi \"");
  Serial.print(ssid);
  Serial.print("\" (senha ");
  Serial.print(password);
  Serial.print(") e abra: http://");
  Serial.println(WiFi.softAPIP());

  // Página
  server.on("/", []() { server.send_P(200, "text/html", PAGINA); });

  // Comandos de movimento
  server.on("/F",  []() { rotaComando("F (frente)",  moverFrente);    });
  server.on("/B",  []() { rotaComando("B (re)",       moverTras);      });
  server.on("/L",  []() { rotaComando("L (esquerda)", girarEsquerda);  });
  server.on("/R",  []() { rotaComando("R (direita)",  girarDireita);   });
  server.on("/S",  []() { rotaComando("S (parar)",    pararMotores);   });

  // Testes individuais
  server.on("/ME", []() { rotaComando("so ESQUERDO", soEsquerdoFrente); });
  server.on("/MD", []() { rotaComando("so DIREITO",  soDireitoFrente);  });

  // Só pisca o LED (testa se a conexão chega no ESP, sem mexer em motor)
  server.on("/LED", []() {
    piscarLed();
    server.send(200, "text/plain", "LED");
    Serial.println("CMD: LED (so pisca)");
  });

  server.begin();
  Serial.println("Servidor HTTP iniciado.");
}

void loop() {
  server.handleClient();
}

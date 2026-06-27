// Gerado a partir de index.html — interface tatica UNIT-DX9
// Servida em PROGMEM na rota "/" do WebServer (porta 80).
#ifndef INDEX_HTML_H
#define INDEX_HTML_H
#include <pgmspace.h>

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

#endif

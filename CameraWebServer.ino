#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include "esp_camera.h"

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
// Motores (L298N) - Atenção ao GPIO 12 no boot!
#define MOTOR_ESQ_IN1 12
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
// COLE SEU HTML AQUI! 
// Lembre-se de configurar o JavaScript na sua página para conectar no WebSocket:
// var ws = new WebSocket('ws://192.168.4.1:82/');
// E a tag de imagem do vídeo deve ser: <img src="http://192.168.4.1:81/stream">
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>

<html lang="pt-BR">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>UNIT-DX9 // COMBAT ROBOTICS DIVISION</title>
<style>
@import url('https://fonts.googleapis.com/css2?family=Share+Tech+Mono&family=Orbitron:wght@400;600;700;900&family=Rajdhani:wght@300;400;500;600;700&display=swap');

 {--black:      #030508;--deep:       #070d10;--panel:      #0a1218;--panel-mid:  #0e1a22;--border:     #1a3040;--border-lit: #0f4a5e;--neon:       #00ffe7;--neon-dim:   #00b8a0;--neon-faint: rgba(0,255,231,0.07);--green:      #39ff14;--green-dim:  #1a7a05;--olive:      #4a5c2f;--olive-lit:  #6b8040;--red-warn:   #ff3b3b;--amber:      #ffaa00;--amber-dim:  #7a5000;--text-main:  #c8dde8;--text-dim:   #4a6878;--text-label: #2a8060;--mono: 'Share Tech Mono', monospace;--display: 'Orbitron', sans-serif;--body: 'Rajdhani', sans-serif;}

*, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }

html { scroll-behavior: smooth; }

body {background: var(--black);color: var(--text-main);font-family: var(--body);overflow-x: hidden;min-height: 100vh;}

/* ─── SCROLLBAR ─── */::-webkit-scrollbar { width: 4px; }::-webkit-scrollbar-track { background: var(--deep); }::-webkit-scrollbar-thumb { background: var(--neon-dim); }

/* ─── GRID OVERLAY ─── */body::before {content: '';position: fixed;inset: 0;background-image:linear-gradient(rgba(0,255,231,0.025) 1px, transparent 1px),linear-gradient(90deg, rgba(0,255,231,0.025) 1px, transparent 1px);background-size: 48px 48px;pointer-events: none;z-index: 0;}

/* ─── SCAN LINE ─── */body::after {content: '';position: fixed;inset: 0;background: repeating-linear-gradient(0deg,transparent,transparent 2px,rgba(0,0,0,0.08) 2px,rgba(0,0,0,0.08) 4px);pointer-events: none;z-index: 0;}

/* ─── TOP NAV ─── */nav {position: fixed;top: 0; left: 0; right: 0;height: 52px;background: rgba(3,5,8,0.92);border-bottom: 1px solid var(--border);display: flex;align-items: center;justify-content: space-between;padding: 0 32px;z-index: 100;backdrop-filter: blur(8px);}

.nav-logo {display: flex;align-items: center;gap: 10px;font-family: var(--display);font-size: 13px;font-weight: 700;letter-spacing: 0.2em;color: var(--neon);}

.nav-logo .logo-box {width: 28px; height: 28px;border: 1px solid var(--neon);display: grid;place-items: center;font-size: 10px;clip-path: polygon(0 0, calc(100% - 6px) 0, 100% 6px, 100% 100%, 6px 100%, 0 calc(100% - 6px));background: rgba(0,255,231,0.08);}

.nav-links {display: flex;gap: 28px;list-style: none;}

.nav-links a {text-decoration: none;font-family: var(--mono);font-size: 11px;letter-spacing: 0.15em;color: var(--text-dim);transition: color 0.2s;}.nav-links a { color: var(--neon); }

.nav-status {display: flex;align-items: center;gap: 6px;font-family: var(--mono);font-size: 10px;color: var(--green);}

.status-dot {width: 6px; height: 6px;background: var(--green);border-radius: 50%;animation: blink 1.4s infinite;}

@keyframes blink {0%,100% { opacity: 1; }50% { opacity: 0.2; }}

/* ─── HERO SECTION ─── */#hero {position: relative;min-height: 100vh;display: grid;grid-template-columns: 280px 1fr 280px;grid-template-rows: 52px 1fr auto;z-index: 1;}

/* ─── LEFT PANEL ─── */.panel-left {grid-column: 1;grid-row: 2;padding: 24px 16px;border-right: 1px solid var(--border);display: flex;flex-direction: column;gap: 20px;background: linear-gradient(180deg, rgba(7,13,16,0.95) 0%, rgba(3,5,8,0.9) 100%);}

/* ─── CENTER ─── */.hero-center {grid-column: 2;grid-row: 2 / 4;display: flex;flex-direction: column;align-items: center;justify-content: center;padding: 60px 40px 40px;position: relative;}

/* ─── RIGHT PANEL ─── */.panel-right {grid-column: 3;grid-row: 2;padding: 24px 16px;border-left: 1px solid var(--border);display: flex;flex-direction: column;gap: 20px;background: linear-gradient(180deg, rgba(7,13,16,0.95) 0%, rgba(3,5,8,0.9) 100%);}

/* ─── PANEL WIDGET ─── */.widget {border: 1px solid var(--border);background: var(--panel);clip-path: polygon(0 0, calc(100% - 10px) 0, 100% 10px, 100% 100%, 10px 100%, 0 calc(100% - 10px));padding: 12px 14px;position: relative;}

.widget::before {content: '';position: absolute;top: 0; left: 0; right: 0;height: 1px;background: linear-gradient(90deg, transparent, var(--neon-dim), transparent);}

.widget-label {font-family: var(--mono);font-size: 9px;letter-spacing: 0.2em;color: var(--text-label);margin-bottom: 10px;display: flex;align-items: center;gap: 6px;}

.widget-label::before {content: '';width: 4px; height: 4px;border: 1px solid var(--neon);transform: rotate(45deg);flex-shrink: 0;}

/* ─── STAT ROW ─── */.stat-row {display: flex;justify-content: space-between;align-items: center;padding: 5px 0;border-bottom: 1px solid rgba(26,48,64,0.4);font-family: var(--mono);font-size: 10px;}

.stat-key { color: var(--text-dim); letter-spacing: 0.08em; }.stat-val { color: var(--neon); font-size: 11px; }.stat-val.warn { color: var(--amber); }.stat-val.danger { color: var(--red-warn); }.stat-val.ok { color: var(--green); }

/* ─── BAR GAUGE ─── */.gauge-wrap {margin: 8px 0;}

.gauge-label {display: flex;justify-content: space-between;font-family: var(--mono);font-size: 9px;color: var(--text-dim);margin-bottom: 4px;}

.gauge-track {height: 6px;background: rgba(10,18,24,0.8);border: 1px solid var(--border);position: relative;overflow: hidden;}

.gauge-fill {height: 100%;background: linear-gradient(90deg, var(--neon-dim), var(--neon));transition: width 1s ease;position: relative;}

.gauge-fill::after {content: '';position: absolute;right: 0; top: 0; bottom: 0;width: 3px;background: var(--neon);box-shadow: 0 0 6px var(--neon);}

.gauge-fill.warn {background: linear-gradient(90deg, var(--amber-dim), var(--amber));}.gauge-fill.warn::after { background: var(--amber); box-shadow: 0 0 6px var(--amber); }

.gauge-fill.danger {background: linear-gradient(90deg, #7a0000, var(--red-warn));}.gauge-fill.danger::after { background: var(--red-warn); box-shadow: 0 0 6px var(--red-warn); }

/* ─── MINI RADAR ─── */.mini-radar {position: relative;width: 100%;aspect-ratio: 1;max-width: 200px;margin: 0 auto;}

.mini-radar svg { width: 100%; height: 100%; }

.radar-sweep {transform-origin: center;animation: sweep 3s linear infinite;}

@keyframes sweep {from { transform: rotate(0deg); }to { transform: rotate(360deg); }}

/* ─── COORD GRID ─── */.coord-display {font-family: var(--mono);font-size: 10px;color: var(--neon);text-align: center;line-height: 1.9;letter-spacing: 0.05em;}

/* ─── HERO CENTER CONTENT ─── */.hero-eyebrow {font-family: var(--mono);font-size: 10px;letter-spacing: 0.35em;color: var(--neon-dim);margin-bottom: 16px;display: flex;align-items: center;gap: 12px;}.hero-eyebrow::before, .hero-eyebrow::after {content: '';flex: 1;max-width: 60px;height: 1px;background: var(--neon-dim);}

.hero-title {font-family: var(--display);font-size: clamp(32px, 5vw, 68px);font-weight: 900;line-height: 0.9;letter-spacing: -0.01em;text-align: center;color: #fff;position: relative;}

.hero-title span {display: block;color: var(--neon);text-shadow: 0 0 20px rgba(0,255,231,0.5), 0 0 60px rgba(0,255,231,0.2);}

.hero-title .sub {font-size: 0.38em;color: var(--text-dim);letter-spacing: 0.4em;font-weight: 400;display: block;margin-top: 12px;}

/* ─── ROBOT DISPLAY ─── */.robot-display {position: relative;width: 100%;max-width: 520px;margin: 40px auto 20px;}

.robot-hud-frame {position: relative;width: 100%;aspect-ratio: 16/10;border: 1px solid var(--border-lit);background: radial-gradient(ellipse at center, rgba(0,80,70,0.15) 0%, var(--deep) 70%);overflow: hidden;clip-path: polygon(0 12px, 12px 0,calc(100% - 12px) 0, 100% 12px,100% calc(100% - 12px), calc(100% - 12px) 100%,12px 100%, 0 calc(100% - 12px));}

/* Corner brackets */.robot-hud-frame::before {content: '';position: absolute;inset: 8px;border: 1px solid transparent;border-top-color: var(--neon);border-left-color: var(--neon);clip-path: polygon(0 0, 30px 0, 30px 1px, 1px 1px, 1px 30px, 0 30px);filter: drop-shadow(0 0 4px var(--neon));}

.robot-hud-frame::after {content: '';position: absolute;inset: 8px;border: 1px solid transparent;border-bottom-color: var(--neon);border-right-color: var(--neon);clip-path: polygon(100% 100%, calc(100% - 30px) 100%, calc(100% - 30px) calc(100% - 1px), calc(100% - 1px) calc(100% - 1px), calc(100% - 1px) calc(100% - 30px), 100% calc(100% - 30px));filter: drop-shadow(0 0 4px var(--neon));}

/* ─── DRONE SVG ─── */.drone-svg-wrap {width: 100%;height: 100%;display: flex;align-items: center;justify-content: center;position: relative;}

.drone-svg-wrap svg {width: 75%;max-width: 340px;filter: drop-shadow(0 0 16px rgba(0,255,231,0.3));animation: hover-float 4s ease-in-out infinite;}

@keyframes hover-float {0%,100% { transform: translateY(0px); }50% { transform: translateY(-10px); }}

/* HUD overlays on drone display */.hud-overlay {position: absolute;inset: 0;pointer-events: none;}

.hud-crosshair {position: absolute;top: 50%; left: 50%;transform: translate(-50%, -50%);width: 60px; height: 60px;border: 1px solid rgba(0,255,231,0.3);border-radius: 50%;animation: rotate-slow 8s linear infinite;}

@keyframes rotate-slow {from { transform: translate(-50%,-50%) rotate(0deg); }to { transform: translate(-50%,-50%) rotate(360deg); }}

.hud-crosshair::before, .hud-crosshair::after {content: '';position: absolute;background: rgba(0,255,231,0.5);}.hud-crosshair::before { width: 1px; height: 20px; left: 50%; top: -10px; margin-left: -0.5px; }.hud-crosshair::after { height: 1px; width: 20px; top: 50%; left: -10px; margin-top: -0.5px; }

/* HUD data tags */.hud-tag {position: absolute;font-family: var(--mono);font-size: 9px;color: var(--neon);background: rgba(3,5,8,0.75);padding: 3px 6px;border: 1px solid rgba(0,255,231,0.25);white-space: nowrap;clip-path: polygon(0 0, calc(100% - 5px) 0, 100% 5px, 100% 100%, 5px 100%, 0 calc(100% - 5px));}

.hud-tag.tl { top: 16px; left: 16px; }.hud-tag.tr { top: 16px; right: 16px; }.hud-tag.bl { bottom: 16px; left: 16px; }.hud-tag.br { bottom: 16px; right: 16px; }

.hud-scan-line {position: absolute;left: 0; right: 0;height: 2px;background: linear-gradient(90deg, transparent, var(--neon), transparent);opacity: 0.4;animation: scan-down 4s linear infinite;}

@keyframes scan-down {from { top: 0%; }to   { top: 100%; }}

/* ─── TARGETING RETICLE ─── */.targeting-lines {position: absolute;inset: 0;overflow: hidden;pointer-events: none;}

.targeting-lines svg {width: 100%; height: 100%;animation: none;filter: none;}

/* ─── SCROLLING DATA TEXT ─── */.data-scroll-wrap {position: absolute;right: 0; top: 0; bottom: 0;width: 90px;overflow: hidden;pointer-events: none;}

.data-scroll {font-family: var(--mono);font-size: 8px;color: rgba(0,255,231,0.25);line-height: 1.6;animation: scroll-up 12s linear infinite;white-space: nowrap;}

@keyframes scroll-up {from { transform: translateY(100%); }to   { transform: translateY(-100%); }}

/* ─── MISSION INFO STRIP ─── */.mission-strip {display: flex;gap: 2px;margin-top: 6px;font-family: var(--mono);font-size: 9px;}

.mission-block {flex: 1;padding: 6px 8px;background: var(--panel);border: 1px solid var(--border);border-top: 2px solid var(--neon-dim);}

.mission-block .mb-label { color: var(--text-label); letter-spacing: 0.15em; margin-bottom: 3px; }.mission-block .mb-value { color: var(--text-main); font-size: 11px; }

/* ─── HERO ACTIONS ─── */.hero-actions {display: flex;gap: 12px;margin-top: 32px;flex-wrap: wrap;justify-content: center;}

.btn {font-family: var(--display);font-size: 11px;font-weight: 700;letter-spacing: 0.2em;padding: 13px 28px;cursor: pointer;text-decoration: none;transition: all 0.2s;clip-path: polygon(0 0, calc(100% - 10px) 0, 100% 10px, 100% 100%, 10px 100%, 0 calc(100% - 10px));border: none;display: inline-flex;align-items: center;gap: 8px;}

.btn-primary {background: var(--neon);color: var(--black);}.btn-primary {background: #fff;box-shadow: 0 0 24px rgba(0,255,231,0.5);}

.btn-secondary {background: transparent;color: var(--neon);border: 1px solid var(--neon-dim);clip-path: polygon(0 0, calc(100% - 10px) 0, 100% 10px, 100% 100%, 10px 100%, 0 calc(100% - 10px));}.btn-secondary {background: var(--neon-faint);border-color: var(--neon);}

/* ─── BOTTOM STATUS BAR ─── */.status-bar {grid-column: 1 / -1;grid-row: 3;background: rgba(7,13,16,0.96);border-top: 1px solid var(--border);padding: 0 32px;height: 38px;display: flex;align-items: center;gap: 0;overflow: hidden;}

.status-item {font-family: var(--mono);font-size: 9px;padding: 0 20px;border-right: 1px solid var(--border);height: 100%;display: flex;align-items: center;gap: 6px;}

.status-item { padding-left: 0; }.si-key { color: var(--text-dim); }.si-val { color: var(--neon); }

.status-ticker {flex: 1;overflow: hidden;padding-left: 20px;}

.ticker-inner {display: inline-flex;gap: 80px;white-space: nowrap;animation: ticker 28s linear infinite;font-family: var(--mono);font-size: 9px;color: var(--text-dim);}

@keyframes ticker {from { transform: translateX(100%); }to { transform: translateX(-100%); }}

/* ─── SPECS SECTION ─── */#specs {position: relative;z-index: 1;padding: 100px 64px;border-top: 1px solid var(--border);background: linear-gradient(180deg, var(--deep) 0%, var(--black) 100%);}

.section-head {text-align: center;margin-bottom: 64px;}

.section-tag {font-family: var(--mono);font-size: 10px;letter-spacing: 0.3em;color: var(--neon-dim);margin-bottom: 14px;}

.section-title {font-family: var(--display);font-size: clamp(26px, 4vw, 46px);font-weight: 700;letter-spacing: 0.05em;color: #fff;}

.section-title span { color: var(--neon); }

.specs-grid {display: grid;grid-template-columns: repeat(auto-fit, minmax(260px, 1fr));gap: 2px;}

.spec-card {background: var(--panel);border: 1px solid var(--border);padding: 28px 24px;position: relative;clip-path: polygon(0 0, calc(100% - 14px) 0, 100% 14px, 100% 100%, 14px 100%, 0 calc(100% - 14px));transition: border-color 0.3s;}

.spec-card {border-color: var(--border-lit);background: var(--panel-mid);}

.spec-card::before {content: attr(data-num);position: absolute;top: 12px; right: 16px;font-family: var(--display);font-size: 36px;font-weight: 900;color: rgba(0,255,231,0.05);line-height: 1;}

.spec-icon {width: 36px; height: 36px;border: 1px solid var(--neon-dim);display: grid;place-items: center;margin-bottom: 16px;clip-path: polygon(0 0, calc(100% - 6px) 0, 100% 6px, 100% 100%, 6px 100%, 0 calc(100% - 6px));background: rgba(0,255,231,0.06);color: var(--neon);font-size: 16px;}

.spec-name {font-family: var(--display);font-size: 13px;font-weight: 700;letter-spacing: 0.1em;color: var(--text-main);margin-bottom: 8px;}

.spec-value {font-family: var(--mono);font-size: 22px;color: var(--neon);margin-bottom: 6px;text-shadow: 0 0 12px rgba(0,255,231,0.3);}

.spec-desc {font-size: 13px;color: var(--text-dim);line-height: 1.5;font-family: var(--body);}

/* ─── MISSION SECTION ─── */#mission {position: relative;z-index: 1;padding: 100px 64px;border-top: 1px solid var(--border);}

.mission-grid {display: grid;grid-template-columns: 1fr 1fr;gap: 60px;align-items: center;max-width: 1100px;margin: 0 auto;}

.mission-visual {position: relative;aspect-ratio: 4/3;border: 1px solid var(--border-lit);background: var(--panel);overflow: hidden;clip-path: polygon(0 0, calc(100% - 20px) 0, 100% 20px, 100% 100%, 20px 100%, 0 calc(100% - 20px));}

.mission-radar-svg {width: 100%;height: 100%;}

.mission-text h3 {font-family: var(--display);font-size: 32px;font-weight: 700;color: #fff;margin-bottom: 20px;letter-spacing: 0.04em;}

.mission-text h3 em {color: var(--neon);font-style: normal;}

.mission-text p {font-size: 16px;color: var(--text-dim);line-height: 1.7;margin-bottom: 20px;font-family: var(--body);}

.mission-list {list-style: none;display: grid;gap: 10px;}

.mission-list li {font-family: var(--mono);font-size: 12px;color: var(--text-main);display: flex;align-items: center;gap: 10px;}

.mission-list li::before {content: '▸';color: var(--neon);}

/* ─── FOOTER ─── */footer {position: relative;z-index: 1;border-top: 1px solid var(--border);padding: 32px 64px;background: var(--deep);display: flex;align-items: center;justify-content: space-between;}

.footer-brand {font-family: var(--display);font-size: 11px;font-weight: 700;letter-spacing: 0.2em;color: var(--neon-dim);}

.footer-copy {font-family: var(--mono);font-size: 9px;color: var(--text-dim);letter-spacing: 0.1em;}

.footer-links {display: flex;gap: 24px;list-style: none;}

.footer-links a {font-family: var(--mono);font-size: 9px;color: var(--text-dim);text-decoration: none;letter-spacing: 0.15em;}.footer-links a { color: var(--neon); }

/* ─── ANIMATED COUNTER ─── */.counter { display: inline-block; }

/* ─── RESPONSIVE ─── */@media (max-width: 1100px) {#hero { grid-template-columns: 220px 1fr 220px; }}

@media (max-width: 860px) {#hero {grid-template-columns: 1fr;grid-template-rows: 52px auto auto auto;}.panel-left, .panel-right {grid-column: 1;border-right: none;border-left: none;border-top: 1px solid var(--border);border-bottom: 1px solid var(--border);display: grid;grid-template-columns: 1fr 1fr;}.hero-center { grid-column: 1; padding: 20px; }.mission-grid { grid-template-columns: 1fr; }#specs, #mission { padding: 60px 24px; }footer { flex-direction: column; gap: 16px; text-align: center; }}

@media (max-width: 500px) {nav { padding: 0 16px; }.nav-links { display: none; }.panel-left, .panel-right { grid-template-columns: 1fr; }}

/* ─── GLITCH EFFECT ON TITLE ─── */@keyframes glitch-1 {0%,96%,100% { clip-path: inset(0 0 100% 0); transform: translate(0); }97% { clip-path: inset(20% 0 60% 0); transform: translate(-3px, 0); }98% { clip-path: inset(50% 0 20% 0); transform: translate(3px, 0); }99% { clip-path: inset(70% 0 5% 0);  transform: translate(-1px, 0); }}

.hero-title::before {content: attr(data-text);position: absolute;left: 0; right: 0;top: 0;color: var(--red-warn);animation: glitch-1 6s infinite;pointer-events: none;}

/* ─── PROGRESS RINGS (Radar) ─── */.ring-label {font-family: var(--mono);font-size: 8px;fill: var(--text-dim);}</style>

</head>
<body>

<!-- ─── NAV ─── -->

<nav>
  <div class="nav-logo">
    <div class="logo-box">DX</div>
    UNIT-DX9 COMBAT SYSTEMS
  </div>
  <ul class="nav-links">
    <li><a href="#hero">// STATUS</a></li>
    <li><a href="#specs">// SPECS</a></li>
    <li><a href="#mission">// MISSION</a></li>
    <li><a href="#">// DEPLOY</a></li>
  </ul>
  <div class="nav-status">
    <div class="status-dot"></div>
    SYS ONLINE // SEC-LVL: ALPHA
  </div>
</nav>

<!-- ─── HERO ─── -->

<section id="hero">

  <!-- LEFT PANEL -->

  <aside class="panel-left">

<div class="widget">
  <div class="widget-label">SYSTEM STATUS</div>
  <div class="stat-row"><span class="stat-key">UNIT ID</span><span class="stat-val">DX9-RAPTOR</span></div>
  <div class="stat-row"><span class="stat-key">FIRMWARE</span><span class="stat-val">v4.7.2</span></div>
  <div class="stat-row"><span class="stat-key">UPTIME</span><span class="stat-val ok" id="uptime">00:00:00</span></div>
  <div class="stat-row"><span class="stat-key">TEMP</span><span class="stat-val warn">38.2°C</span></div>
  <div class="stat-row"><span class="stat-key">THREAT LVL</span><span class="stat-val danger">HIGH</span></div>
</div>

<div class="widget">
  <div class="widget-label">POWER SYSTEMS</div>
  <div class="gauge-wrap">
    <div class="gauge-label"><span>MAIN POWER</span><span class="stat-val">87%</span></div>
    <div class="gauge-track"><div class="gauge-fill" style="width:87%"></div></div>
  </div>
  <div class="gauge-wrap">
    <div class="gauge-label"><span>SHIELDS</span><span class="stat-val warn">54%</span></div>
    <div class="gauge-track"><div class="gauge-fill warn" style="width:54%"></div></div>
  </div>
  <div class="gauge-wrap">
    <div class="gauge-label"><span>WEAPONS SYS</span><span class="stat-val ok">100%</span></div>
    <div class="gauge-track"><div class="gauge-fill" style="width:100%"></div></div>
  </div>
  <div class="gauge-wrap">
    <div class="gauge-label"><span>COMMS</span><span class="stat-val danger">21%</span></div>
    <div class="gauge-track"><div class="gauge-fill danger" style="width:21%"></div></div>
  </div>
</div>

<div class="widget">
  <div class="widget-label">GPS COORDINATES</div>
  <div class="coord-display">
    30°35'84.5" N<br>
    17°95'64.5" W<br>
    <span style="color:var(--text-label)">ALT:</span> 217m<br>
    <span style="color:var(--text-label)">DIR:</span> 013°<br>
    <span style="color:var(--text-label)">SPD:</span> 0 km/h
  </div>
</div>

<div class="widget">
  <div class="widget-label">MISSION LOG</div>
  <div style="font-family:var(--mono);font-size:9px;line-height:2;color:var(--text-dim);" id="log-output">
    &gt; UNIT DEPLOYED...<br>
    &gt; AREA SCAN INIT...<br>
    &gt; TARGET LOCKED...<br>
    &gt; AWAITING CMD...
  </div>
</div>

  </aside>

  <!-- CENTER -->

  <div class="hero-center">
    <div class="hero-eyebrow">CLASSIFIED // ACTIVE DEPLOYMENT // SEC-ALPHA</div>

<h1 class="hero-title" data-text="UNIT-DX9" style="position:relative">
  UNIT-DX9
  <span>PREDATOR MQ9</span>
  <span class="sub">AUTONOMOUS COMBAT RECONNAISSANCE PLATFORM</span>
</h1>

<!-- ROBOT DISPLAY -->
<div class="robot-display">
  <div class="robot-hud-frame">

    <!-- Targeting SVG overlay -->
    <div class="targeting-lines">
      <svg viewBox="0 0 520 325" preserveAspectRatio="none">
        <!-- corner L brackets -->
        <polyline points="0,40 0,0 40,0" fill="none" stroke="#00ffe7" stroke-width="1" opacity="0.5"/>
        <polyline points="480,0 520,0 520,40" fill="none" stroke="#00ffe7" stroke-width="1" opacity="0.5"/>
        <polyline points="0,285 0,325 40,325" fill="none" stroke="#00ffe7" stroke-width="1" opacity="0.5"/>
        <polyline points="480,325 520,325 520,285" fill="none" stroke="#00ffe7" stroke-width="1" opacity="0.5"/>
        <!-- H/V center lines -->
        <line x1="0" y1="162" x2="180" y2="162" stroke="#00ffe7" stroke-width="0.5" opacity="0.2"/>
        <line x1="340" y1="162" x2="520" y2="162" stroke="#00ffe7" stroke-width="0.5" opacity="0.2"/>
        <line x1="260" y1="0" x2="260" y2="120" stroke="#00ffe7" stroke-width="0.5" opacity="0.2"/>
        <line x1="260" y1="200" x2="260" y2="325" stroke="#00ffe7" stroke-width="0.5" opacity="0.2"/>
        <!-- Horizon rule -->
        <line x1="50" y1="120" x2="100" y2="120" stroke="#00ffe7" stroke-width="0.5" opacity="0.3"/>
        <line x1="420" y1="120" x2="470" y2="120" stroke="#00ffe7" stroke-width="0.5" opacity="0.3"/>
        <!-- Range lines right -->
        <line x1="460" y1="80"  x2="480" y2="80"  stroke="#00ffe7" stroke-width="0.4" opacity="0.3"/>
        <line x1="460" y1="120" x2="480" y2="120" stroke="#00ffe7" stroke-width="0.4" opacity="0.3"/>
        <line x1="460" y1="160" x2="480" y2="160" stroke="#00ffe7" stroke-width="0.4" opacity="0.3"/>
        <line x1="460" y1="200" x2="480" y2="200" stroke="#00ffe7" stroke-width="0.4" opacity="0.3"/>
        <line x1="460" y1="240" x2="480" y2="240" stroke="#00ffe7" stroke-width="0.4" opacity="0.3"/>
        <!-- Range ticks right text -->
        <text x="450" y="83"  font-family="'Share Tech Mono',monospace" font-size="7" fill="#2a8060" text-anchor="end">400</text>
        <text x="450" y="123" font-family="'Share Tech Mono',monospace" font-size="7" fill="#2a8060" text-anchor="end">300</text>
        <text x="450" y="163" font-family="'Share Tech Mono',monospace" font-size="7" fill="#2a8060" text-anchor="end">200</text>
        <text x="450" y="203" font-family="'Share Tech Mono',monospace" font-size="7" fill="#2a8060" text-anchor="end">100</text>
        <!-- DST badge -->
        <text x="260" y="24"  font-family="'Share Tech Mono',monospace" font-size="8" fill="#00ffe7" text-anchor="middle" opacity="0.6">DSMT</text>
        <text x="260" y="308" font-family="'Share Tech Mono',monospace" font-size="7" fill="#2a8060" text-anchor="middle">LOCKED</text>
      </svg>
    </div>

    <!-- Drone wireframe SVG -->
    <div class="drone-svg-wrap">
      <svg viewBox="0 0 400 220" xmlns="http://www.w3.org/2000/svg">
        <!-- Main fuselage -->
        <path d="M190 108 L180 60 L220 60 L210 108 Z" fill="none" stroke="#00ffe7" stroke-width="1" opacity="0.7"/>
        <!-- Fuselage body -->
        <ellipse cx="200" cy="112" rx="38" ry="16" fill="rgba(0,255,231,0.04)" stroke="#00ffe7" stroke-width="0.8" opacity="0.6"/>
        <!-- Tail section -->
        <path d="M185 112 L170 145 L175 148 L190 120 Z" fill="rgba(0,255,231,0.04)" stroke="#00ffe7" stroke-width="0.8" opacity="0.6"/>
        <path d="M215 112 L230 145 L225 148 L210 120 Z" fill="rgba(0,255,231,0.04)" stroke="#00ffe7" stroke-width="0.8" opacity="0.6"/>
        <!-- Vertical stabilizer -->
        <path d="M198 125 L195 158 L205 158 L202 125 Z" fill="rgba(0,255,231,0.06)" stroke="#00ffe7" stroke-width="0.8" opacity="0.6"/>
        <!-- Main wings -->
        <path d="M175 110 L50 130 L55 138 L178 118 Z" fill="rgba(0,255,231,0.04)" stroke="#00ffe7" stroke-width="0.8" opacity="0.7"/>
        <path d="M225 110 L350 130 L345 138 L222 118 Z" fill="rgba(0,255,231,0.04)" stroke="#00ffe7" stroke-width="0.8" opacity="0.7"/>
        <!-- Wing details -->
        <line x1="80" y1="126" x2="78" y2="134" stroke="#00ffe7" stroke-width="0.5" opacity="0.4"/>
        <line x1="110" y1="122" x2="108" y2="130" stroke="#00ffe7" stroke-width="0.5" opacity="0.4"/>
        <line x1="140" y1="118" x2="138" y2="126" stroke="#00ffe7" stroke-width="0.5" opacity="0.4"/>
        <line x1="320" y1="126" x2="322" y2="134" stroke="#00ffe7" stroke-width="0.5" opacity="0.4"/>
        <line x1="290" y1="122" x2="292" y2="130" stroke="#00ffe7" stroke-width="0.5" opacity="0.4"/>
        <line x1="260" y1="118" x2="262" y2="126" stroke="#00ffe7" stroke-width="0.5" opacity="0.4"/>
        <!-- Engine pods -->
        <ellipse cx="80" cy="132" rx="14" ry="6" fill="rgba(0,255,231,0.05)" stroke="#00ffe7" stroke-width="0.7" opacity="0.6"/>
        <ellipse cx="320" cy="132" rx="14" ry="6" fill="rgba(0,255,231,0.05)" stroke="#00ffe7" stroke-width="0.7" opacity="0.6"/>
        <!-- Propeller front -->
        <circle cx="200" cy="70" r="6" fill="rgba(0,255,231,0.08)" stroke="#00ffe7" stroke-width="0.8" opacity="0.7"/>
        <!-- Sensor ball nose -->
        <circle cx="200" cy="104" r="9" fill="rgba(0,255,231,0.06)" stroke="#00ffe7" stroke-width="0.8" opacity="0.7"/>
        <!-- Weapon pylons -->
        <rect x="110" y="124" width="20" height="5" rx="2" fill="rgba(57,255,20,0.1)" stroke="#39ff14" stroke-width="0.6" opacity="0.6"/>
        <rect x="150" y="120" width="22" height="5" rx="2" fill="rgba(57,255,20,0.1)" stroke="#39ff14" stroke-width="0.6" opacity="0.6"/>
        <rect x="270" y="124" width="20" height="5" rx="2" fill="rgba(57,255,20,0.1)" stroke="#39ff14" stroke-width="0.6" opacity="0.6"/>
        <rect x="228" y="120" width="22" height="5" rx="2" fill="rgba(57,255,20,0.1)" stroke="#39ff14" stroke-width="0.6" opacity="0.6"/>
        <!-- Wing ground track -->
        <line x1="50" y1="138" x2="350" y2="138" stroke="#00ffe7" stroke-width="0.3" opacity="0.1"/>
        <!-- Glow effect dots -->
        <circle cx="80"  cy="132" r="2" fill="#00ffe7" opacity="0.5">
          <animate attributeName="opacity" values="0.5;1;0.5" dur="1.6s" repeatCount="indefinite"/>
        </circle>
        <circle cx="320" cy="132" r="2" fill="#00ffe7" opacity="0.5">
          <animate attributeName="opacity" values="0.5;1;0.5" dur="1.6s" begin="0.4s" repeatCount="indefinite"/>
        </circle>
        <circle cx="200" cy="104" r="2" fill="#39ff14" opacity="0.8">
          <animate attributeName="opacity" values="0.8;0.2;0.8" dur="0.9s" repeatCount="indefinite"/>
        </circle>
      </svg>
    </div>

    <!-- Scan line -->
    <div class="hud-overlay">
      <div class="hud-scan-line"></div>
    </div>

    <!-- Corner data tags -->
    <div class="hud-tag tl">SQ: 048/X2</div>
    <div class="hud-tag tr">ARMED ● ACTIVE</div>
    <div class="hud-tag bl">RNG: 219.4 KM</div>
    <div class="hud-tag br">ELV: 440M</div>

    <!-- Crosshair -->
    <div class="hud-overlay" style="display:flex;align-items:center;justify-content:center;">
      <div class="hud-crosshair"></div>
    </div>

    <!-- Data stream right -->
    <div class="data-scroll-wrap">
      <div class="data-scroll">
        48°52'31"N<br>
        123°23'33"W<br>
        DST:122<br>
        RNG:219.4<br>
        ELV:440M<br>
        SPD:0KPH<br>
        HEADING:013<br>
        MODE:RECON<br>
        FUEL:87PCT<br>
        STATUS:OK<br>
        SYS:ARMED<br>
        THREAT:HI<br>
        48°52'31"N<br>
        123°23'33"W
      </div>
    </div>

  </div>

  <!-- Mission strip -->
  <div class="mission-strip">
    <div class="mission-block">
      <div class="mb-label">MISSION</div>
      <div class="mb-value">RECON-012564</div>
    </div>
    <div class="mission-block">
      <div class="mb-label">AREA</div>
      <div class="mb-value">34M²</div>
    </div>
    <div class="mission-block">
      <div class="mb-label">DURATION</div>
      <div class="mb-value" id="mission-time">00:00:00</div>
    </div>
    <div class="mission-block">
      <div class="mb-label">TARGETS</div>
      <div class="mb-value" style="color:var(--red-warn)">04 ACTIVE</div>
    </div>
  </div>
</div>

<div class="hero-actions">
  <a href="#specs" class="btn btn-primary">▶ DEPLOY UNIT</a>
  <a href="#mission" class="btn btn-secondary">◈ VIEW MISSION</a>
</div>

  </div>

  <!-- RIGHT PANEL -->

  <aside class="panel-right">

<div class="widget">
  <div class="widget-label">RADAR SCAN</div>
  <div class="mini-radar">
    <svg viewBox="0 0 200 200" xmlns="http://www.w3.org/2000/svg">
      <!-- Grid rings -->
      <circle cx="100" cy="100" r="90" fill="none" stroke="rgba(0,255,231,0.08)" stroke-width="1"/>
      <circle cx="100" cy="100" r="68" fill="none" stroke="rgba(0,255,231,0.1)" stroke-width="0.5"/>
      <circle cx="100" cy="100" r="46" fill="none" stroke="rgba(0,255,231,0.12)" stroke-width="0.5"/>
      <circle cx="100" cy="100" r="24" fill="none" stroke="rgba(0,255,231,0.15)" stroke-width="0.5"/>
      <!-- Cross hairs -->
      <line x1="100" y1="10" x2="100" y2="190" stroke="rgba(0,255,231,0.08)" stroke-width="0.5"/>
      <line x1="10" y1="100" x2="190" y2="100" stroke="rgba(0,255,231,0.08)" stroke-width="0.5"/>
      <!-- Diagonal -->
      <line x1="36" y1="36" x2="164" y2="164" stroke="rgba(0,255,231,0.04)" stroke-width="0.5"/>
      <line x1="164" y1="36" x2="36" y2="164" stroke="rgba(0,255,231,0.04)" stroke-width="0.5"/>
      <!-- Sweep -->
      <g class="radar-sweep">
        <path d="M100 100 L100 14 A86,86 0 0,1 165 155 Z"
          fill="url(#sweep-grad)" opacity="0.5"/>
        <defs>
          <radialGradient id="sweep-grad" cx="50%" cy="50%">
            <stop offset="0%" stop-color="#00ffe7" stop-opacity="0.4"/>
            <stop offset="100%" stop-color="#00ffe7" stop-opacity="0"/>
          </radialGradient>
        </defs>
      </g>
      <!-- Contact dots -->
      <circle cx="130" cy="65"  r="3" fill="#ff3b3b" opacity="0.9"><animate attributeName="opacity" values="0.9;0.2;0.9" dur="1.2s" repeatCount="indefinite"/></circle>
      <circle cx="75"  cy="140" r="2" fill="#ffaa00" opacity="0.8"><animate attributeName="opacity" values="0.8;0.1;0.8" dur="1.8s" repeatCount="indefinite"/></circle>
      <circle cx="155" cy="120" r="2" fill="#ff3b3b" opacity="0.7"><animate attributeName="opacity" values="0.7;0.1;0.7" dur="0.9s" repeatCount="indefinite"/></circle>
      <circle cx="50"  cy="80"  r="2" fill="#00ffe7" opacity="0.6"><animate attributeName="opacity" values="0.6;0.0;0.6" dur="2.2s" repeatCount="indefinite"/></circle>
      <!-- Center dot -->
      <circle cx="100" cy="100" r="3" fill="#00ffe7" opacity="1"/>
      <!-- Range labels -->
      <text class="ring-label" x="102" y="57"  font-family="'Share Tech Mono',monospace" font-size="7" fill="rgba(0,255,231,0.3)">40</text>
      <text class="ring-label" x="102" y="79"  font-family="'Share Tech Mono',monospace" font-size="7" fill="rgba(0,255,231,0.3)">60</text>
      <text class="ring-label" x="102" y="101" font-family="'Share Tech Mono',monospace" font-size="7" fill="rgba(0,255,231,0.3)">80</text>
    </svg>
  </div>
  <div style="display:flex;justify-content:space-between;font-family:var(--mono);font-size:8px;color:var(--text-dim);margin-top:6px;">
    <span>▲ 4 CONTACTS</span>
    <span>RNG: 80KM</span>
  </div>
</div>

<div class="widget">
  <div class="widget-label">TARGET INTEL</div>
  <div class="stat-row"><span class="stat-key">TGT-001</span><span class="stat-val danger">HOSTILE</span></div>
  <div class="stat-row"><span class="stat-key">TGT-002</span><span class="stat-val danger">HOSTILE</span></div>
  <div class="stat-row"><span class="stat-key">TGT-003</span><span class="stat-val warn">UNKNOWN</span></div>
  <div class="stat-row"><span class="stat-key">TGT-004</span><span class="stat-val ok">NEUTRAL</span></div>
</div>

<div class="widget">
  <div class="widget-label">WEAPON LOADOUT</div>
  <div class="stat-row"><span class="stat-key">AGM-114</span><span class="stat-val">× 4</span></div>
  <div class="stat-row"><span class="stat-key">GBU-12</span><span class="stat-val">× 2</span></div>
  <div class="stat-row"><span class="stat-key">FLIR POD</span><span class="stat-val ok">ACTIVE</span></div>
  <div class="stat-row"><span class="stat-key">ECM</span><span class="stat-val warn">STANDBY</span></div>
</div>

<div class="widget">
  <div class="widget-label">SIGNAL EQUALIZER</div>
  <div style="display:flex;align-items:flex-end;gap:2px;height:40px;margin-top:4px;" id="eq-bars">
    <!-- Generated by JS -->
  </div>
  <div style="font-family:var(--mono);font-size:8px;color:var(--text-dim);margin-top:6px;">
    ZX1/AUDIO INFO G5E-L
  </div>
</div>

  </aside>

  <!-- STATUS BAR -->

  <div class="status-bar">
    <div class="status-item"><span class="si-key">SYS:</span><span class="si-val">ONLINE</span></div>
    <div class="status-item"><span class="si-key">MODE:</span><span class="si-val">RECON</span></div>
    <div class="status-item"><span class="si-key">FREQ:</span><span class="si-val">5.8GHz</span></div>
    <div class="status-item"><span class="si-key">ENC:</span><span class="si-val">AES-256</span></div>
    <div class="status-item"><span class="si-key">SAT:</span><span class="si-val ok">LINKED</span></div>
    <div class="status-ticker">
      <div class="ticker-inner">
        ◈ UNIT-DX9 PREDATOR MQ9 DRONE // ACTIVE DEPLOYMENT ZONE DELTA // THREAT LEVEL: HIGH // AWAIT AUTHORIZATION CODE // COMMS DEGRADED — BACKUP CHANNEL ACTIVE // ALL SYSTEMS GO // MISSION CLOCK RUNNING
        &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
        ◈ UNIT-DX9 PREDATOR MQ9 DRONE // ACTIVE DEPLOYMENT ZONE DELTA // THREAT LEVEL: HIGH // AWAIT AUTHORIZATION CODE
      </div>
    </div>
  </div>

</section>

<!-- ─── SPECS ─── -->

<section id="specs">
  <div class="section-head">
    <div class="section-tag">// TECHNICAL SPECIFICATIONS //</div>
    <h2 class="section-title">COMBAT <span>CAPABILITIES</span></h2>
  </div>

  <div class="specs-grid">

<div class="spec-card" data-num="01">
  <div class="spec-icon">◎</div>
  <div class="spec-name">OPERATIONAL RANGE</div>
  <div class="spec-value"><span class="counter" data-target="1850">0</span> KM</div>
  <div class="spec-desc">Extended loitering capability with onboard fuel reserve systems and satellite uplink.</div>
</div>

<div class="spec-card" data-num="02">
  <div class="spec-icon">↑</div>
  <div class="spec-name">MAX ALTITUDE</div>
  <div class="spec-value"><span class="counter" data-target="15240">0</span> M</div>
  <div class="spec-desc">Stratospheric deployment ceiling with pressurized avionics and thermal shielding.</div>
</div>

<div class="spec-card" data-num="03">
  <div class="spec-icon">⚡</div>
  <div class="spec-name">ENGINE OUTPUT</div>
  <div class="spec-value"><span class="counter" data-target="900">0</span> SHP</div>
  <div class="spec-desc">Honeywell TPE331-10GD turboprop with redundant ignition and auto-lean control.</div>
</div>

<div class="spec-card" data-num="04">
  <div class="spec-icon">◈</div>
  <div class="spec-name">PAYLOAD CAPACITY</div>
  <div class="spec-value"><span class="counter" data-target="1700">0</span> KG</div>
  <div class="spec-desc">Internal + external hardpoints accepting AGM, GBU, and sensor pod configurations.</div>
</div>

<div class="spec-card" data-num="05">
  <div class="spec-icon">▶</div>
  <div class="spec-name">CRUISE SPEED</div>
  <div class="spec-value"><span class="counter" data-target="482">0</span> KM/H</div>
  <div class="spec-desc">Maximum dash speed with sustained cruise at 310 km/h for extended endurance.</div>
</div>

<div class="spec-card" data-num="06">
  <div class="spec-icon">◉</div>
  <div class="spec-name">SENSOR RESOLUTION</div>
  <div class="spec-value"><span class="counter" data-target="4">0</span>K FLIR</div>
  <div class="spec-desc">Multi-spectral targeting pod with IR, daylight TV, and laser designator integration.</div>
</div>

  </div>
</section>

<!-- ─── MISSION ─── -->

<section id="mission">
  <div class="section-head">
    <div class="section-tag">// MISSION PROFILE //</div>
    <h2 class="section-title">OPERATIONAL <span>DOCTRINE</span></h2>
  </div>

  <div class="mission-grid">

<div class="mission-visual">
  <svg class="mission-radar-svg" viewBox="0 0 480 360" xmlns="http://www.w3.org/2000/svg">
    <!-- background grid -->
    <defs>
      <pattern id="grid-pat" x="0" y="0" width="32" height="32" patternUnits="userSpaceOnUse">
        <path d="M32 0 L0 0 0 32" fill="none" stroke="rgba(0,255,231,0.06)" stroke-width="0.5"/>
      </pattern>
      <radialGradient id="radar-glow" cx="50%" cy="50%">
        <stop offset="0%" stop-color="#00ffe7" stop-opacity="0.12"/>
        <stop offset="60%" stop-color="#00ffe7" stop-opacity="0.03"/>
        <stop offset="100%" stop-color="#00ffe7" stop-opacity="0"/>
      </radialGradient>
    </defs>
    <rect width="480" height="360" fill="url(#grid-pat)"/>
    <!-- Radar rings -->
    <circle cx="240" cy="180" r="150" fill="url(#radar-glow)" stroke="rgba(0,255,231,0.15)" stroke-width="0.8"/>
    <circle cx="240" cy="180" r="110" fill="none" stroke="rgba(0,255,231,0.1)" stroke-width="0.5"/>
    <circle cx="240" cy="180" r="70"  fill="none" stroke="rgba(0,255,231,0.12)" stroke-width="0.5"/>
    <circle cx="240" cy="180" r="30"  fill="none" stroke="rgba(0,255,231,0.15)" stroke-width="0.5"/>
    <!-- Cross -->
    <line x1="240" y1="30"  x2="240" y2="330" stroke="rgba(0,255,231,0.08)" stroke-width="0.5"/>
    <line x1="90"  y1="180" x2="390" y2="180" stroke="rgba(0,255,231,0.08)" stroke-width="0.5"/>
    <!-- Diagonals -->
    <line x1="134" y1="74"  x2="346" y2="286" stroke="rgba(0,255,231,0.04)" stroke-width="0.4"/>
    <line x1="346" y1="74"  x2="134" y2="286" stroke="rgba(0,255,231,0.04)" stroke-width="0.4"/>
    <!-- Sweep arm -->
    <g style="transform-origin:240px 180px; animation: sweep 4s linear infinite;">
      <path d="M240 180 L240 34 A146,146 0 0,1 358 286 Z" fill="url(#sweep-grad2)" opacity="0.6"/>
      <line x1="240" y1="180" x2="370" y2="80" stroke="rgba(0,255,231,0.4)" stroke-width="1"/>
      <defs>
        <radialGradient id="sweep-grad2" cx="240" cy="180" r="150" gradientUnits="userSpaceOnUse">
          <stop offset="0%" stop-color="#00ffe7" stop-opacity="0.35"/>
          <stop offset="100%" stop-color="#00ffe7" stop-opacity="0"/>
        </radialGradient>
      </defs>
    </g>
    <!-- Contact blips -->
    <circle cx="310" cy="105" r="5" fill="#ff3b3b" opacity="0.9"><animate attributeName="opacity" values="0.9;0.1;0.9" dur="0.8s" repeatCount="indefinite"/></circle>
    <circle cx="170" cy="250" r="4" fill="#ffaa00" opacity="0.8"><animate attributeName="opacity" values="0.8;0.1;0.8" dur="1.4s" repeatCount="indefinite"/></circle>
    <circle cx="340" cy="230" r="4" fill="#ff3b3b" opacity="0.7"><animate attributeName="opacity" values="0.7;0.2;0.7" dur="1.1s" repeatCount="indefinite"/></circle>
    <circle cx="130" cy="130" r="3" fill="#00ffe7" opacity="0.5"><animate attributeName="opacity" values="0.5;0.0;0.5" dur="2s" repeatCount="indefinite"/></circle>
    <!-- Range labels -->
    <text x="244" y="115" font-family="'Share Tech Mono',monospace" font-size="8" fill="rgba(0,255,231,0.3)">60</text>
    <text x="244" y="155" font-family="'Share Tech Mono',monospace" font-size="8" fill="rgba(0,255,231,0.3)">40</text>
    <text x="244" y="195" font-family="'Share Tech Mono',monospace" font-size="8" fill="rgba(0,255,231,0.3)">20</text>
    <!-- Center dot -->
    <circle cx="240" cy="180" r="4" fill="#00ffe7"/>
    <circle cx="240" cy="180" r="10" fill="none" stroke="#00ffe7" stroke-width="0.8" opacity="0.4"/>
    <!-- UI labels -->
    <text x="10" y="20" font-family="'Share Tech Mono',monospace" font-size="9" fill="rgba(0,255,231,0.5)">MILITARY ARSENAL / RADAR</text>
    <text x="10" y="344" font-family="'Share Tech Mono',monospace" font-size="8" fill="rgba(0,255,231,0.25)">SYS.JZX/C1 // MLTR MPG1</text>
    <text x="380" y="20" font-family="'Share Tech Mono',monospace" font-size="8" fill="rgba(0,255,231,0.3)" text-anchor="end">DS 16 / RZX</text>
    <!-- Compass marks -->
    <text x="237" y="22"  font-family="'Share Tech Mono',monospace" font-size="8" fill="rgba(0,255,231,0.4)">N</text>
    <text x="237" y="346" font-family="'Share Tech Mono',monospace" font-size="8" fill="rgba(0,255,231,0.4)">S</text>
    <text x="376" y="184" font-family="'Share Tech Mono',monospace" font-size="8" fill="rgba(0,255,231,0.4)">E</text>
    <text x="94"  y="184" font-family="'Share Tech Mono',monospace" font-size="8" fill="rgba(0,255,231,0.4)">W</text>
  </svg>
</div>

<div class="mission-text">
  <h3>FULL-SPECTRUM <em>DOMINANCE</em> PLATFORM</h3>
  <p>The UNIT-DX9 represents the apex of autonomous combat robotics — a multi-role platform engineered for persistent intelligence gathering, precision strike, and electronic warfare dominance across contested environments.</p>
  <p>Deployable in any theater within 6 hours, the system integrates seamlessly with satellite networks, ground control stations, and allied combat platforms.</p>
  <ul class="mission-list">
    <li>Autonomous target identification and prioritization</li>
    <li>Triple-redundant encrypted communication links</li>
    <li>All-weather, day/night sensor fusion</li>
    <li>Low-observable airframe with radar-absorbing materials</li>
    <li>AI-assisted flight path planning and obstacle avoidance</li>
    <li>Real-time battlefield data relay to command HQ</li>
  </ul>
  <div style="margin-top:28px;">
    <a href="#" class="btn btn-primary">REQUEST BRIEFING</a>
  </div>
</div>

  </div>
</section>

<!-- ─── FOOTER ─── -->

<footer>
  <div class="footer-brand">UNIT-DX9 // COMBAT ROBOTICS DIV.</div>
  <div class="footer-copy">© 2025 — CLASSIFIED // ALL RIGHTS RESERVED // DO NOT DISTRIBUTE</div>
  <ul class="footer-links">
    <li><a href="#">INTEL</a></li>
    <li><a href="#">DEPLOY</a></li>
    <li><a href="#">SECURE COMMS</a></li>
  </ul>
</footer>

<script>
/* ─── UPTIME CLOCK ─── */
(function(){
  const start = Date.now();
  function pad(n){ return String(n).padStart(2,'0'); }
  function tick(){
    const e = Math.floor((Date.now()-start)/1000);
    const h=Math.floor(e/3600), m=Math.floor((e%3600)/60), s=e%60;
    const el = document.getElementById('uptime');
    if(el) el.textContent = pad(h)+':'+pad(m)+':'+pad(s);
    const mt = document.getElementById('mission-time');
    if(mt) mt.textContent = pad(h)+':'+pad(m)+':'+pad(s);
  }
  setInterval(tick, 1000);
  tick();
})();

/* ─── EQ BARS ─── */
(function(){
  const wrap = document.getElementById('eq-bars');
  if(!wrap) return;
  const cols = ['#00ffe7','#00ffe7','#00b8a0','#39ff14','#00ffe7','#39ff14','#00b8a0','#ffaa00','#00ffe7','#00b8a0','#39ff14','#00ffe7','#00b8a0','#00ffe7','#00ffe7','#ffaa00'];
  const bars = [];
  for(let i=0;i<16;i++){
    const b = document.createElement('div');
    b.style.cssText = `flex:1;background:${cols[i]};min-height:2px;transition:height 0.12s ease;`;
    wrap.appendChild(b);
    bars.push(b);
  }
  function animate(){
    bars.forEach(b=>{
      const h = Math.floor(Math.random()*36)+4;
      b.style.height = h+'px';
    });
  }
  setInterval(animate, 120);
  animate();
})();

/* ─── MISSION LOG FEED ─── */
(function(){
  const el = document.getElementById('log-output');
  if(!el) return;
  const msgs = [
    '&gt; SCANNING SECTOR...',
    '&gt; NO CHANGE DETECTED.',
    '&gt; PING SAT-LINK... OK',
    '&gt; UPDATING COORDS...',
    '&gt; TGT-001 CONFIRMED.',
    '&gt; FUEL: 87% NOMINAL.',
    '&gt; ECM STANDBY MODE.',
    '&gt; COMMS CHECK... OK',
    '&gt; AWAITING AUTH CODE.',
    '&gt; ORBIT MAINTAINED.',
  ];
  let i = 0;
  const lines = ['&gt; UNIT DEPLOYED...','&gt; AREA SCAN INIT...','&gt; TARGET LOCKED...','&gt; AWAITING CMD...'];
  setInterval(()=>{
    if(lines.length > 6) lines.shift();
    lines.push(msgs[i % msgs.length]);
    el.innerHTML = lines.join('<br>');
    i++;
  }, 2200);
})();

/* ─── COUNTER ANIMATION ─── */
(function(){
  const counters = document.querySelectorAll('.counter[data-target]');
  const io = new IntersectionObserver(entries => {
    entries.forEach(e => {
      if(!e.isIntersecting) return;
      const el = e.target;
      const target = parseInt(el.getAttribute('data-target'));
      let current = 0;
      const step = Math.max(1, Math.floor(target / 60));
      const interval = setInterval(()=>{
        current = Math.min(current + step, target);
        el.textContent = current.toLocaleString();
        if(current >= target) clearInterval(interval);
      }, 24);
      io.unobserve(el);
    });
  }, { threshold: 0.3 });
  counters
)rawliteral";

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
  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t * _jpg_buf = NULL;
  char * part_buf[64];

  res = streamServer.sendHeader("Access-Control-Allow-Origin", "*");
  streamServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
  streamServer.send(200, "multipart/x-mixed-replace;boundary=123456789000000000000987654321");

  WiFiClient client = streamServer.client();
  
  while (true) {
    if (!client.connected()) break;
    
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Falha na captura da câmera");
      res = ESP_FAIL;
    } else {
      _jpg_buf_len = fb->len;
      _jpg_buf = fb->buf;
    }
    
    if (res == ESP_OK) {
      size_t hlen = snprintf((char *)part_buf, 64, "--123456789000000000000987654321\r\nContent-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n", _jpg_buf_len);
      res = client.write((const char *)part_buf, hlen) ? ESP_OK : ESP_FAIL;
    }
    if (res == ESP_OK) {
      res = client.write((const char *)_jpg_buf, _jpg_buf_len) ? ESP_OK : ESP_FAIL;
    }
    if (res == ESP_OK) {
      res = client.write("\r\n", 2) ? ESP_OK : ESP_FAIL;
    }
    
    if (fb) esp_camera_fb_return(fb);
    if (res != ESP_OK) break;
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
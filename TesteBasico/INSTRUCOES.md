# 🧪 Teste Completo do Robô — Câmera + LED + Movimento (LEIA TUDO ANTES)

> **Pra quem vai testar:** agora é o teste **com tudo junto** — vídeo da câmera,
> LED e os 4 movimentos. Ainda é simples (botões grandes, HTTP). No final tem uma
> **tabela pra você preencher** com o que aconteceu. ~10 minutos. Valeu de novo! 🙏
>
> (O teste anterior já confirmou que os motores giram certo. Agora a estrela é a
> **CÂMERA** — queremos ver se o vídeo aparece e se continua funcionando enquanto
> o robô se move.)

---

## 📦 1. O que você precisa

- A placa **ESP32-CAM** montada no robô (ponte H L298N + 2 motores + a câmera).
- Adaptador/gravador (FTDI ou placa "ESP32-CAM-MB") + cabo USB.
- **Arduino IDE** com suporte a ESP32 (placas Espressif).
- Bateria dos motores **carregada**.

---

## ⬇️ 2. Pegar este código

Está no **Pull Request** que você recebeu, na pasta `TesteBasico/`.

- Baixe/clone o repositório (ou baixe o ZIP da branch do PR).
- Abra **`TesteBasico/TesteBasico.ino`** no Arduino IDE.
  *(Tem que estar numa pasta de mesmo nome `TesteBasico` — já está.)*

---

## 🔌 3. Gravar (flashar) na placa

1. **Ferramentas → Placa → ESP32 → "AI Thinker ESP32-CAM"**.
2. **Ferramentas → Porta →** a porta COM do gravador.
3. Pra gravar a ESP32-CAM: **GPIO 0 no GND** (modo gravação). Na placa
   "ESP32-CAM-MB" basta segurar **BOOT** ao iniciar o upload.
4. Clique em **Upload (→)**.
5. Ao terminar ("Hard resetting..."), **tire o GPIO 0 do GND** (ou solte o BOOT)
   e aperte **RST** pra rodar.

### (Recomendado) Monitor Serial — 115200
Abra **Ferramentas → Monitor Serial** (115200) e aperte **RST**. Deve aparecer:
```
Camera OK.
AP no ar! Conecte no Wi-Fi "ROBO_TESTE" (senha 12345678) e abra: http://192.168.4.1
Servidores HTTP iniciados (pagina :80, video :81).
```
> ⚠️ Se aparecer **"ERRO ao iniciar a camera!"** em vez de "Camera OK.", anota isso
> na tabela — significa que a câmera não inicializou (os motores ainda funcionam).

---

## 📶 4. Conectar e abrir a tela

1. No **celular**: Wi-Fi → conecte em **`ROBO_TESTE`** / senha **`12345678`**.
   *(Pode dizer "sem internet" — normal, ignore e fique conectado.)*
2. Navegador → **`http://192.168.4.1`**
3. Abre a tela **"TESTE ROBÔ — CÂMERA + MOVIMENTO"**: vídeo em cima, botões embaixo.

> ⚠️ Deixe o robô **suspenso** (esteiras no ar) pra não cair da mesa. 🙃

---

## ▶️ 5. Os testes (nesta ordem)

### Teste 1 — VÍDEO 📹
Olhe o quadro de vídeo no topo.
- Aparece a imagem da câmera? Está nítida ou embaçada/travando?
- Embaixo dos botões aparece **"vídeo OK"** ou **"vídeo falhou"**?

### Teste 2 — LED 💡
Aperte **"LIGAR / DESLIGAR LED"** algumas vezes. O LED branco (flash) deve
acender e apagar a cada toque.

### Teste 3 — MOVIMENTO 🚜
Aperte e **segure** cada botão ~2s, depois solte (ao soltar, para):
**FRENTE, RÉ, ESQ, DIR, PARAR.** Anote o que os motores fizeram.

### Teste 4 — VÍDEO + MOVIMENTO JUNTOS ⭐ (o mais importante)
Segure **FRENTE** e, ao mesmo tempo, **olhe o vídeo**: a imagem continua passando
ou ela **congela/trava** quando o robô anda? Esse é o ponto-chave deste teste.

---

## 📝 6. TABELA DE FEEDBACK (preencha e mande de volta)

> Responda do jeito que der. Quanto mais detalhe, melhor.

**Vídeo / câmera:**

| Pergunta | Resposta |
|---|---|
| No Serial apareceu "Camera OK" ou "ERRO ao iniciar a camera"? | |
| O vídeo apareceu na tela? (sim / não / só fundo preto) | |
| Qualidade: nítido / embaçado / travando muito? | |
| Embaixo dos botões mostrou "vídeo OK" ou "vídeo falhou"? | |

**LED:**

| Pergunta | Resposta |
|---|---|
| O LED acende e apaga ao tocar o botão? (sim / não) | |

**Movimento:**

| Botão | Os MOTORES fizeram o quê? |
|---|---|
| FRENTE | |
| RÉ | |
| ESQ | |
| DIR | |
| PARAR | |

**O teste mais importante (vídeo + movimento juntos):**

| Pergunta | Resposta |
|---|---|
| Ao segurar FRENTE, o vídeo continua passando ou trava/congela? | |
| O robô responde rápido ou tem delay grande pra andar? | |

**Geral:**

| Pergunta | Resposta |
|---|---|
| Celular conectou no Wi-Fi numa boa? | |
| A página abriu normal? | |
| Algo estranho (esquentou, reiniciou sozinho, cheiro de queimado)? | |

---

Com essa tabela eu monto o **código definitivo** já ajustado (resolução da câmera,
FPS, etc.). 🚀

---

## 🛠️ Notas técnicas (pra não esquecer)

### Bug: `error: 'function' does not name a type; did you mean 'union'?`

**Sintoma:** ao compilar, erro apontando pra uma linha de JavaScript dentro do HTML
(ex: `function cmd(c){`). Parece erro de C++, mas é JavaScript.

**Causa REAL:** o pré-processador do Arduino gera protótipos de função automáticos e
**não respeita a raw string `R"HTML(...)"`**. Ele lê o `function nome(){` do
JavaScript embutido e cria um protótipo C++ inválido (`function nome(args);`) no topo
do arquivo. O erro aponta pra linha do JS, mas o problema é o protótipo gerado.

**NÃO é** encoding, BOM, nem `R"HTML(` quebrado (isso já foi descartado byte-a-byte).

**Correção:** declarar as funções JavaScript como expressão, não declaração:

```
// quebra a compilação:
function cmd(c){ ... }

// forma correta:
var cmd = function(c){ ... };
```

Funciona igual no navegador. Detalhe: expressão `var` precisa de `;` no fim e não
sofre hoisting (não chamar a função antes da linha onde ela é declarada).

Confirmado compilando de verdade com `arduino-cli` (FQBN `esp32:esp32:esp32cam`,
core esp32 3.3.10) — exit code 0. O mesmo padrão foi corrigido no `CameraWebServer`.

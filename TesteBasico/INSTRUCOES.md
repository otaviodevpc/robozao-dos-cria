# 🧪 Teste Básico do Robô — Instruções (LEIA TUDO ANTES)

> **Pra quem vai testar:** este é um teste **mínimo** pra descobrir onde está o
> problema. **Não tem câmera, não tem vídeo.** Só Wi-Fi + botões + o LED do ESP.
> No final tem uma **tabela pra você preencher** com o que aconteceu — é isso que
> a gente precisa de volta. Demora uns 10 minutos. Valeu! 🙏

---

## 🎯 O que esse teste descobre

A cada botão apertado, o **LED branco (flash) do ESP32 pisca**. Isso é de propósito:

- **LED piscou** = o comando CHEGOU no ESP. Se o robô mesmo assim não mexeu,
  o problema é **ponte H / fiação / bateria dos motores**.
- **LED NÃO piscou** = o comando não chegou. Problema é de **Wi-Fi / conexão**.

Ou seja: já dá pra saber em qual parte está o defeito só de olhar o LED. 💡

---

## 📦 1. O que você precisa

- A placa **ESP32-CAM** já montada no robô (com a ponte H L298N e os 2 motores).
- O adaptador/gravador (FTDI ou a placa "ESP32-CAM-MB") + cabo USB.
- **Arduino IDE** instalado, com o suporte a ESP32 (placas Espressif).
- A bateria dos motores **carregada** (motor fraco/parado quase sempre é bateria).

---

## ⬇️ 2. Pegar este código

Este teste está no **Pull Request** que você recebeu, na pasta `TesteBasico/`.

- Baixe/clone o repositório (ou baixe o ZIP da branch do PR).
- Abra o arquivo **`TesteBasico/TesteBasico.ino`** no Arduino IDE.
  *(O arquivo PRECISA estar dentro de uma pasta com o mesmo nome `TesteBasico` —
  já está, então é só abrir.)*

---

## 🔌 3. Gravar (flashar) na placa

1. No Arduino IDE: **Ferramentas → Placa → ESP32 → "AI Thinker ESP32-CAM"**.
2. **Ferramentas → Porta →** selecione a porta COM do gravador.
3. Para gravar a ESP32-CAM normalmente é preciso **ligar o GPIO 0 no GND**
   (no modo gravação). Se a sua placa é a "ESP32-CAM-MB", ela já faz isso —
   só pode ser que precise segurar o botão **BOOT** ao iniciar o upload.
4. Clique em **Upload (→)**.
5. Quando terminar ("Hard resetting..."), **tire o GPIO 0 do GND** (ou solte o
   BOOT) e aperte o botão **RST** da placa pra ela rodar o programa.

### (Recomendado) Abrir o Monitor Serial
- **Ferramentas → Monitor Serial**, e ajuste a velocidade para **115200**.
- Aperte **RST** na placa. Deve aparecer algo como:
  ```
  AP no ar! Conecte no Wi-Fi "ROBO_TESTE" (senha 12345678) e abra: http://192.168.4.1
  Servidor HTTP iniciado.
  ```
  > Se aparecer isso, o ESP está vivo e o Wi-Fi subiu. ✅
  > A cada botão que você apertar depois, aparece aqui `CMD: ...` — outra forma
  > de confirmar que o comando chegou.

---

## 📶 4. Conectar e abrir a tela

1. No **celular**, vá em Wi-Fi e conecte na rede:
   - **Nome:** `ROBO_TESTE`
   - **Senha:** `12345678`
   - *(O celular pode avisar "sem internet" — é normal, ignore e mantenha conectado.)*
2. Abra o navegador e digite: **`http://192.168.4.1`**
3. Deve abrir uma tela preta escrito **"TESTE ROBÔ — BÁSICO"** com botões grandes.

> ⚠️ Coloque o robô **suspenso** (rodas/esteiras no ar, em cima de um copo/caixa)
> pra ele não sair andando e cair da mesa enquanto você testa. 🙃

---

## ▶️ 5. Os testes (faça nesta ordem e anote)

### Teste 0 — Conexão (botão **PISCAR LED**)
Aperte **"PISCAR LED"**. O LED branco do ESP deve dar uma piscada.
- ✅ Piscou → a conexão está OK, pode seguir.
- ❌ Não piscou → anota e **pula pro fim** (provável problema de Wi-Fi). Veja
  também se na tela aparece "OK: ..." ou "FALHOU: ..." embaixo dos botões.

### Teste 1 — Direções (botões **FRENTE / RÉ / ESQ / DIR / PARAR**)
Aperte e **segure** cada um por uns 2 segundos, depois solte (ao soltar ele para).
Para cada botão, anote: **o LED piscou?** e **os motores fizeram o quê?**

### Teste 2 — Um motor de cada vez (botões **SÓ MOTOR ESQUERDO** / **SÓ MOTOR DIREITO**)
Esses ligam **um motor só**, pra frente, por instante. Servem pra ver se cada
motor funciona sozinho e em que sentido ele gira. Anote qual roda/esteira mexeu
e pra que lado.

---

## 📝 6. TABELA DE FEEDBACK (preencha e mande de volta)

> Responda do jeito que der — "girou", "não fez nada", "girou pro lado errado",
> "só uma esteira andou", "fez barulho mas não mexeu", etc. Quanto mais detalhe melhor.

| Botão                 | LED piscou? (sim/não) | O que os MOTORES fizeram? |
|-----------------------|------------------------|----------------------------|
| PISCAR LED            |        OK                | RODARAM BEM (TUDO OK)          |
| FRENTE                |      OK                  |            OS DOIS RODARA PARA FRENTE                |
| RÉ                    |       OK                 |                     OS DOIS RODARAM PARA TRAZ       |
| ESQ (girar esquerda)  |        OK                |                   O MOTOR DIREITO RODOU PARA TRAZ E O ESQUERO PARA FRENTE        |
| DIR (girar direita)   |          OK              |               O MOTOR DIREITO RODOU PARA FRENTE E O MOTOR ESQUERODO PARA TRAZ              |
| PARAR                 |              OK          |                  PAROU          |
| SÓ MOTOR ESQUERDO     |             OK           |                 SO O MOTOR ESQUERDO RODOU           |
| SÓ MOTOR DIREITO      |            OK            |               SO O MOTOR ESQUERDO RODOU             |

**Perguntas extras (responda no que souber):**

1. Apareceu a mensagem `AP no ar!...` no Monitor Serial? (sim / não / não abri o serial) SIM
2. O celular conectou no Wi-Fi `ROBO_TESTE` numa boa? (sim / com dificuldade / não) SIM
3. A tela `http://192.168.4.1` abriu? (sim / não / abriu mas travada) SIM
4. Embaixo dos botões, aparece **"OK: ..."** ou **"FALHOU: ..."** ao apertar?OK
5. Os motores fazem **barulho/zumbido** mas não giram? (isso costuma ser bateria fraca)NAO
6. A bateria dos motores está carregada e ligada? (sim / não / não sei) SIM
7. Qualquer coisa estranha (esquentou, reiniciou sozinho, cheiro de queimado): NAO_

---

## 🧭 O que cada resultado provavelmente significa (só pra referência)

| Sintoma | Causa mais provável |
|---|---|
| LED não pisca em nada | Wi-Fi/página não conecta — problema de rede |
| LED pisca, mas nenhum motor anda | Bateria dos motores fraca/desligada, ou ponte H sem alimentação |
| Só **um** motor anda | Fio solto/invertido naquele motor, ou canal da ponte H com defeito |
| Motor gira pro **lado errado** | 2 fios daquele motor invertidos (a gente corrige no código depois) |
| Faz zumbido mas não gira | Quase sempre **bateria fraca** (sem força pros motores) |

Com a tabela preenchida eu monto o **código definitivo** já com os ajustes certos. 🚀

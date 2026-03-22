# 🚢 Boat Radar v2 — Simulador de Tráfego Marítimo

[![C](https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white)](https://en.wikipedia.org/wiki/C_(programming_language))
[![CMake](https://img.shields.io/badge/CMake-064F8C?style=for-the-badge&logo=cmake&logoColor=white)](https://cmake.org/)
[![CLion](https://img.shields.io/badge/CLion-000000?style=for-the-badge&logo=jetbrains&logoColor=white)](https://www.jetbrains.com/clion/)

**Simulador de tráfego marítimo em C com 4 tipos de embarcações, sistema de correntes, previsão de colisões e histórico reverso**

> Evolução do projeto [boatRadar](https://github.com/goncaloalegria/boatRadar) — com novos tipos de barcos, listas ligadas, sistema de histórico e funcionalidades avançadas

[Descrição](#-descrição) •
[Funcionalidades](#-funcionalidades) •
[Tipos de Barcos](#-tipos-de-barcos) •
[Arquitetura](#-arquitetura) •
[Instalação](#-instalação) •
[Utilização](#-utilização)

---

## 📋 Descrição

O **Boat Radar v2** é um simulador de tráfego marítimo baseado em grelha, onde embarcações se movimentam de acordo com vetores de direção, velocidade e efeito de correntes marítimas. A simulação avança frame a frame, com deteção automática de colisões, remoção de barcos fora dos limites e persistência do estado em ficheiros de texto.

Este projeto é a evolução do [boatRadar original](https://github.com/goncaloalegria/boatRadar), acrescentando:

- 4 tipos de embarcações com comportamentos distintos (normal, drone, submarino, veleiro)
- Gestão dinâmica de memória com **listas ligadas**
- Sistema de **histórico reverso** por frame
- **Previsão de colisões** até 1000 frames no futuro
- Cálculo de **velocidade média** por embarcação
- Correntes marítimas dinâmicas (alteráveis em runtime)

---

## ✨ Funcionalidades

| Funcionalidade | Descrição |
|---|---|
| 🎬 **Simulação por Frames** | Avanço manual ou automático, frame a frame |
| 🚢 **4 Tipos de Barcos** | Normal, Drone, Submarino e Veleiro com físicas diferentes |
| 🌊 **Correntes Marítimas** | Vetor de corrente que afeta todos os barcos, alterável em runtime |
| 💥 **Deteção de Colisões** | Colisões resolvidas automaticamente a cada frame |
| 🔮 **Previsão de Colisões** | Simulação até 1000 frames no futuro para prever impactos |
| ⏪ **Histórico Reverso** | Voltar a qualquer frame anterior, restaurando todo o estado |
| 📊 **Velocidade Média** | Cálculo de distância e velocidade média por embarcação |
| ➕ **Inserir/Alterar Barcos** | Adicionar novos barcos ou modificar existentes em runtime |
| 💾 **Persistência** | Leitura e gravação do estado da simulação em ficheiros `.txt` |
| 🧹 **Gestão de Memória** | Listas ligadas com libertação completa de memória |

---

## ⛵ Tipos de Barcos

Cada tipo de embarcação tem um comportamento de movimento único:

| Tipo | ID | Comportamento |
|---|---|---|
| **Normal** | 0 | Movimento linear: soma do vetor próprio + corrente |
| **Drone** | 1 | Movimento alternado — horizontal em frames ímpares, vertical em pares. Inverte direção nos limites da grelha |
| **Submarino** | 2 | Movimento normal, mas alterna visibilidade a cada 5 frames. Quando invisível, destrói barcos com que colida sem ser destruído |
| **Veleiro** | 3 | Velocidade horizontal duplicada quando se move para a direita. Inverte direção nos limites em vez de ser removido |

---

## 🏗️ Arquitetura

O projeto usa **listas ligadas** para gestão dinâmica de barcos e histórico de frames.

```
┌──────────────────────────────────────────────────────────────┐
│                        Simulation                             │
│                                                               │
│  grid_width × grid_height · current_angle · current_speed     │
│  current_frame                                                │
├───────────────────────┬───────────────────────────────────────┤
│                       │                                       │
│   boat_list ──►       │   history ──►                         │
│                       │                                       │
│   ┌──────────┐        │   ┌──────────────┐                    │
│   │ BoatNode │──►...  │   │ HistoryNode  │──►...              │
│   │          │        │   │              │                    │
│   │ • id     │        │   │ • boats[]    │                    │
│   │ • x, y   │        │   │ • num_boats  │                    │
│   │ • angle  │        │   │ • frame_nr   │                    │
│   │ • speed  │        │   │ • corrente   │                    │
│   │ • type   │        │   └──────────────┘                    │
│   │ • visible│        │                                       │
│   └──────────┘        │                                       │
└───────────────────────┴───────────────────────────────────────┘
```

### Estruturas de Dados

| Estrutura | Tipo | Função |
|---|---|---|
| `Simulation` | Struct | Estado global da simulação |
| `Boat` | Struct | Dados de uma embarcação |
| `BoatNode` | Lista ligada | Cadeia dinâmica de barcos ativos |
| `HistoryNode` | Lista ligada | Cadeia de snapshots por frame |
| `HistoryBoat` | Struct | Estado de um barco num frame do histórico |
| `Vector` | Struct | Vetor de movimento (dx, dy) |

### Fluxo da Simulação

```
Carregar barcos (antes.txt)
         │
         ▼
┌─────────────────┐
│  A cada frame:   │◄────────────────────┐
│                  │                     │
│  1. Atualizar    │   (posições + tipo) │
│  2. Colisões     │   (deteção/remoção) │
│  3. Fora limites │   (remoção)         │
│  4. Histórico    │   (snapshot)        │
│                  │                     │
└────────┬─────────┘                     │
         │                               │
         ▼                               │
   ┌───────────┐     Avançar frames      │
   │   Menu    │─────────────────────────┘
   │           │
   │ Inserir / Alterar barco
   │ Prever colisões
   │ Histórico reverso
   │ Velocidade média
   │ Alterar corrente
   │ Sair → gravar (depois.txt)
   └───────────┘
```

---

## 💻 Instalação

### Pré-requisitos

| Requisito | Descrição |
|---|---|
| **GCC / MinGW** | Compilador C com suporte a C11 |
| **CMake 3.30+** | Sistema de build |
| **IDE** | CLion (recomendado), VSCode, ou terminal |

### 1. Clonar o Repositório

```bash
git clone https://github.com/goncaloalegria/boatRadar-v2.git
cd boatRadar-v2
```

### 2. Compilar

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### 3. Executar

```bash
./boatRadar-v2 antes.txt 80x80 0 0 0 depois.txt
```

---

## 📱 Utilização

### Argumentos de Linha de Comandos

```
./boatRadar-v2 <input> <dimensão> <ângulo_corrente> <vel_corrente> <frames_auto> <output>
```

| Argumento | Exemplo | Descrição |
|---|---|---|
| `input` | `antes.txt` | Ficheiro com estado inicial dos barcos |
| `dimensão` | `80x80` | Largura × Altura da grelha |
| `ângulo_corrente` | `0` | Direção da corrente (múltiplo de 45°) |
| `vel_corrente` | `0` | Velocidade da corrente |
| `frames_auto` | `5` | Frames a simular antes de abrir o menu (0 = menu direto) |
| `output` | `depois.txt` | Ficheiro onde gravar o estado final |

### Formato do Ficheiro de Input

Cada linha define um barco no formato `id x y ângulo velocidade tipo`:

```
A 10 10 90 2 0
B 50 50 180 3 0
C 30 20 45 1 1
D 60 60 270 2 2
E 5 40 0 4 3
```

### Menu Interativo

```
=== MENU DA SIMULACAO ===
1. Atualizar simulacao        → Avançar N frames
2. Inserir ou alterar barco   → Adicionar/modificar embarcação
3. Previsao de colisoes       → Simular até 1000 frames futuros
4. Rastrear historico reverso → Voltar a um frame anterior
5. Velocidade media de barco  → Estatísticas de uma embarcação
6. Alterar corrente           → Mudar direção/velocidade da corrente
0. Sair                       → Gravar e terminar
```

### Ângulos Válidos

```
        270°
         ↑
  225° ↗   ↖ 315°
180° ←       → 0°
  135° ↘   ↙ 45°
         ↓
        90°
```

---

## 🔄 Diferenças face ao boatRadar v1

| Aspeto | [v1 (boatRadar)](https://github.com/goncaloalegria/boatRadar) | v2 (este projeto) |
|---|---|---|
| **Tipos de barcos** | Básico | 4 tipos com físicas diferentes |
| **Estrutura de dados** | Arrays estáticos | Listas ligadas dinâmicas |
| **Histórico** | Não disponível | Histórico reverso por frame |
| **Previsão** | Não disponível | Previsão de colisões até 1000 frames |
| **Correntes** | Fixas | Alteráveis em runtime |
| **Velocidade média** | Não disponível | Cálculo por embarcação |
| **Gestão de memória** | Stack | Heap com `malloc`/`free` |

---

## 🔧 Resolução de Problemas

| Problema | Solução |
|---|---|
| `Angulo de corrente invalido` | Usar múltiplo de 45 (0, 45, 90, 135, 180, 225, 270, 315) |
| Ficheiro de input não encontrado | Verificar que o `antes.txt` está no Working Directory |
| `depois.txt` vazio | Usar `frames_auto > 0` ou interagir com o menu antes de sair |
| Barcos desaparecem | Barcos fora dos limites são removidos (exceto veleiros no eixo X) |
| Build falha com `-lm` | Usar `target_link_libraries(boatRadar-v2 m)` no CMakeLists |

---

## 📄 Licença

Este projeto está licenciado sob a **MIT License** — veja o ficheiro [LICENSE](LICENSE) para detalhes.

---

## 👤 Autor

- **Gonçalo Alegria** — a22408663 — [@goncaloalegria](https://github.com/goncaloalegria)

---

## 🙏 Agradecimentos

- [Universidade Lusófona](https://www.ulusofona.pt/) — Instituição de ensino
- [CMake](https://cmake.org/) — Sistema de build
- [JetBrains CLion](https://www.jetbrains.com/clion/) — IDE de desenvolvimento
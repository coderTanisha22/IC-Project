# Network Multiplayer Checkers (C, SDL2, TCP Sockets)

This project implements a **two-player networked Checkers game written in C**.  
The graphical interface is built using **SDL2**, and communication between players is handled through **TCP sockets** using a **client–server architecture**.

Two clients connect to a central server. Each client renders the game board locally and sends moves to the server. The server relays moves between the two players so that both boards remain synchronized.

---

# Project Overview

This project demonstrates concepts from multiple areas of systems programming:

- Network programming using TCP sockets  
- Event-driven graphical programming using SDL2  
- Turn-based game logic  
- Client–server communication  
- Non-blocking network input using `select()`

Each player runs a client application that opens a graphical window displaying the board.  
Players interact with the board using mouse clicks.

The server only handles communication and **does not maintain the game state**.

---

# Project Folder Structure

```
project/
│
├── src/
│   ├── client_main.c
│   ├── server_main.c
│   ├── game.c
│   └── ui.c
│
├── assets/
│   └── screenshots of the game
│
├── Makefile
├── README.md
└── .gitignore
```

## Folder Description

**src/**  
Contains all the source code for the project.

- **client_main.c** – Client program responsible for rendering the game window, handling user input, and communicating with the server.  
- **server_main.c** – Server program that accepts two clients and relays moves between them.  
- **game.c** – Implements the core game logic such as board initialization, move validation, captures, and turn handling.  
- **ui.c** – Handles rendering of the board and pieces using SDL2.

**assets/**  
Contains screenshots of the game used in the README for documentation and demonstration.

**Makefile**  
Automates the compilation of the project and generates the client and server executables.

**README.md**  
Provides documentation about the project, how it works, and how to run it.

**.gitignore**  
Specifies files that should not be tracked by Git, such as compiled binaries.

---

# Game Logic

## Board Representation

The board is represented as an **8×8 grid**.

Each cell stores a value representing the piece type:

```
enum { khali, RED, BLACK, RedK, BlackK };
```

Where:

- `khali` = empty square  
- `RED` = red piece  
- `BLACK` = black piece  
- `RedK` = red king  
- `BlackK` = black king  

---

## Initial Board Setup

Pieces are placed on alternating dark squares.

- Rows **0–2** contain black pieces  
- Rows **5–7** contain red pieces  
- Middle rows start empty  

---

## Move Validation

A move is valid if:

1. The destination square is inside the board  
2. The destination square is empty  
3. The move is diagonal  

Movement rules:

- Red pieces move upward  
- Black pieces move downward  
- Kings can move diagonally in both directions  

---

## Capturing

A capture occurs when a piece jumps over an opponent piece.

Conditions:

- Move distance is two diagonal squares  
- The middle square contains an opponent piece  

When captured:

- The opponent piece is removed  
- The moving piece lands in the destination square  

---

## King Promotion

A piece becomes a king when it reaches the opposite end of the board.

- Red piece → becomes king at row `0`  
- Black piece → becomes king at row `7`

Kings are rendered with an additional white circle.

---

# Networking Architecture

The project follows a **client–server architecture**.

```
Client 1  <---->  Server  <---->  Client 2
```

Steps:

1. The server starts and waits for connections.  
2. Two clients connect to the server.  
3. The server randomly assigns roles:
   - Red
   - Black  
4. Clients send moves to the server.  
5. The server forwards moves to the opponent client.

---

# Message Format

Moves are sent as text messages:

```
sr sc dr dc
```

Where:

- `sr` = source row  
- `sc` = source column  
- `dr` = destination row  
- `dc` = destination column  

Example:

```
5 2 4 3
```

Meaning the piece moves from `(5,2)` to `(4,3)`.

---

# Requirements

Required software:

- GCC compiler  
- SDL2 library  
- Linux / Ubuntu  

Install SDL2 on Ubuntu:

```
sudo apt update
sudo apt install libsdl2-dev
```

---

# Building the Project

Compile the project using the Makefile:

```
make
```

This generates two executables:

```
client
server
```

---

# Running the Game

## Step 1: Start the server

```
./server
```

The server will start listening on port **8080**.

---

## Step 2: Start the first client

Open a new terminal and run:

```
./client
```

---

## Step 3: Start the second client

Open another terminal and run:

```
./client
```

The server assigns one player as **Red** and the other as **Black**.

Two game windows will appear, one for each player. Move one window to the side to view the other window behind it.

---

# How to Play

1. Click on one of your pieces to select it.  
2. The selected square will be highlighted.  
3. Click the destination square to move the piece.  
4. The move is validated locally.  
5. The move is sent to the server.  
6. The server forwards the move to the opponent.  
7. Both boards update automatically.

Only the player whose turn it is can move.

---

# Networking Implementation

The server uses `select()` to monitor both client sockets simultaneously.  
This allows the server to detect incoming messages from either client without blocking.

Clients also use `select()` to receive moves without blocking the SDL rendering loop.

---

# Future Improvements

Possible extensions include:

- Server-side rule validation
- Multi-capture moves  
- Move highlighting  
- Game restart option     
- Sound effects  
- Game timers  
- Online matchmaking  
- Remote multiplayer support  

---

# About the Project

This project was built as a **first-year C programming project** aimed at developing a deeper understanding of the **C programming language**, along with practical exposure to **network programming, SDL2-based graphical interfaces, and client–server communication** by implementing a multiplayer Checkers game.
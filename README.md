# **Awale - Server Game Project**


## **Description**
The **Awale** project is a multiplayer game server for the traditional African board game *Awale*. This project allows multiple players to compete against each other in online games. The server manages the game sessions, challenges, observers, and game results.


### **Main Features:**
- Creation and management of public and private challenges between players.
- Management of online games with the possibility of observing ongoing games.
- Tracking of game progress and saving results.
- Socket-based communication between players and the server.


## **Prerequisites**
Before you begin, ensure you have the following:
- **C Compiler**: gcc or any other compatible C compiler.
- **Libraries**: Standard C libraries (stdio.h, stdlib.h, string.h, etc.)
- A **UNIX** system (Linux, macOS, or WSL under Windows).


## **Installation**

### 1. Clone the Repository
To clone the project, use the following command:
git clone https://github.com/cmonluc/awale.git


### 2. Compile the Project
To compile the project, you just have to type "make server" and "make client" to compile the server and the client.


### 3. Run the Server
Once the compilation is finished, run the server with the following command:
./bin/server

The server will listen for connections on the default port 1977. This port can be changed in the code if necessary.

### 4. Client
Once the compilation is finished, run the client with the following command:
./bin/client <address> [pseudo]
You can use the address 127.0.0.1 if you launch all the clients in the same laptop. You can write a pseudo (3 characters alphanumeric minimum) or if you forget to write one, the server will ask you to choose a pseudo. You have 5 attempts, otherwise you will be disconnected from the server.
Clients can connect via terminal and the following menu displays :

1. View all online players
    If the client wants to list all online players (even himself and those who are playing).

2. Challenge a player
    If the client wants to challenge an other client (who is not in game) on a Awale game.
    The client can initiate a challenge, choosing to make it:
        Public: Anyone can view and join the game.
        Private: Only the challenged player can accept the challenge.
    The client also selects an opponent from the list of available players.

3. Modify your bio
    If the client wants to modify his bio.

4. Observe a game
    If the client wants to observe a game in real-time without participating directly. He can sees the public games and the private games only if a player had added him to his friends. When a client observes a game, he can send chats and all the observers will see it (even the players).

5. Chat with other users
    If the client wants to send a message to an other client.

6. Add, remove or list your friends
    If a the client wants to add, remove or list his friends. If a client adds a friend, the friend will be notified but if he removes one, he won't be notified.

0. or Ctrl-C Quit the server
    If the client wants to quit the server.


### The Awale Game
wale is a traditional African board game that is played between two players. The game is played on a board with two rows of six pits, each containing a 4 seeds. The goal is to capture more seeds than your opponent by moving seeds strategically across the board.

Basic Rules:

    Setup: Each player has a row of six pits, and each pit starts with 4 seeds.

    Gameplay: Players take turns. On their turn, a player picks up all the seeds from one of their pits and distributes them one by one in a counterclockwise direction into the other pits. This is called "sowing."

    Capturing Seeds: If a player’s sowing ends in a pit that contains 2 or 3 seeds, those seeds are captured and added to the player's score.

    Goal: The game continues until one player captures a certain number of seeds (typically 25) or when no more moves can be made. The player who captures the most seeds wins.

    Endgame: A player wins when they collect a majority of the seeds, or if the opponent cannot make a valid move.

Awale is a strategic game, requiring careful planning and anticipation of the opponent's moves.


In this sevrer, once the game starts, each player chooses a pit to play. When a player writes something else than a number between 1 and 12, it is considered as message and it is sent to all the observers.
Players can ask to tie (propose a draw during the game).
At the end of a game, players can save their game in a text file containing a summary of the game, including the moves made and the final score, if they want to replay it.




### Project Structure
The project is organized as follows:

/awale
│
├── bin 
|    ├── client
|    ├── server       
├── build 
|    ├── display.o
|    ├── game.o
|    ├── gamelogic.o       
├── interfaces
|    ├── client.h
|    ├── display.h
|    ├── gamelogic.h
|    ├── clientServer.h
|    ├── server.h 
├── savedGames
|    ├── game.txt
├── src 
|    ├── client
|    |    ├── client.c
|    ├── game
|    |    ├── game.c
|    |    ├── display.c
|    |    ├── gamelogic.c
|    ├── server
|    |    ├── server.c        
├── Makefile         
└── README.md        



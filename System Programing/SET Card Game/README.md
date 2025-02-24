# Set Card Game with Multithreading

## Introduction

This project involves implementing a simplified version of the **Set** card game, with a specific focus on **multithreading programming**. The core logic of the game needs to be implemented, while handling multiple tasks simultaneously through threads. The User Interface (UI), graphics, and keyboard handling are already provided, and your task is to implement the game logic, utilizing multithreading to improve performance and responsiveness.

For reference, you can find a description of the original game [here](https://en.wikipedia.org/wiki/Set_(card_game)). Additionally, you can watch a video tutorial on how to play Set (note: our version has slightly different rules).

## Our Version of the Game

The game uses a deck of **81 cards**, each containing four features:

- **Color**: red, green, or purple
- **Number of shapes**: 1, 2, or 3
- **Shape**: squiggle, diamond, or oval
- **Shading**: solid, partial, or empty

### Initial Setup

- The game starts with **12 cards** drawn from the deck, placed in a 3x4 grid.
- Players will attempt to find a combination of **three cards** that form a "legal set."

### Gameplay

- The **dealer** and **players** are the active components in the game.
- Players work together simultaneously to identify legal sets of 3 cards on the table.
- Players place **tokens** on the cards they believe form a legal set. Once the third token is placed, the player asks the dealer to check if the set is legal.
  
  - If the set is **legal**, the dealer:
    - Discards the three cards forming the set.
    - Replaces them with three new cards from the deck.
    - The successful player earns one point and is frozen for a short time.
  
  - If the set is **not legal**, the player receives a penalty and is frozen for a specified time period.

### Reshuffling

If there are no legal sets available on the table, every minute the dealer:
- Collects all the cards on the table.
- Reshuffles the deck and draws new cards.
  
This ensures the game remains dynamic and engaging.

### End of the Game

The game continues until no legal sets can be found in the remaining cards (either on the table or in the deck). When this happens, the game ends, and the player with the most points is declared the winner!

---

## **Multithreading Programming**

### What is Multithreading?

**Multithreading** is a programming technique that allows a program to execute multiple tasks simultaneously, by creating multiple threads. Each thread is a smaller unit of a process that can run concurrently with others. 

In multithreaded programs, tasks that are independent or can be executed in parallel (such as updating the game state, handling user input, and checking for legal sets) can be divided among threads. This allows for **improved performance**, **better responsiveness**, and more efficient use of system resources.

### Why is Multithreading Important?

Multithreading is crucial for building **high-performance applications** that require real-time responsiveness. Some of the key benefits include:

- **Concurrency**: Allowing multiple tasks to run simultaneously makes the application more responsive, which is especially important in games and interactive applications.
- **Efficiency**: By distributing tasks across multiple threads, the application can make better use of CPU resources, reducing idle times.
- **Real-time responsiveness**: In multiplayer games or applications requiring continuous feedback (such as handling user input while performing background tasks), multithreading ensures smooth user interactions.

### How Multithreading is Used in This Project

In this project, multithreading is used to enhance the gameplay experience. The main aspects of the game that benefit from multithreading include:

1. **Simultaneous Gameplay**:
   - Players and the dealer can interact concurrently. While players are searching for sets and placing tokens, the dealer can simultaneously check for legal sets or manage penalties.
   
2. **Game Updates and Reshuffling**:
   - The game logic checks for legal sets continuously, even as the players are interacting with the game. A separate thread is responsible for reshuffling the cards every minute if no legal sets are available on the table.
   
3. **Timers for Penalties**:
   - Players who make incorrect guesses are penalized by being frozen for a short period. This is handled using separate threads that count down the penalty time for each player.

4. **Real-time Interaction**:
   - Player input (placing tokens on cards) is handled in its own thread, ensuring that the game remains interactive while the game logic runs in parallel.

By using multithreading, the game ensures that the different components — such as player actions, game state updates, and dealer actions — run concurrently without blocking one another, resulting in a smoother and more responsive gaming experience.

---

## Features to Implement

1. **Legal Set Logic**: Implement the logic to identify when three cards form a legal set based on the defined rules.
2. **Penalty System**: Implement a penalty mechanism for players who select an illegal set, utilizing separate threads to handle time-based penalties.
3. **Card Replacement**: When a set is found, the corresponding cards are discarded and replaced by new cards, with thread management ensuring smooth transitions.
4. **Deck Management**: Implement reshuffling of the deck if no legal sets are available on the table, utilizing a separate thread to manage this process at regular intervals.
5. **Multithreaded Interaction**: Use threads for handling multiple gameplay components simultaneously (player actions, checking sets, timer management).

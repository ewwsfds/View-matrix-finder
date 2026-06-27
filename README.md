# ViewMatrix Finder

A tool for identifying the correct view matrix inside a game process by comparing computed world-to-screen positions with the actual on-screen position of an entity.

The goal is to determine which candidate view matrix produces screen coordinates that match where the entity is visually rendered.

## Overview

This method works by taking a set of possible view matrix addresses and testing them against known in-game data. It then evaluates which matrix correctly transforms a 3D world position into the expected 2D screen position.

## Requirements

- A list of potential view matrix addresses  
- The world position of at least one visible entity  
- The game’s field of view (FOV)  
- The entity’s current position on the screen  

## How it works

The system uses the provided data to project world coordinates into screen space using each candidate view matrix.

If the resulting screen coordinates closely match the entity’s actual position on the screen, that view matrix is considered a valid match and is stored.

To improve accuracy, the tool also scans memory around each starting view matrix address, checking offsets in the range of -32 to +32 bytes.

## Getting started

To use the tool, populate the global variables at the top of the C++ source file with the required values for your target process.

<img width="1165" height="622" alt="image" src="https://github.com/user-attachments/assets/2878e659-6add-4031-9852-79b8624310c9" />

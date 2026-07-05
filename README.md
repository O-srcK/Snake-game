# WS2812B Snake Game

A Snake game for an **Arduino Uno** driving a **64-pixel WS2812B LED matrix** (8x8).  
The game runs directly on the matrix and includes multiple snake color modes plus win/lose animations.

## Features

- Runs on an Arduino Uno.
- Designed for an **8x8 WS2812B matrix**.
- Linear LED mapping.
- Snake movement with collision detection.
- Apple spawning and growth.
- Win and lose animations.
- Multiple snake color modes:
  - `REGULAR`
  - `GRADIENT`
  - `STRIPED`
- Serial input control.

## Hardware

- Arduino Uno
- WS2812B 8x8 LED matrix
- External 5V power supply recommended for the LED matrix
- Data line connected to `DATA_PIN 6`

## Wiring

### Arduino Uno to WS2812B matrix

- **Data** → `D6`
- **5V** → external 5V supply
- **GND** → common ground between Arduino and LED power supply

## Libraries

This project uses:

- [`FastLED`](https://fastled.io/)

Install it through the Arduino Library Manager before compiling.

## Controls

Use the Serial Monitor to control the game:

- `W` → move up
- `A` → move left
- `S` → move down
- `D` → move right
- `R` → restart game

Set the Serial Monitor baud rate to:

- `9600`

## Game behavior

- The snake starts with one segment.
- The apple is spawned randomly on the board.
- When the snake eats the apple, it grows.
- If the snake hits a wall or bites itself, the lose animation starts.
- If the snake fills the entire board, the win animation starts.

## Color modes

### Regular
The entire snake uses one fixed green color.

### Gradient
Each segment shifts hue slightly from the head toward the tail.

### Striped
The snake alternates between two green-ish tones in bands.

You can change the mode in code with:

```cpp
SnakeColor mode = STRIPED;
```

## Board mapping

This project uses a **linear 8x8 mapping**:

```cpp
uint16_t XY(uint8_t x, uint8_t y) {
    return x + 8 * y;
}
```

That means the matrix is treated as a simple row-major grid.  
This is correct for the specific board used in this project.

## Notes

- The apple spawning logic is intentionally naive because the board is only 64 pixels wide, so practical simplicity is preferred over extra optimization.
- The game uses a full redraw each frame, which keeps the rendering logic clean and makes the color modes easier to manage.
- If you change to a serpentine matrix later, only the `XY()` mapping function needs to be updated.

## How it works

- The snake body is stored as a linked list of body segments.
- Each frame, the game clears the board, draws the apple, then redraws the snake.
- Win and lose animations replace the normal game loop once a terminal state is reached.

## Project structure

```text
snake-game/
├── snake-game.ino
└── README.md
```

## License

Add your preferred license here.

# Arduino Snake – LCD Pixel Display

An Arduino implementation of the classic Snake game. It combines the pixel-based LCD rendering system from one of my Arduino projects with the game logic from my original C Snake project.

The game runs on an **Arduino Uno R3** and uses a **16×2 I2C LCD** as a 16×16 pixel display. The player controls the snake with a joystick.

## Features

- 16×16 game field rendered on a 16×2 LCD
- Joystick controls
- Snake movement and collision detection
- Random apple generation
- Score counter
- Custom LCD characters for pixel rendering

## Hardware

- Arduino Uno R3
- 16×2 I2C LCD
- Analog joystick module
- Push button

## Pin Connections

| Component | Arduino Uno |
|---|---|
| Joystick X-axis | A3 |
| Joystick Y-axis | A2 |
| Exit button | D8 |
| LCD SDA | A4 |
| LCD SCL | A5 |
| LCD VCC | 5V |
| LCD GND | GND |

The exit button uses `INPUT_PULLUP`, so it should connect between D8 and GND.

## Software Requirements

- Arduino IDE
- `LiquidCrystal_I2C` library

## Controls

- Move the joystick to control the snake.
- Press the exit button to stop the game.

## Original Projects

This project combines and adapts code from two of my original repositories:

1. **LCD Pixel Control** – pixel buffer, joystick input and LCD rendering functions.  
   https://github.com/Pavlo-dev2/Arduino/blob/main/LCD_pixel_control.ino

2. **Snake** – original Snake game logic and linked-list implementation in C.  
   https://github.com/Pavlo-dev2/snake

## Adaptations

The original Snake project was written for Linux and used Linux input and framebuffer interfaces.

For this Arduino version:

- Linux input handling was replaced with analog joystick input.
- Linux framebuffer output was replaced with the LCD pixel rendering system.
- The original game logic and linked-list structure were adapted to work in the Arduino environment.
- The code from both projects was combined into a single `.ino` file.

## License

The original Snake repository uses the MIT License. Check the original repositories for the licensing terms of the reused code.

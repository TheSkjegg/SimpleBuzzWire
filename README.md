# Simple Buzz Wire
This repo holds the code and wiring diagram to build a simple Arduino Buzz Wire game with timer and high score as shown in this video:  
https://youtu.be/MsHMBPJExv0?si=4v26Z5gdegQsc1nm

Following parts are required:
  1. Some break or copper pipe (4,7mm diameter) + 2x M2,5 screws
  2. Some copper wire for the start/stop pads and ring/handle (ca 1,7mm diameter)
  3. Seeed Studio XIAO SAMD21
  4. 8 digit 7segment LED display MAX7219 + 2x M3 screws
  5. Regular 5mm red LEDs
  6. Resistors as defined in wiring diagram
  7. Passive buzzer (f.ex ABT-402-RC)
  9. 3D printed parts, as shared here:  https://www.printables.com/model/805640-simple-arduino-buzz-wire-game-with-timer

  **Note:** The code relies on following library's:
  LedControl
  FlashStorage (as the SAMD21 does not have EEPROM)

  Game on!

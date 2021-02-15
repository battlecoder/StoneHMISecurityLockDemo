# Stone Security Lock Demo
This project is a demo application of the [STONE](https://www.stoneitech.com) LCD displays and my [StoneLCDLib](https://github.com/battlecoder/StoneLCDLib) Arduino library.

The goal is to implement a Security Lock panel that will operate an electronic lock once the correct PIN Code is entered, or the right RFID card is used.

![](https://blog.damnsoft.org/wp-content/uploads/2021/02/screenshot.png)

## Components needed

* A [STONE](https://www.stoneitech.com) HMI Display. The assets and project are for a 800x480 screen
* An Arduino board (Uno, Nano or Pro Mini should work withou changes)
* A MFRC-522 RFID Card Reader
* A TTL to RS232 adapter (if using the screen through RS232)
* A 5V Power supply
* An electronic lock *

(*) Currently there's no actual implementation of the electronic lock switching in the code. The Arduino sketch keeps track of whether the lock should be open or closed, but there's only placeholder code for it. Depending on the kind of lock, a driver circuit and its corresponding code may need to be added.
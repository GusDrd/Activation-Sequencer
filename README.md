# Important

The only code written by myself can be found in the `src/main.c` file.  
All other files were either created on compilation or externally provided to use features of the KL25Z board such as its LEDs or accelerometer.

More details explaining the program's strcture can be found directly in the comments.  


# Information

The project uses a serial data link (over USB) to a terminal emulator running on the
connected PC / laptop.
  * The accelerometer is polled every 2 seconds
  * The X, Y and Z acceleration values are converted to proportions of g
  * These values are used to determine in which orientation the board is.
  * Event flags are used in order to signal a change in orientation or a new Flat, Right or Up orientation.
  * An activation sequence requires the board to be held in certain orientations for different amounts of time to trigger the system.
  * If the sequence is not completed correctly, the system will enter an error state and the error type will be printed to the terminal.

The project uses two threads:
  * One determines when a change in orientation occurs.
  * The other one manages the activation sequence of orientations.

The project includes the code for the serial interface. Only one of the API's function is used :  
`sendMsg` which write a message to the terminal

The project includes code for the accelerometer. This API has two functions:  
`initAccel` initialises the accelerometer  
`readXYZ` reads the three accelerations

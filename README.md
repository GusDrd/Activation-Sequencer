# Important

The only code written by myself can be found in the `main.c` file.  
All other files were either created on compilation or externally provided to use features of the KL25Z board such as its LEDs or accelerometer.

More details explaining the program's strcture can be found directly in the comments.  


# Information

The project uses a serial data link (over USB) to a terminal emulator running on the
connected PC / laptop.
  * The accelerometer is polled every 2 seconds
  * The X, Y and Z acceleration values are printed as proportions of g (the acceleration due to gravity)
  * The green LED is toggled on every poll

The project uses a single thread.

The project includes the code for the serial interface. Only one of the API's function is used :  
`sendMsg` which write a message to the terminal

The project includes code for the accelerometer. This API has two functions:  
`initAccel` initialises the accelerometer  
`readXYZ` reads the three accelerations

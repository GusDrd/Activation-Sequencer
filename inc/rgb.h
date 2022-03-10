#ifndef RGB_H
#define RGB_H

// Header for on board RGB led


// Freedom KL25Z LEDs pin numbers
#define RED_LED_POS (18)		// on port B
#define GREEN_LED_POS (19)	// on port B
#define BLUE_LED_POS (1)		// on port D

// Symbols for constants
#define RGB_OFF 0
#define RGB_ON 1

// There are 3 LEDs
typedef enum {RED, GREEN, BLUE} RGB_t ;

// API
void configureRGB(void) ;    // configure all the LEDs using GPIO
void setRGB (RGB_t l, int onOff) ;  // turn an LED on or off

#endif

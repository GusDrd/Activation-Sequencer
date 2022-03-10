/*----------------------------------------------------------------------------
    Header file for shield LEDs
    
    Functions:
      configureLEDs  Configure all the shield LEDs
      ledOnOff       Turn LEDs on or off
 *---------------------------------------------------------------------------*/

#ifndef LED_H
#define LED_H

#include <stdbool.h>
#include <stdint.h>

// ===============
// LEDs 1 - 5
// ===============

// There are 5 LEDs, wired active low
typedef enum {LED1, LED2, LED3, LED4, LED5} LED_t ;

#define LED1pin (0)    /* on port D */
#define LED2pin (2)    /* on port D */
#define LED3pin (3)    // on port D
#define LED4pin (4)    // on port D
#define LED5pin (5)    // on port D

// LED states
#define LED_ON (1)
#define LED_OFF (0)

// API
void configureLEDs(void) ;    // configure all the LEDs using GPIO
void ledOnOff (LED_t l, int onOff) ;  // turn an LED on or off

#endif
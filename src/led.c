/*----------------------------------------------------------------------------
    Shield LEDs
 *---------------------------------------------------------------------------*/
 
#include <MKL25Z4.h>
#include <stdbool.h>
#include "../inc/led.h"
#include "../inc/mask.h"

/* ----------------------------------------
   Configure GPIO output for shield LEDs 
     1. Enable clock to GPIO ports
     2. Enable GPIO ports
     3. Set GPIO direction to output
     4. Ensure LEDs are off
 * ---------------------------------------- */
void configureLEDs(void) {

  // Enable clock to ports D
  SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK;
  
  // Make 5 pins GPIO
  PORTD->PCR[LED1pin] &= ~PORT_PCR_MUX_MASK;          
  PORTD->PCR[LED1pin] |= PORT_PCR_MUX(1);          
  PORTD->PCR[LED2pin] &= ~PORT_PCR_MUX_MASK;          
  PORTD->PCR[LED2pin] |= PORT_PCR_MUX(1);          
  PORTD->PCR[LED3pin] &= ~PORT_PCR_MUX_MASK;          
  PORTD->PCR[LED3pin] |= PORT_PCR_MUX(1);          
  PORTD->PCR[LED4pin] &= ~PORT_PCR_MUX_MASK;          
  PORTD->PCR[LED4pin] |= PORT_PCR_MUX(1);          
  PORTD->PCR[LED5pin] &= ~PORT_PCR_MUX_MASK;          
  PORTD->PCR[LED5pin] |= PORT_PCR_MUX(1);          
  
  // Set ports to outputs
  PTD->PDDR |= MASK(LED1pin) | MASK(LED2pin) | MASK(LED3pin) | MASK(LED4pin) | MASK(LED5pin) ;

  // Turn off LEDs
  PTD->PSOR = MASK(LED1pin) | MASK(LED2pin) | MASK(LED3pin) | MASK(LED4pin) | MASK(LED5pin) ;
}

/*----------------------------------------------------------------------------
  Function that turns LED on or off
 *----------------------------------------------------------------------------*/
const uint32_t ledMask[] = {MASK(LED1pin), MASK(LED2pin), MASK(LED3pin), MASK(LED4pin), MASK(LED5pin)} ;

void ledOnOff (LED_t l, int onOff) {
  if (onOff == LED_ON) {
    PTD->PCOR = ledMask[l] ;
  } else {
    PTD->PSOR = ledMask[l] ;
  }
}

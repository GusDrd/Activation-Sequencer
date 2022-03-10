/*----------------------------------------------------------------------------
    There are two threads
       t_accel: polls the accelerometer every 20 ms.
       t_sequence: manages the orientation sequence of activation.
 *---------------------------------------------------------------------------*/
 
#include "cmsis_os2.h"
#include <MKL25Z4.h>
#include <stdbool.h>
#include "../inc/rgb.h"
#include "../inc/led.h"
#include "../inc/serialPort.h"
#include "../inc/i2c.h"
#include "../inc/accel.h"

#define MASK(x) (1UL << (x))

osEventFlagsId_t errorFlags ;  // Event flags ID


/*--------------------------------------------------------------
 *   Thread t_accel
 *      Read accelarations periodically  
 *      Write results to terminal
 *      Toggle green LED on each poll
 *--------------------------------------------------------------*/
#define INTERMEDIATE 0
#define FLAT 1
#define OVER 2
#define RIGHT 3
#define LEFT 4
#define UP 5
#define DOWN 6

// Used for eventFlags to signal a change of orientation (we can use 2 because OVER is not used as an eventFlag)
#define CHANGE 2
 
osThreadId_t t_accel;      /* id of thread to poll accelerometer */

void accelThread(void *arg) {
	
    int16_t xyz[3] ; // array of values from accelerometer
                     // signed integers in range +8191 (+2g) to -8192 (-2g)
	
    int orientationState = INTERMEDIATE;
    
    // initialise accelerometer
    int aOk = initAccel() ;
    if (aOk) {
        sendMsg("Accel init ok", CRLF) ;
    } else {
        sendMsg("Accel init failed", CRLF) ;
    }
		
    // Loop forever
    while(1) {
        osDelay(20) ;  // delay 20ms
        readXYZ(xyz) ; // read X, Y, Z values
			
        // Convert x, y, z values in the range +/-100
        int16_t x = (xyz[0] * 100) / 4096 ;
        int16_t y = (xyz[1] * 100) / 4096 ;
        int16_t z = (xyz[2] * 100) / 4096 ;

        switch (orientationState) {
					
            // Treat intermediate state to determine new current orientation - Send eventFlag with orientation when needed
            case INTERMEDIATE:
                if(z > 90) {
                    osEventFlagsSet(errorFlags, MASK(FLAT)) ; // Send eventFlag for Flat orientation (Needed for sequence steps 1 & 4)
                    orientationState = FLAT ;
                }else if(z < -90) {
                    orientationState = OVER ;
                }else if(y < -90) {
                    osEventFlagsSet(errorFlags, MASK(RIGHT)) ; // Send eventFlag for Right orientation (Needed for sequence step 2)
                    orientationState = RIGHT ;
                }else if(y > 90) {
                    orientationState = LEFT ;
                }else if(x < -90) {
                    osEventFlagsSet(errorFlags, MASK(UP)) ; // Send eventFlag for Up orientation (Needed for sequence step 3)
                    orientationState = UP ;
                }else if(x > 90) {
                    orientationState = DOWN ;
                }
                break ;
								
            // Treat orientation states to go back to intermediate state - Send eventFlag to signal an orientation change
            case FLAT:
                if(z < 80) {
                    orientationState = INTERMEDIATE ;
                    osEventFlagsSet(errorFlags, MASK(CHANGE)) ;
                }
                break ;
            case OVER:
                if(z > -80) {
                    orientationState = INTERMEDIATE ;
                    osEventFlagsSet(errorFlags, MASK(CHANGE)) ;
                }
                break ; 
            case RIGHT:
                if(y > -80) {
                    orientationState = INTERMEDIATE ;
                    osEventFlagsSet(errorFlags, MASK(CHANGE)) ;
                }
                break ; 
            case LEFT:
                if(y < 80) {
                    orientationState = INTERMEDIATE ;
                    osEventFlagsSet(errorFlags, MASK(CHANGE)) ;
                }
                break ; 
            case UP:
                if(x > -80) {
                    orientationState = INTERMEDIATE ;
                    osEventFlagsSet(errorFlags, MASK(CHANGE)) ;
                }
                break ; 
            case DOWN:
                if(x < 80) {
                    orientationState = INTERMEDIATE ;
                    osEventFlagsSet(errorFlags, MASK(CHANGE)) ;
                }
                break ; 						
        }            
    }
}


/*--------------------------------------------------------------
 *   Thread t_sequence
 *      Regularly waits for eventFlags from t_accel for a change in orientation
 *      Goes through the steps if orientation and time conditions are met
 *      Enable one extra shield LED on each step
 *      Toogle green LED if steps done correctly
 *      Toogle red LED if error in the steps
 *--------------------------------------------------------------*/
#define SEQUENCE_ERROR 0
#define TIME_ERROR 1
#define STEP_ON 2
#define STEP_FLAT 3
#define STEP_RIGHT 4
#define STEP_UP 5
#define TRIGGER 6
 
osThreadId_t t_sequence;      /* id of thread to manage the activation sequence */


void sequenceThread(void *arg) {
	
    int systemState = STEP_ON ;  // Start on first step and wait Flat orientation to start sequence.
	
    uint32_t flags ;  // Determined by osEventFlagWait.
	
    uint32_t timeB ;  // Stores the before time of a step in ms.
    uint32_t timeT ;  // Stores the total time of a step in ms using timeB.
	
    // Loop forever
    while(1) {
        
        switch(systemState) {
					
            /* ------- FIRST STEP ----------------------------------------- */
            case(STEP_ON):
							
                // Wait until Flat orientation is detected to start the sequence.
                flags = osEventFlagsWait(errorFlags, MASK(FLAT), osFlagsWaitAny, osWaitForever) ;
						
                ledOnOff(LED1, LED_ON) ;
                systemState = STEP_FLAT ;
                break;

            /* ------- SECOND STEP ----------------------------------------- */
            case(STEP_FLAT):
                timeB = osKernelGetTickCount();  // Store time before waiting.
						
                // Wait for any change in orientation to confirm step.
                flags = osEventFlagsWait(errorFlags, MASK(CHANGE), osFlagsWaitAny, osWaitForever) ;
						
                timeT = osKernelGetTickCount() - timeB ;  // Store time once orientation has changed.
						
                // Check if the change occured in less or more than 10s (10000ms) - Time Error
                if(timeT < 10000) {
                    ledOnOff(LED1, LED_OFF) ;
                    setRGB(RED, RGB_ON) ;
                    sendMsg("Timing error", CRLF) ;
                    systemState = TIME_ERROR ;
                }else {
									
                    // Leave user 0.5s to complete the detected change and check if new orientation is correct (RIGHT)
                    // Timeout delay is necessary otherwise thread blocks until board is in correct orientation
                    flags = osEventFlagsWait(errorFlags, MASK(RIGHT), osFlagsWaitAny, 500) ;
			
                    // Check if new orientation is correct - Sequence Error
                    if(flags != osFlagsErrorTimeout) {
                        ledOnOff(LED2, LED_ON) ;
                        systemState = STEP_RIGHT ;
                    }else {
                        ledOnOff(LED1, LED_OFF) ;
                        setRGB(RED, RGB_ON) ;
                        sendMsg("Sequence error", CRLF) ;
                        systemState = SEQUENCE_ERROR ;
                    }
                }
                break;
						
            /* ------- THIRD STEP ----------------------------------------- */
            case(STEP_RIGHT):
                timeB = osKernelGetTickCount() ;
						
                // Wait 6s for any change in orientation to confirm step.
                flags = osEventFlagsWait(errorFlags, MASK(CHANGE), osFlagsWaitAny, 6000) ;
						
                timeT = osKernelGetTickCount() - timeB ;
						
                // Check if the change occured between 2s and 6s
                if(flags == osFlagsErrorTimeout || timeT < 2000) {
                    ledOnOff(LED1, LED_OFF) ;
                    ledOnOff(LED2, LED_OFF) ;
                    setRGB(RED, RGB_ON) ;
                    sendMsg("Timing error", CRLF) ;
                    systemState = TIME_ERROR ;
									
                }else if(flags != osFlagsErrorTimeout && timeT >= 2000) {
									
                    // Leave user 0.5s to complete the detected change and check if new orientation is correct (UP)
                    flags = osEventFlagsWait(errorFlags, MASK(UP), osFlagsWaitAny, 500) ;
									
                    // Check if new orientation is correct
                    if(flags != osFlagsErrorTimeout) {
                        ledOnOff(LED3, LED_ON) ;
                        systemState = STEP_UP ;
                    }else {
                        ledOnOff(LED1, LED_OFF) ;
                        ledOnOff(LED2, LED_OFF) ;
                        setRGB(RED, RGB_ON) ;
                        sendMsg("Sequence error", CRLF) ;
                        systemState = SEQUENCE_ERROR ;
                    }
                }
                break;
						
            /* ------- FINAL STEP ----------------------------------------- */
            case(STEP_UP):
                timeB = osKernelGetTickCount() ;
						
                // Wait 8s for any change in orientation to confirm step.
                flags = osEventFlagsWait(errorFlags, MASK(CHANGE), osFlagsWaitAny, 8000) ;
						
                timeT = osKernelGetTickCount() - timeB ;
						
                // Check if the change occured between 4s and 8s
                if(flags == osFlagsErrorTimeout || timeT < 4000) {
                    ledOnOff(LED1, LED_OFF) ;
                    ledOnOff(LED2, LED_OFF) ;
                    ledOnOff(LED3, LED_OFF) ;
                    setRGB(RED, RGB_ON) ;
                    sendMsg("Timing error", CRLF) ;
                    systemState = TIME_ERROR ;
									
                }else if(flags != osFlagsErrorTimeout && timeT >= 4000) {
			
                    // Leave user 0.5s to complete the detected change and check if new orientation is correct (FLAT)
                    flags = osEventFlagsWait(errorFlags, MASK(FLAT), osFlagsWaitAny, 500) ;
									
                    // Check if new orientation is correct
                    if(flags != osFlagsErrorTimeout) {
                        ledOnOff(LED1, LED_OFF) ;
                        ledOnOff(LED2, LED_OFF) ;
                        ledOnOff(LED3, LED_OFF) ;
                        setRGB(GREEN, RGB_ON) ;
                        systemState = TRIGGER ;
                    }else {
                        ledOnOff(LED1, LED_OFF) ;
                        ledOnOff(LED2, LED_OFF) ;
                        ledOnOff(LED3, LED_OFF) ;
                        setRGB(RED, RGB_ON) ;
                        sendMsg("Sequence error", CRLF) ;
                        systemState = SEQUENCE_ERROR ;
                    }
                }
                break;
						
            // Nothing happens in these states, press RESET to restart sequence.
            case(TRIGGER):
                break;
            case(TIME_ERROR):
                break;
            case(SEQUENCE_ERROR):
                break;
        }
    }
}
    

/*----------------------------------------------------------------------------
 * Application main
 *   Initialise I/O
 *   Initialise kernel
 *   Create threads
 *   Start kernel
 *---------------------------------------------------------------------------*/

int main (void) { 
    
    // System Initialization
    SystemCoreClockUpdate() ;

    //configureGPIOinput();
    init_UART0(115200) ;

    // Initialize CMSIS-RTOS
    osKernelInitialize() ;
	
    // Create event flags
    errorFlags = osEventFlagsNew(NULL) ;
    
    // initialise serial port 
    initSerialPort() ;

    // Initialise I2C0 for accelerometer 
    i2c_init() ;
    
    // Initialise GPIO for on-board RGB LED
    configureRGB() ;
		
    // Initialise shield LEDs
    configureLEDs() ;
    
    // Create threads
    t_accel = osThreadNew(accelThread, NULL, NULL);
    t_sequence = osThreadNew(sequenceThread, NULL, NULL);
 
    osKernelStart();    // Start thread execution - DOES NOT RETURN
    for (;;) {}         // Only executed when an error occurs
}

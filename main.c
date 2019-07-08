#include <msp430.h>
#include "driverlib.h"
#include "Board.h"
#include "msp430fr4133.h"
#include "HAL_FR4133LP_LCD.h"
#include "HAL_FR4133LP_Learn_Board.h"


#define MAX_DIST 23200
#define SPEED_OF_SOUND 340
#define COMPARE_VALUE 65535


void setRowsHigh();
void setRowsLow();
void Key();
void poll();

char pressedKey;
int speed;

void main (void)
{
    WDTCTL = WDTPW | WDTHOLD;               // Stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;                   // Disable the GPIO power-on default high-impedance mode
                                            // to activate previously configured port settings

    Init_LCD();                                 //Initialize LCD

    speed = 0;
    LCD_Display_Buttons(1); // SPEED
    LCD_Display_digit(pos6, speed); // 0

    P1DIR |= 0x01;                          // Set P1.0 to output direction
    P4DIR |= 0x01;                          // Set P2.0 to output direction

    P1OUT |= 0x01; // turn on red LED P1.0
    P4OUT &= 0x00; // turn off green LED P4.0

    // ROWS ARE OUTPUTS
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN4);                  // Row 1: Output direction
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN6);                  // Row 2: Output direction
    setRowsLow();

    // COLUMNS ARE ISR TRIGGERS
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1, GPIO_PIN5, GPIO_PRIMARY_MODULE_FUNCTION);     // Column 1: Input direction
    GPIO_selectInterruptEdge(GPIO_PORT_P1, GPIO_PIN5, GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN5);
    GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN5);
    GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN5);

    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P2, GPIO_PIN7, GPIO_PRIMARY_MODULE_FUNCTION);     // Column 2: Input direction
    GPIO_selectInterruptEdge(GPIO_PORT_P2, GPIO_PIN7, GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P2, GPIO_PIN7);
    GPIO_clearInterrupt(GPIO_PORT_P2, GPIO_PIN7);
    GPIO_enableInterrupt(GPIO_PORT_P2, GPIO_PIN7);

    _EINT();        // Start interrupt

    //=================================================================================================
    // glow(); // tight-poll the ultrasonic sensor
    // instead of call function glow(), just put the set up code below:
    // P2.5 trigger
    // P1.3 echo
    // P5.0 LED/motor
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN5);
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN5);
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1, GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
    GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN0);
    GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN0);
    //    while(1)  // tight polling loop, should be replaced by periodic polling interrupt in keypadButton.c
    //        poll();

    //Start timer in continuous mode sourced by SMCLK
        Timer_A_initContinuousModeParam initContParam = {0};
        initContParam.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
        initContParam.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
        initContParam.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_DISABLE;
        initContParam.timerClear = TIMER_A_DO_CLEAR;
        initContParam.startTimer = false;
        Timer_A_initContinuousMode(TIMER_A1_BASE, &initContParam);

        //Initiaze compare mode
        Timer_A_clearCaptureCompareInterrupt(TIMER_A1_BASE,
            TIMER_A_CAPTURECOMPARE_REGISTER_0
            );

        Timer_A_initCompareModeParam initCompParam = {0};
        initCompParam.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_0;
        initCompParam.compareInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE;
        initCompParam.compareOutputMode = TIMER_A_OUTPUTMODE_OUTBITVALUE;
        initCompParam.compareValue = COMPARE_VALUE;
        Timer_A_initCompareMode(TIMER_A1_BASE, &initCompParam);


        Timer_A_startCounter( TIMER_A1_BASE,
                TIMER_A_CONTINUOUS_MODE
        );
    // ==========================================================================================

    PMM_unlockLPM5();           // Need this for LED to turn on- in case of "abnormal off state"
    __bis_SR_register(LPM4_bits + GIE);     // Need this for interrupts or else "abnormal termination"
    __no_operation();           //For debugger
}

void toggle_direction_LEDs(){
    P1OUT ^= 0x01; // toggle red LED P1.0
    P4OUT ^= 0x01; // toggle green LED P4.0
}

void setRowsHigh(){
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN4); // Row 1- HIGH
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN6); // Row 2- HIGH
}

void setRowsLow(){
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN4); // Row 1- LOW
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN6); // Row 2- LOW
}

void Key()
{
        if (GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN5) == GPIO_INPUT_PIN_LOW){     // Column 1 to GND
            GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN4); // Row 1- HIGH
            if (GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN5) == GPIO_INPUT_PIN_HIGH) { // Column 1 to HIGH
//                LCD_Clear();
                if (speed < 6) {
                    speed++;
                }
//                LCD_Display_digit(pos6, speed);
                LCD_Display_battery(battery, speed);
//                LCD_Display_Buttons(1);/
                if (speed == 1) {
                    toggle_direction_LEDs();
                }
            } else {
                GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN6); // Row 2- HIGH
                if (GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN5) == GPIO_INPUT_PIN_HIGH) { // Column 1 to HIGH
                    LCD_Clear();
                    LCD_Display_letter(pos1, 11); // L
                    LCD_Display_letter(pos2, 4); // E
                    LCD_Display_letter(pos3, 5); // F
                    LCD_Display_letter(pos4, 19); // T
                    speed = 0;
                    P1OUT |= 0x01; // turn on red LED P1.0
                    P4OUT &= 0x00; // turn off green LED P4.0
                }
            }
        } else
            if (GPIO_getInputPinValue(GPIO_PORT_P2, GPIO_PIN7) == GPIO_INPUT_PIN_LOW) {     // Column 2 to GND
            GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN4); // Row 1- HIGH
            if (GPIO_getInputPinValue(GPIO_PORT_P2, GPIO_PIN7) == GPIO_INPUT_PIN_HIGH) { // Column 2 to HIGH
                LCD_Clear();
                LCD_Display_letter(pos1, 17); // R
                LCD_Display_letter(pos2, 8); // I
                LCD_Display_letter(pos3, 6); // G
                LCD_Display_letter(pos4, 7); // H
                LCD_Display_letter(pos5, 19); // T
                speed = 0;
                P1OUT |= 0x01; // turn on red LED P1.0
                P4OUT &= 0x00; // turn off green LED P4.0
            } else {
                    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN6); // Row 2- HIGH
                    if (GPIO_getInputPinValue(GPIO_PORT_P2, GPIO_PIN7) == GPIO_INPUT_PIN_HIGH) { // Column 2 to HIGH
                        LCD_Clear();
                        if (speed > -1) {
                            speed--;
                        }
                        if (speed >= 0) {
//                            LCD_Display_digit(pos6, speed);
                            LCD_Display_battery(battery, speed);
                        } else if (speed == -1){
//                          LCD_Display_letter(pos6, 17); // R
                            LCD_Display_R();
                        }
                        if (speed == 0) {
                            toggle_direction_LEDs();
                            LCD_Display_Buttons(18);            //display STOP
                        }
//                        LCD_Display_Buttons(1);
                    }
            }
        }
        setRowsLow();
}

void poll() {
    unsigned long timeElapsed;
    unsigned long pulseWidth;

    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN5);
//    delay EXACTLY 10 microseconds via TimerA or RTC.h library
//    __delay_cycles(160); // 10ms = 1/(16MHz processor)*150 cycles
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN5);

    // poll P1.3 until we read a low
    while (GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN3) == GPIO_INPUT_PIN_LOW);
    timeElapsed = 0;
    // poll P1.3 until we read a high
    while (GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN3) == GPIO_INPUT_PIN_HIGH){
        timeElapsed++;
    };
    pulseWidth = timeElapsed;

    if (pulseWidth < MAX_DIST/1600) { // magic number
        GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN0);
    } else {
        GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN0);
    }
}


#pragma vector = PORT1_VECTOR       // Using PORT1_VECTOR interrupt because P1.5 is in port 1
__interrupt void PORT1_ISR(void)
{
    Key();

    GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN5);
    GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN6);
}

#pragma vector = PORT2_VECTOR       // Using PORT1_VECTOR interrupt because P2.7 is in port 1
__interrupt void PORT2_ISR(void)
{
    Key();

    GPIO_clearInterrupt(GPIO_PORT_P2, GPIO_PIN7);
}

//*****************************************************************************
//This is the TIMER1_A0 interrupt vector service routine.
//******************************************************************************
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER1_A0_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(TIMER1_A0_VECTOR)))
#endif
void TIMER1_A0_ISR (void)
{
    uint16_t compVal = Timer_A_getCaptureCompareCount(TIMER_A1_BASE,
            TIMER_A_CAPTURECOMPARE_REGISTER_0)
            + COMPARE_VALUE;

    //Toggle LED1
//    GPIO_toggleOutputOnPin(
//        GPIO_PORT_LED1,
//        GPIO_PIN_LED1
//        );
    poll();

    //Add Offset to CCR0
    Timer_A_setCompareValue(TIMER_A1_BASE,
        TIMER_A_CAPTURECOMPARE_REGISTER_0,
        compVal
        );
}


/*_____ I N C L U D E S ____________________________________________________*/
#include <stdio.h>

#include "inc_main.h"

#include "misc_config.h"
#include "custom_func.h"

/*_____ D E C L A R A T I O N S ____________________________________________*/

struct flag_32bit flag_PROJ_CTL;
#define FLAG_PROJ_TIMER_PERIOD_1000MS                 	(flag_PROJ_CTL.bit0)
#define FLAG_PROJ_TIMER_PERIOD_SPECIFIC                 (flag_PROJ_CTL.bit1)
#define FLAG_PROJ_TRIG_BTN1                 	        (flag_PROJ_CTL.bit2)
#define FLAG_PROJ_TRIG_BTN2                    		    (flag_PROJ_CTL.bit3)
#define FLAG_PROJ_PWM_DUTY_INC                          (flag_PROJ_CTL.bit4)
#define FLAG_PROJ_PWM_DUTY_DEC                          (flag_PROJ_CTL.bit5)
#define FLAG_PROJ_GET_WIDTH_MEASUREMENT                 (flag_PROJ_CTL.bit6)
#define FLAG_PROJ_GET_PERIOD_MEASUREMENT                (flag_PROJ_CTL.bit7)


#define FLAG_PROJ_WIDTH_HIGH_TRIG                       (flag_PROJ_CTL.bit8)
#define FLAG_PROJ_WIDTH_LOW_TRIG                        (flag_PROJ_CTL.bit9)
#define FLAG_PROJ_REVERSE10                             (flag_PROJ_CTL.bit10)
#define FLAG_PROJ_REVERSE11                             (flag_PROJ_CTL.bit11)
#define FLAG_PROJ_REVERSE12                             (flag_PROJ_CTL.bit12)
#define FLAG_PROJ_REVERSE13                             (flag_PROJ_CTL.bit13)
#define FLAG_PROJ_REVERSE14                             (flag_PROJ_CTL.bit14)
#define FLAG_PROJ_REVERSE15                             (flag_PROJ_CTL.bit15)

/*_____ D E F I N I T I O N S ______________________________________________*/

volatile unsigned int counter_tick = 0;
volatile unsigned int btn_counter_tick = 0;

#define BTN_PRESSED_LONG                                (2500)

#define DUTY_RESOLUTION                                 (1000)
volatile unsigned int pwm_duty[16] = {0};

#define PERIOD_MEASURE_TIME                             (8)
unsigned int g_pulse_width[PERIOD_MEASURE_TIME] = {0};  /* Store pulse width */
unsigned char g_times = 0;                              /* Measurement times counter */
unsigned int  period_total = 0;

#define WIDTH_MEASURE_TIME                              (4)
unsigned char g_times_low = 0;                          /* Measurement times counter (low level width) */
unsigned char g_times_high = 0;                         /* Measurement times counter (high level width) */
unsigned char g_count = WIDTH_MEASURE_TIME;             /* Number of measurement */
volatile unsigned char g_edge_flag = 0xFFU;             /* Edge flag */ 
unsigned char g_port_data[2] = {0U, 0U};                /* Store PULSE level */
unsigned int g_width_low[WIDTH_MEASURE_TIME] = {0U};    /* Store low level width */
unsigned int g_width_high[WIDTH_MEASURE_TIME] = {0U};   /* Store high level width */
unsigned char g_times_invalid = 0x00U;                  /* Invalid edge counter */

unsigned int pulse_high = 0;
unsigned int pulse_low = 0;
unsigned int pulse_total = 0;
// float cal_duty = 0;

// #define ENABLE_AVG

/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/

extern volatile unsigned int g_tau0_ch0_width;

void set_TIMER_PERIOD_SPECIFIC(void)
{
    FLAG_PROJ_TIMER_PERIOD_SPECIFIC = 1;
}

void reset_TIMER_PERIOD_SPECIFIC(void)
{
    FLAG_PROJ_TIMER_PERIOD_SPECIFIC = 0;
}

bool Is_TIMER_PERIOD_SPECIFIC_Trig(void)
{
    return FLAG_PROJ_TIMER_PERIOD_SPECIFIC;
}

void set_TIMER_PERIOD_1000MS(void)
{
    FLAG_PROJ_TIMER_PERIOD_1000MS = 1;
}

void reset_TIMER_PERIOD_1000MS(void)
{
    FLAG_PROJ_TIMER_PERIOD_1000MS = 0;
}

bool Is_TIMER_PERIOD_1000MS_Trig(void)
{
    return FLAG_PROJ_TIMER_PERIOD_1000MS;
}

unsigned int btn_get_tick(void)
{
	return (btn_counter_tick);
}

void btn_set_tick(unsigned int t)
{
	btn_counter_tick = t;
}

void btn_tick_counter(void)
{
	btn_counter_tick++;
    if (btn_get_tick() >= 60000)
    {
        btn_set_tick(0);
    }
}

unsigned int get_tick(void)
{
	return (counter_tick);
}

void set_tick(unsigned int t)
{
	counter_tick = t;
}

void tick_counter(void)
{
	counter_tick++;
    if (get_tick() >= 60000)
    {
        set_tick(0);
    }
}

void delay_ms(unsigned int ms)
{
	#if 1
    unsigned int tickstart = get_tick();
    unsigned int wait = ms;
	unsigned int tmp = 0;
	
    while (1)
    {
		if (get_tick() > tickstart)	// tickstart = 59000 , tick_counter = 60000
		{
			tmp = get_tick() - tickstart;
		}
		else // tickstart = 59000 , tick_counter = 2048
		{
			tmp = 60000 -  tickstart + get_tick();
		}		
		
		if (tmp > wait)
			break;
    }
	
	#else
	TIMER_Delay(TIMER0, 1000*ms);
	#endif
}

void Timer_1ms_IRQ(void)
{
    tick_counter();

    if ((get_tick() % 1000) == 0)
    {
        set_TIMER_PERIOD_1000MS();
    }

    if ((get_tick() % 100) == 0)
    {
        set_TIMER_PERIOD_SPECIFIC();
    }

    if ((get_tick() % 50) == 0)
    {

    }	

    Button_Process_long_counter();
}


/*
    TAU1 PWM : 10K
        - SLAVE 1 : P12 , duty : 100%
*/
unsigned int get_TAU1_pwm_ch_duty(unsigned char ch)
{
	return (pwm_duty[ch]);
}

void set_TAU1_pwm_ch_duty(unsigned char ch ,unsigned int duty)
{
    pwm_duty[ch] = duty;
}

/*copy from R_Config_TAU1_0_Create*/
void generate_TAU1_pwm_ch1(void)
{
    TOM1 |= _0002_TAU_CH1_SLAVE_OUTPUT;
    TOL1 &= (uint16_t)~_0002_TAU_CH1_OUTPUT_LEVEL_L;
    TO1 &= (uint16_t)~_0002_TAU_CH1_OUTPUT_VALUE_1;
    PWMDLY2 &= _FFF3_TAU_CH1_OUTPUT_CLEAR;
    PWMDLY2 |= _0000_TAU_CH1_OUTPUT_NO_DELAY;
    TOE1 |= _0002_TAU_CH1_OUTPUT_ENABLE;
}

void PWM_Process_Adjust(void)
{
    int temp_duty = 0;
    unsigned int duty_hex = 0;

    if (FLAG_PROJ_PWM_DUTY_INC)
    {
        FLAG_PROJ_PWM_DUTY_INC = 0;

        temp_duty = get_TAU1_pwm_ch_duty(1);

        /*
            _0FA0_TAU_TDR11_VALUE : 0x0FA0 (4000) : 100 %
            _0FA0_TAU_TDR11_VALUE : 0x07D0 (2000) : 50 %
            _0FA0_TAU_TDR11_VALUE : 0x0028 (40) : 1 %
        */

        duty_hex = (_0FA0_TAU_TDR11_VALUE / DUTY_RESOLUTION) * 1;   //0.1 %
        temp_duty = (temp_duty >= _0FA0_TAU_TDR11_VALUE) ? (_0FA0_TAU_TDR11_VALUE) : (temp_duty + duty_hex ) ;   

        set_TAU1_pwm_ch_duty(1,temp_duty);
        printf("+duty:0x%02X(%2.2f)\r\n",temp_duty , (float) temp_duty/_0FA0_TAU_TDR11_VALUE*100 );

        TDR11 = get_TAU1_pwm_ch_duty(1);
        generate_TAU1_pwm_ch1();
    }
    if (FLAG_PROJ_PWM_DUTY_DEC)
    {
        FLAG_PROJ_PWM_DUTY_DEC = 0;

        temp_duty = get_TAU1_pwm_ch_duty(1);

        duty_hex = (_0FA0_TAU_TDR11_VALUE / DUTY_RESOLUTION) * 1;   //0.1 %
        temp_duty = (_0FA0_TAU_TDR11_VALUE <= 0) ? (0) : (temp_duty - duty_hex ) ;   

        set_TAU1_pwm_ch_duty(1,temp_duty);
        printf("-duty:0x%02X(%2.2f)\r\n",temp_duty , (float) temp_duty/_0FA0_TAU_TDR11_VALUE*100 );

        TDR11 = get_TAU1_pwm_ch_duty(1);
        generate_TAU1_pwm_ch1();
    }
}


/*
    Set the TI00 pin input valid edge to “Both edge”.
*/
void PulseWidthCapture_Process_in_IRQ(void)    
{
    g_port_data[0] = P1_bit.no7;//P17
    // NOP();
    // NOP();
    // NOP();
    // NOP();
    g_port_data[1] = P1_bit.no7;//P17

    if ((0U == TMIF00) && ((0x01U & g_port_data[0]) == (0x01U & g_port_data[1])))
    {
        g_edge_flag = (g_port_data[0] & 0x01U);    /* Set edge flag */

        if (0U == g_edge_flag)    /* Store high level width */
        {
            #if defined (ENABLE_AVG)
            if (0U < g_times_high)    /* 4 times measurement finished ? */
            {
                g_width_high[g_count - g_times_high] = g_tau0_ch0_width;    /* Store capture value */
                g_times_high--;
            }
            else
            {
                FLAG_PROJ_WIDTH_HIGH_TRIG = 1;
            }
            #else
            pulse_high = g_tau0_ch0_width;
            FLAG_PROJ_WIDTH_HIGH_TRIG = 1;

            #endif
        }
        else    /* Store low level width */
        {
            #if defined (ENABLE_AVG)
            if (0U < g_times_low)     /* 4 times measurement finished ? */
            {
                g_width_low[g_count-g_times_low] = g_tau0_ch0_width;    /* Store capture value */
                g_times_low--;
            }
            else
            {
                FLAG_PROJ_WIDTH_LOW_TRIG = 1;
            }
            #else
            pulse_low = g_tau0_ch0_width;
            FLAG_PROJ_WIDTH_LOW_TRIG = 1;
            #endif
         }
    }
    else     /* Interrupt request is generated */
    {
        g_times_invalid++;      /* Increment counter of invalid edge */
        TMIF00 = 0U;            /* Clear interrupt request */
    }
}

void PulseWidthCapture_Process_logging(void)
{
    if (FLAG_PROJ_GET_WIDTH_MEASUREMENT)
    {
        FLAG_PROJ_GET_WIDTH_MEASUREMENT = 0;
        pulse_total = pulse_high + pulse_low;

        printf("H:%4d(0x%04X),L:%4d(0x%04X),total:%4d,duty:%2.1f\r\n" , 
        pulse_high,pulse_high,
        pulse_low,pulse_low,
        pulse_total,(float) pulse_high/pulse_total*100); 

        // printf("H:%4d(0x%04X),L:%4d(0x%04X)\r\n" ,pulse_high,pulse_high,pulse_low,pulse_low); 

        printf("\r\n");

        pulse_high = 0;
        pulse_low = 0;
        pulse_total = 0;

    }
}

/*
    refer to 
    RL78/G23 
    Timer Array Unit (Pulse Interval Measurement: Width) 
*/
void PulseWidthCapture_Process_in_polling(void) //duty
{
    #if 1
    unsigned short  i = 0;

    if (TMIF00)
    {
        TMIF00 = 0;

        g_times_low = g_count;     /* Set number of measurement */
        g_times_high = g_count;

        #if 1
        do
        {
            /* code */
        } while ( !FLAG_PROJ_WIDTH_HIGH_TRIG && !FLAG_PROJ_WIDTH_LOW_TRIG );

        R_Config_TAU0_0_Stop();    /* Stop timer counter */
        FLAG_PROJ_WIDTH_HIGH_TRIG = 0;
        FLAG_PROJ_WIDTH_LOW_TRIG = 0;
        
        #else
        while(1)
        {
            if ((0U == g_times_low) && (0U == g_times_high))
            {
                R_Config_TAU0_0_Stop();    /* Stop timer counter */
                break;
            }
        }
        #endif

        #if defined (ENABLE_AVG)
        // sum
        for (i = 0 ; i < WIDTH_MEASURE_TIME ; i++)
        {
            pulse_high += g_width_high[i];
            pulse_low += g_width_low[i];          
        }

        // avg
        pulse_high = pulse_high / WIDTH_MEASURE_TIME;
        pulse_low = pulse_low / WIDTH_MEASURE_TIME;
        #endif

        // calculate
        // cal_duty = (float) pulse_high/pulse_total;
        // cal_duty *= 100;

        FLAG_PROJ_GET_WIDTH_MEASUREMENT = 1;

        R_Config_TAU0_0_Start(); 
    }


    #else
    while (0U == TMIF00)
    {
        HALT();    /* Wait first edge detection */
    }
    TMIF00 = 0U;    /* Clear interrupt flag */
    
    g_times_low = g_count;     /* Set number of measurement */
    g_times_high = g_count;
    
    EI();         /* Enable interrupt */

    /* Wait for the measurement to finish */
    while (1U)
    {
        HALT();
        if ((0U == g_times_low) && (0U == g_times_high))
        {
            R_Config_TAU0_0_Stop();    /* Stop timer counter */
        }
    }
    #endif
}

/*
    Set the TI00 pin input valid edge to “Rising edge”. 
*/
void PulsePeriodCapture_Process_in_IRQ(void)    
{
    g_pulse_width[PERIOD_MEASURE_TIME - g_times] = g_tau0_ch0_width;    /* Store capture value */

    if (0U == ( --g_times))        /* 8 times measurement finished ? */
    {
        R_Config_TAU0_0_Stop();    /* Stops timer counter */
    }
}


void PulsePeriodCapture_Process_logging(void)
{
    if (FLAG_PROJ_GET_PERIOD_MEASUREMENT)
    {
        FLAG_PROJ_GET_PERIOD_MEASUREMENT = 0;

        printf("PERIOD:%4d(0x%04X)\r\n" , period_total,period_total);    

        printf("\r\n");
        
        period_total = 0;
    }
}

/*
    refer to 
    RL78/G23 
    Timer Array Unit (Pulse Interval Measurement: Period) 
*/
void PulsePeriodCapture_Process_in_polling(void)
{
    #if 1
    unsigned short  i = 0;

    if (TMIF00)
    {
        TMIF00 = 0;

        g_times = PERIOD_MEASURE_TIME;

        while (0U != g_times);
        
        // sum
        for (i = 0 ; i < PERIOD_MEASURE_TIME ; i++)
        {
            period_total += g_pulse_width[i];          
        }

        // avg
        period_total = period_total / PERIOD_MEASURE_TIME;

        FLAG_PROJ_GET_PERIOD_MEASUREMENT = 1;
    }

    #else
	while (!TMIF00)
	{
	    HALT();     /* Wait first edge detection */
	}
	TMIF00 = 0U;    /* Clear interrupt flag */

	g_times = 8U;   /* Set number of measurement */

	EI();           /* Enable interrupt */

	/* Wait for the measurement to finish */
	while (0U != g_times)
	{
	    HALT();
	}	
    #endif
}

/*
    F24 target board
    LED1 connected to P66, LED2 connected to P67
*/
void LED_Toggle(void)
{
    PIN_WRITE(6,6) = ~PIN_READ(6,6);
    PIN_WRITE(6,7) = ~PIN_READ(6,7);
}

void loop(void)
{
	static unsigned int LOG1 = 0;

    if (Is_TIMER_PERIOD_1000MS_Trig())
    {
        reset_TIMER_PERIOD_1000MS();

        // printf("log(timer):%4d\r\n",LOG1++);
        LED_Toggle();        

    }

    if (Is_TIMER_PERIOD_SPECIFIC_Trig())
    {
        reset_TIMER_PERIOD_SPECIFIC();

        PulseWidthCapture_Process_logging();     
        // PulsePeriodCapture_Process_logging();
    }    

    Button_Process_in_polling();
    
    PulseWidthCapture_Process_in_polling();
    // PulsePeriodCapture_Process_in_polling();
        
    PWM_Process_Adjust();
}


// F24 EVB , P137/INTP0 , set both edge 
void Button_Process_long_counter(void)
{
    if (FLAG_PROJ_TRIG_BTN2)
    {
        btn_tick_counter();
    }
    else
    {
        btn_set_tick(0);
    }
}

void Button_Process_in_polling(void)
{
    static unsigned char cnt = 0;

    if (FLAG_PROJ_TRIG_BTN1)
    {
        FLAG_PROJ_TRIG_BTN1 = 0;
        printf("BTN pressed(%d)\r\n",cnt);

        if (cnt == 0)   //set both edge  , BTN pressed
        {
            FLAG_PROJ_TRIG_BTN2 = 1;
        }
        else if (cnt == 1)  //set both edge  , BTN released
        {
            FLAG_PROJ_TRIG_BTN2 = 0;
        }

        cnt = (cnt >= 1) ? (0) : (cnt+1) ;
    }

    if ((FLAG_PROJ_TRIG_BTN2 == 1) && 
        (btn_get_tick() > BTN_PRESSED_LONG))
    {         
        printf("BTN pressed LONG\r\n");
        btn_set_tick(0);
        FLAG_PROJ_TRIG_BTN2 = 0;
    }
}

// F24 EVB , P137/INTP0
void Button_Process_in_IRQ(void)    
{
    FLAG_PROJ_TRIG_BTN1 = 1;
}

void UARTx_Process(unsigned char rxbuf)
{    
    if (rxbuf > 0x7F)
    {
        printf("invalid command\r\n");
    }
    else
    {
        printf("press:%c(0x%02X)\r\n" , rxbuf,rxbuf);   // %c :  C99 libraries.
        switch(rxbuf)
        {
            case 'a':
            case 'A':
                FLAG_PROJ_PWM_DUTY_INC = 1;
                break;
            case 'd':
            case 'D':
                FLAG_PROJ_PWM_DUTY_DEC = 1;
                break;

            case 'X':
            case 'x':
                RL78_soft_reset(7);
                break;
            case 'Z':
            case 'z':
                RL78_soft_reset(1);
                break;
        }
    }
}

/*
    Reset Control Flag Register (RESF) 
    BIT7 : TRAP
    BIT6 : 0
    BIT5 : 0
    BIT4 : WDCLRF
    BIT3 : 0
    BIT2 : 0
    BIT1 : IAWRF
    BIT0 : LVIRF
*/
void check_reset_source(void)
{
    /*
        Internal reset request by execution of illegal instruction
        0  Internal reset request is not generated, or the RESF register is cleared. 
        1  Internal reset request is generated. 
    */
    unsigned char src = RESF;
    printf("Reset Source <0x%08X>\r\n", src);

    #if 1   //DEBUG , list reset source
    if (src & BIT0)
    {
        printf("0)voltage detector (LVD)\r\n");       
    }
    if (src & BIT1)
    {
        printf("1)illegal-memory access\r\n");       
    }
    if (src & BIT2)
    {
        printf("2)EMPTY\r\n");       
    }
    if (src & BIT3)
    {
        printf("3)EMPTY\r\n");       
    }
    if (src & BIT4)
    {
        printf("4)watchdog timer (WDT) or clock monitor\r\n");       
    }
    if (src & BIT5)
    {
        printf("5)EMPTY\r\n");       
    }
    if (src & BIT6)
    {
        printf("6)EMPTY\r\n");       
    }
    if (src & BIT7)
    {
        printf("7)execution of illegal instruction\r\n");       
    }
    #endif

}

/*
    7:Internal reset by execution of illegal instruction
    1:Internal reset by illegal-memory access
*/
//perform sofware reset
void _reset_by_illegal_instruction(void)
{
    static const unsigned char illegal_Instruction = 0xFF;
    void (*dummy) (void) = (void (*)(void))&illegal_Instruction;
    dummy();
}
void _reset_by_illegal_memory_access(void)
{
    #if 1
    const unsigned char ILLEGAL_ACCESS_ON = 0x80;
    IAWCTL |= ILLEGAL_ACCESS_ON;            // switch IAWEN on (default off)
    *(__far volatile char *)0x00000 = 0x00; //write illegal address 0x00000(RESET VECTOR)
    #else
    signed char __far* a;                   // Create a far-Pointer
    IAWCTL |= _80_CGC_ILLEGAL_ACCESS_ON;    // switch IAWEN on (default off)
    a = (signed char __far*) 0x0000;        // Point to 0x000000 (FLASH-ROM area)
    *a = 0;
    #endif
}

void RL78_soft_reset(unsigned char flag)
{
    switch(flag)
    {
        case 7: // do not use under debug mode
            _reset_by_illegal_instruction();        
            break;
        case 1:
            _reset_by_illegal_memory_access();
            break;
    }
}

// retarget printf
int __far putchar(int c)
{
    // F24 , UART0
    STMK0 = 1U;    /* disable INTST0 interrupt */
    SDR00L = (unsigned char)c;
    while(STIF0 == 0)
    {

    }
    STIF0 = 0U;    /* clear INTST0 interrupt flag */
    return c;
}

void hardware_init(void)
{
    // const unsigned char indicator[] = "hardware_init";
    BSP_EI();
    R_Config_UART0_Start();         // UART , P15 , P16
    R_Config_TAU0_1_Start();        // TIMER
    R_Config_INTC_INTP0_Start();    // BUTTON , P137
    
    R_Config_TAU0_0_Start();        // input capture , P17
    
    R_Config_TAU1_0_Start();        // PWM , 10K , P12

    // check_reset_source();
    printf("%s finish\r\n\r\n",__func__);
}

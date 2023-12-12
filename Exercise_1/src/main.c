
#include <stdint.h>
#include "device_registers.h"
#include "clocks_and_modes.h"


volatile uint8_t mode_led =0;
volatile uint8_t mode_blink = 0;

volatile uint32_t timer_val =20000000;

void NVIC_init_IRQs (void)
{
	//interrupt portC
	//S32_NVIC->ICPR[1] = 1 << (61 % 32);
	S32_NVIC->ISER[1] = 1 << (61 % 32);
	S32_NVIC->IP[61] = 0x9;

	//interrupt LPIT
//	S32_NVIC->ICPR[1] = 1 << (48 % 32);  /* IRQ48-LPIT0 ch0: clr any pending IRQ*/
	S32_NVIC->ISER[1] = 1 << (48 % 32);  /* IRQ48-LPIT0 ch0: enable IRQ */
	S32_NVIC->IP[48] = 0xA;              /* IRQ48-LPIT0 ch0: priority 9 of 0-15*/
}


void PORT_Init(){
//	//init clock PortC
	PCC-> PCCn[PCC_PORTC_INDEX] = PCC_PCCn_CGC_MASK;

	//init interrupt button C12 C13
	PORTC->PCR[12] |=  PORT_PCR_MUX(1); // Mux GPIO
	PORTC->PCR[13] |=  PORT_PCR_MUX(1); // Mux GPIO
	PTC->PDDR &= ~(1<<12); //set input C12
	PTC->PDDR &= ~(1<<13); //set input C13

	//init clock PortD
	PCC-> PCCn[PCC_PORTD_INDEX] = PCC_PCCn_CGC_MASK;

	//init Led D0 D15 D16
	PORTD->PCR[0] |=  PORT_PCR_MUX(1); // Mux GPIO
	PORTD->PCR[15] |=  PORT_PCR_MUX(1); // Mux GPIO
	PORTD->PCR[16] |=  PORT_PCR_MUX(1); // Mux GPIO
	PTD->PDDR |= (1<<0)|(1<<15)|(1<<16); //set output D0 D15 D16


}



void LPIT0_init(){
	//LPIT Clocking:
	PCC->PCCn[PCC_LPIT_INDEX] = PCC_PCCn_PCS(6);    /* Clock Src = 6 (SPLL2_DIV2_CLK)*/
	PCC->PCCn[PCC_LPIT_INDEX] |= PCC_PCCn_CGC_MASK; /* Enable clk to LPIT0 regs 		*/

	/*!
	* LPIT Initialization:
	*/
	LPIT0->MCR |= LPIT_MCR_M_CEN_MASK;  /* DBG_EN-0: Timer chans stop in Debug mode */
									  /* DOZE_EN=0: Timer chans are stopped in DOZE mode */
									  /* SW_RST=0: SW reset does not reset timer chans, regs */
									  /* M_CEN=1: enable module clk (allows writing other LPIT0 regs) */
	LPIT0->MIER = LPIT_MIER_TIE0_MASK;  /* TIE0=1: Timer Interrupt Enabled fot Chan 0 */
	LPIT0->TMR[0].TVAL = 10000000;      /* Chan 0 Timeout period: 40M clocks */

	LPIT0->TMR[0].TCTRL |= LPIT_TMR_TCTRL_T_EN_MASK;
							  /* T_EN=1: Timer channel is enabled */
							 /* CHAIN=0: channel chaining is disabled */
							 /* MODE=0: 32 periodic counter mode */
							 /* TSOT=0: Timer decrements immediately based on restart */
							 /* TSOI=0: Timer does not stop after timeout */
							 /* TROT=0 Timer will not reload on trigger */
							 /* TRG_SRC=0: External trigger soruce */
							 /* TRG_SEL=0: Timer chan 0 trigger source is selected*/
}



int timer_blink(int mode_blink){
	uint32_t time_val;
	switch (mode_blink){
	case 0:
		time_val =0;
		break;
	case 1:
		time_val =20000000;
		break;
	case 2:
		time_val =40000000;
		break;
	case 3:
		time_val =80000000;
		break;

	}
	return time_val;
}

//Toggle LED  b0 r15
void Toogle_blue(){
	PTD->PTOR |= 1<<0;
}

void Toogle_red(){
	PTD->PTOR |= 1<<15;
}

void Toogle_green(){
	PTD->PTOR |= 1<<16;
}

void On_blue(){
	PTD->PDOR &= ~(1<<0);
}

void On_red(){
	PTD->PDOR &= ~(1<<15);
}

void On_green(){
	PTD->PDOR &= ~(1<<16);
}

void clear_led(){
	PTD->PDOR |= (1<<0);
	PTD->PDOR |= (1<<15);
	PTD->PDOR |= (1<<16);
}

void func_blink_led(uint8_t mode_blink , uint8_t mode_led){
	if(mode_blink!=0){
		switch(mode_led){
			case 0:
				Toogle_blue();
				break;
			case 1:
				Toogle_green();
				break;
			case 2:
				Toogle_red();
				break;
			case 3:
				Toogle_red();
				Toogle_blue();
				break;
			case 4:
				Toogle_red();
				Toogle_green();
				break;
			case 5:
				Toogle_blue();
				Toogle_green();
				break;
			case 6:
				Toogle_blue();
				Toogle_green();
				Toogle_red();
				break;
		}
	}

	else{
		switch(mode_led){
			case 0:
				On_blue();
				break;
			case 1:
				On_green();
				break;
			case 2:
				On_red();
				break;
			case 3:
				On_red();
				On_blue();
				break;
			case 4:
				On_red();
				On_green();
				break;
			case 5:
				On_blue();
				On_green();
				break;
			case 6:
				On_blue();
				On_green();
				On_red();
				break;
			}
	}


}


int main() {

	PORT_Init();
	SOSC_init_8MHz();       /* Initialize system oscilator for 8 MHz xtal */
	SPLL_init_160MHz();     /* Initialize SPLL to 160 MHz with 8 MHz SOSC */
	NormalRUNmode_80MHz();  /* Init clocks: 80 MHz sysclk & core, 40 MHz bus, 20 MHz flash */
	NVIC_init_IRQs();
	LPIT0_init();
	//config interrupt
	PORTC->PCR[12] |= PORT_PCR_IRQC(0xA) | PORT_PCR_ISF_MASK;	// IRQ falling edge | Clear ISF flag
	PORTC->PCR[13] |= PORT_PCR_IRQC(0xA) | PORT_PCR_ISF_MASK;	// IRQ falling edge | Clear ISF flag
	clear_led();

	while(1){

	}
	return 0;
}
void PORTC_IRQHandler() {

	if((PORTC->ISFR & (1U << 12)) != 0) { // SW2 nút nhấn
	// Xử lý ngắt SW1 nhấn
		mode_blink++;
		if(mode_blink>3) {
			mode_blink =0;
			func_blink_led(mode_blink, mode_led);
		}
		timer_val = timer_blink(mode_blink);
		LPIT0->TMR[0].TVAL = timer_val;
		PORTC->ISFR |= (1<<12); // Clear cờ ngắt
	}

	if((PORTC->ISFR & (1U << 13)) != 0) { // SW2 nút nhấn
		// Xử lý ngắt SW2 nhấn
		mode_led++;
		if(mode_led>6) mode_led =0;
		clear_led();
		PORTC->ISFR |= (1<<13); // Clear cờ ngắt

	}
}

void LPIT0_Ch0_IRQHandler ()
{
	func_blink_led(mode_blink, mode_led);
	LPIT0->MSR |= LPIT_MSR_TIF0_MASK; /* Clear LPIT0 timer flag 0 */
}

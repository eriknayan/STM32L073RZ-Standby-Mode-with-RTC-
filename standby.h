#include "mbed.h"

void debug_LPM(void){ 
	
		RCC->APB2ENR |= RCC_APB2ENR_DBGMCUEN; // To be able to debug in low power modes
		//DBGMCU->CR |= DBGMCU_CR_DBG_SLEEP;  
		//DBGMCU->CR |= DBGMCU_CR_DBG_STOP; 
		DBGMCU->CR |= DBGMCU_CR_DBG_STANDBY; // Debugger enable in standby mode

}

void config_RTC(int time){
	
		// RTC Reset
		RCC->CSR |= RCC_CSR_RTCRST; 
		RCC->CSR &=~ RCC_CSR_RTCRST; 
	
		// Reset and Clock Control
    RCC->APB1ENR |= RCC_APB1ENR_PWREN; // Power interface clock enabled
    RCC->CSR |= RCC_CSR_RTCSEL_LSE; // Select the RTC clock source: LSE oscillator
		RCC->CSR |= RCC_CSR_LSEON; // Enable the interal LSE oscillator

    /* Enable write access for RTC registers */
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53; 
	
	// RTC configuration
    RTC->CR &=~ RTC_CR_WUTE; // Disable wake up timer to modify it
    while((RTC->ISR & RTC_ISR_WUTWF) != RTC_ISR_WUTWF) { //  Wait until it is allowed to modify wake up reload value 
    }
    RTC->WUTR = time-1; // Wake up value reload counter [s]
    RTC->PRER = 0x7F00FF; // ck_spre = 1Hz PREDIV_A = 0x7F(127) PREDIV_S = 0xFF(255) using LSE (32768Hz) 
    RTC->CR |= RTC_CR_OSEL; // OSEL = 0x3 -> RTC_ALARM output = Wake up timer
    RTC->CR |= RTC_CR_WUCKSEL_2; // WUCKSEL = 0x4 -> RTC Timer [1s - 18h]           
    RTC->CR |= RTC_CR_WUTE | RTC_CR_WUTIE; // Enable wake up counter/interrupt
		
	/* Disable write access for RTC registers */
    RTC->WPR = 0xFE;  
    RTC->WPR = 0x64;  
		
		RCC->CSR |= RCC_CSR_RTCEN; // Re-enable the RTC clock
}

void config_LowPower(void) {
	    			
		// Power configuration  
    //PWR->CSR |= PWR_CSR_EWUP1 | PWR_CSR_EWUP2; // Enable wake-up pins (WUF bit will be set)
    PWR->CR |= PWR_CR_DBP; // Disable backup write protection 
    PWR->CR |= PWR_CR_CWUF; // Clear the WUF flag
		//PWR->CR |= PWR_CR_FWU; // Enable Fast wake-up
    PWR->CR |= PWR_CR_ULP; // V_{REFINT} is off in low-power mode
    PWR->CR |= PWR_CR_PDDS; // Enter Standby mode when the CPU enters deepsleep
		
}

void config_EXTI(void){ // 
	
		NVIC_EnableIRQ(RTC_IRQn); // Enable Interrupt on RTC
		NVIC_SetPriority(RTC_IRQn,0); // Set priority for RTC
		EXTI->IMR |= EXTI_IMR_IM20; // Enable RTC alarm going through EXTI 20 line to NVIC
		//EXTI->EMR |= EXTI_IMR_IM20;
		EXTI->RTSR |= EXTI_IMR_IM20; // Select Rising Edge Trigger 
}

void enter_Standby(void){
	
		// System Control Block
		SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk; // Low power mode -> Deep Sleep
		SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk; // Reenter low-power mode after ISR
}

void standby_mode(int time){

		//debug_LPM();
		config_EXTI();
		config_LowPower();
		config_RTC(time);
		enter_Standby();
		__WFI(); // Waiting for Interruption -> Enter low-power mode
}

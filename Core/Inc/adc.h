#pragma once


#include "interrupt.h"
#include "hysteresis.h"
#include <cmath>

enum {LEAK = 1, CASE_MINUS = 2, CASE_PLUS = 3};

constexpr float k_adc = 3.33 / 4095;
constexpr uint16_t k_div_plus = 200; // делители на плате
constexpr uint16_t k_div_minus = 212;

class ADC_ : TickSubscriber
{
	Interrupt& adc_callback;
	Interrupt& adc_injected_callback;

	uint16_t time_refresh{50};
	uint16_t time{0};

	uint16_t new_case_plus_value{0};
	uint16_t new_case_minus_value{0};
	uint16_t new_leak_value{0};

	uint16_t leak_value{0};

	void adc_injected_interrupt() {

		HAL_ADCEx_InjectedStop_IT (&hadc2);
		new_case_plus_value = HAL_ADCEx_InjectedGetValue(&hadc2, CASE_PLUS);
		new_case_minus_value = HAL_ADCEx_InjectedGetValue(&hadc2, CASE_MINUS);
		new_leak_value = HAL_ADCEx_InjectedGetValue(&hadc2, LEAK);

		case_plus_value += (new_case_plus_value - case_plus_value) / 4;
		case_minus_value += (new_case_minus_value - case_minus_value) / 4;

		leak_value += (new_leak_value - leak_value) / 4;
	}

	using Parent = ADC_;

	struct ADC_INJ_interrupt : Interrupting {
		Parent &parent;
		ADC_INJ_interrupt(Parent &parent) :
				parent(parent) {
			parent.adc_injected_callback.subscribe(this);
		}
		void interrupt() override {
			parent.adc_injected_interrupt();
		}
	} adc_injected_ { *this };

public:

	ADC_(Interrupt& adc_injected_callback, uint16_t time_refresh)
    : adc_callback {adc_callback}
    , adc_injected_callback {adc_injected_callback}
    , time_refresh {time_refresh}
	{
		subscribed = false;
		if (time_refresh > 0)
		  subscribe();
	}

	void notify(){
		if (time++ >= time_refresh) {
		   time = 0;
		   HAL_ADCEx_InjectedStart_IT(&hadc2);
		}

		voltage_plus  = k_adc * case_plus_value  * k_div_plus;
		voltage_minus = k_adc * case_minus_value * k_div_minus;
	}

	void start(){ subscribe()  ;}
	void stop (){ unsubscribe();}

	uint16_t case_plus_value{0};
	uint16_t case_minus_value{0};

	uint16_t voltage_plus{0};
	uint16_t voltage_minus{0};

	uint16_t lk_value(){ return leak_value; }
	uint16_t kz_value(){ return case_minus_value; } // 1653

	Hysteresis<uint16_t, uint16_t> case_minus {voltage_minus, 290, 310};
	Hysteresis<uint16_t, uint16_t> case_plus  {voltage_plus,  280, 300};
//
//	Hysteresis<uint16_t, uint16_t> case_minus {case_minus_value, 2700, 2840}; // 2770 - 2763 2840-2850
//	Hysteresis<uint16_t, uint16_t> case_plus  {case_plus_value, 540, 700, true}; // 630-614 530-540 (3M)
//
//	Hysteresis<uint16_t, uint16_t> case_minus_al {case_value, 3100, 3250}; // 2770 - 2763
//	Hysteresis<uint16_t, uint16_t> case_plus_al  {case_value, 140, 300, true}; // 630-614

	Hysteresis<uint16_t, uint16_t> leak_first_level {leak_value, 350, 400};
	Hysteresis<uint16_t, uint16_t> leak_second_level{leak_value, 600, 680};

};

Interrupt adc_injected_callback;

void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef* hadc){
	if(hadc->Instance == ADC2) //check if the interrupt comes from ACD2
	{
		adc_injected_callback.interrupt();
	}
}


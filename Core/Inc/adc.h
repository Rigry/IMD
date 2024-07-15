#pragma once


#include "interrupt.h"
#include "hysteresis.h"
#include <cmath>

enum {LEAK = 1, CASE = 2};

class ADC_ : TickSubscriber
{
	Interrupt& adc_callback;
	Interrupt& adc_injected_callback;

	uint16_t time_refresh{50};
	uint16_t time{0};

	uint16_t new_case_value{0};
	uint16_t new_leak_value{0};

	uint16_t case_value{0};
	uint16_t leak_value{0};

	void adc_injected_interrupt() {

		HAL_ADCEx_InjectedStop_IT (&hadc2);
		new_case_value = HAL_ADCEx_InjectedGetValue(&hadc2, CASE);
		new_leak_value = HAL_ADCEx_InjectedGetValue(&hadc2, LEAK);

		case_value += (new_case_value - case_value) / 4;
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
	}

	void start(){ subscribe()  ;}
	void stop (){ unsubscribe();}

	uint16_t lk_value(){ return leak_value; }
	uint16_t kz_value(){ return case_value; }

	Hysteresis<uint16_t, uint16_t> case_minus {case_value, 480, 500};
	Hysteresis<uint16_t, uint16_t> case_plus  {case_value, 70, 80, true};

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


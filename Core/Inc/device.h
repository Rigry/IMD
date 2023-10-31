#pragma once

#include "adc.h"
#include "can.h"
#include "interrupt.h"
#include "pin.h"

class Device {

	ADC_& adc;
	CAN<In_id, Out_id>& can;

	Pin& led_red;
	Pin& led_green;
	Pin& on;
	Pin& relay;
	Pin& relay_r;
	Pin& test;
	Pin& norma;
	Pin& fb_norma;
	Pin& first_level;
	Pin& fb_first;
	Pin& second_level;
	Pin& fb_second;
	Pin& KZ_plus;
	Pin& fb_KZ_plus;
	Pin& KZ_minus;
	Pin& fb_KZ_minus;

	Timer timer{2000};
	Timer delay{1500};
	Timer blink{500};

	bool trigger{false};
	bool delay_{true};
	bool check{false};

public:

	Device(ADC_& adc, CAN<In_id, Out_id>& can, Pin& led_red, Pin& led_green, Pin& on, Pin& relay, Pin& relay_r, Pin& test, Pin& norma, Pin& fb_norma
	     , Pin& first_level, Pin& fb_first, Pin& second_level, Pin& fb_second, Pin& KZ_plus, Pin& fb_KZ_plus
	     , Pin& KZ_minus, Pin& fb_KZ_minus)
	: adc {adc}, can {can}, led_red {led_red}, led_green{led_green}, on {on}, relay{relay}, relay_r{relay_r}, test{test}
	, norma{norma}, fb_norma{fb_norma}, first_level{first_level}, fb_first{fb_first}
	, second_level{second_level}, fb_second{fb_second}, KZ_plus{KZ_plus}, fb_KZ_plus{fb_KZ_plus}
	, KZ_minus{KZ_minus}, fb_KZ_minus{fb_KZ_minus}
	{
		relay = false; relay_r = false;
	}



	void operator() (){

		if(delay_ and delay.done()) {
			delay.stop();
			delay_ = false;
		}

		relay_r = true;


		if(can.is_work()) {
			can.outID.state.HV_off = can.inID.control.HV_off;
			can.outID.state.ignition = can.inID.control.ignition;

			if(not delay_ and not can.inID.control.HV_off and can.inID.control.ignition) {

				if(can.inID.control.test) {
					check = true;
				}

				if(check) {
					on = true;
					relay = true;
					timer.stop();
					if(adc.leak_first_level and adc.leak_second_level) {
						can.outID.state.test_OK = true;
						can.outID.state.test_FAILED = false;
						check = false;
					} else {
						can.outID.state.test_OK = false;
						can.outID.state.test_FAILED = true;
						check = false;
					}
				} else {
			//		if(not timer.isCount()) timer.start();
			//		on ^= timer.event();
					relay = false;
				}

				on = true;
				on ? adc.start() : adc.stop();

				can.outID.state.test = check;

				can.outID.state.kz_on_plus = adc.case_plus;
				can.outID.state.kz_on_minus = adc.case_minus;

				can.outID.state.level_first = adc.leak_first_level;
				can.outID.state.level_second = adc.leak_second_level;

				can.outID.state.leak_value = adc.lk_value();
				can.outID.state.kz_value = adc.kz_value();

				led_green = true;
				led_red = false;
			}

		} else {

			if(not delay_) {

				if(test) {
					on = true;
					relay = true;
					timer.stop();
				} else {
	//				if(not timer.isCount()) timer.start();
	//				on ^= timer.event();
					relay = false;
				}

				on = true;

				on ? adc.start() : adc.stop();

				KZ_plus  = adc.case_plus;
				KZ_minus = adc.case_minus;

				first_level  = adc.leak_first_level;
				second_level = adc.leak_second_level;

				if(fb_norma ^ norma) {
					norma = false;
					trigger = true;
				} else if(fb_first ^ first_level) {
					first_level = false;
					trigger = true;
				} else if(fb_second ^ second_level) {
					second_level = false;
					trigger = true;
				} else if(fb_KZ_plus ^ KZ_plus) {
					KZ_plus = false;
					trigger = true;
				} else if(fb_KZ_minus ^ KZ_minus) {
					KZ_minus = false;
					trigger = true;
				} else  {
					trigger = false;
				}

				if(not trigger)
					norma = not (adc.case_plus | adc.case_minus | adc.leak_first_level | adc.leak_second_level);
				else
					norma ^= blink.event();

				led_red = trigger;

				led_green = not led_red;

			}

		}

	}



};

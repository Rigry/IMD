#pragma once

#include <adc.h>
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
	Timer wait;
	Timer wait_kz_plus;
	Timer wait_kz_minus;
	Timer wait_kz_plus_al;
	Timer wait_kz_minus_al;

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
			if(can.outID.state.HV_off == false) {
				led_red = false;
			}
			can.outID.state.ignition = can.inID.control.ignition;

			if(not delay_ and can.inID.control.HV_off and can.inID.control.ignition) {

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

//				can.outID.state.kz_on_plus = adc.case_plus;
//				can.outID.state.kz_on_minus = adc.case_minus;

/////////////////////////
				if(adc.case_plus and not wait_kz_plus.isCount()) {
					wait_kz_plus.start(2'000);
				} else if (not adc.case_plus and wait_kz_plus.isGreater(200)) {
					wait_kz_plus.stop();
					can.outID.state.kz_on_plus = false;
				}

				if(wait_kz_plus.done()) {
					wait_kz_plus.stop();
					can.outID.state.kz_on_plus = true;
				}
////////////////////////
				if (adc.case_minus and not wait_kz_minus.isCount()) {
					wait_kz_minus.start(2'000);
				} else if (not adc.case_minus and wait_kz_minus.isGreater(200)) {
					wait_kz_minus.stop();
					can.outID.state.kz_on_minus = false;
				}

				if (wait_kz_minus.done()) {
					wait_kz_minus.stop();
					can.outID.state.kz_on_minus = true;
				}
///////////////////////
				if (adc.case_plus_al and not wait_kz_plus_al.isCount()) {
					wait_kz_plus_al.start(3'000);
				} else if (not adc.case_plus_al and wait_kz_plus_al.isGreater(200)) {
					wait_kz_plus_al.stop();
					can.outID.state.kz_on_plus_al = false;
				}

				if (wait_kz_plus_al.done()) {
					wait_kz_plus_al.stop();
					can.outID.state.kz_on_plus_al = true;
				}
////////////////////////
				if (adc.case_minus_al and not wait_kz_minus_al.isCount()) {
					wait_kz_plus_al.start(3'000);
				} else if (not adc.case_minus_al and wait_kz_plus_al.isGreater(200)) {
					wait_kz_minus_al.stop();
					can.outID.state.kz_on_minus_al = false;
				}

				if (wait_kz_minus_al.done()) {
					wait_kz_minus_al.stop();
					can.outID.state.kz_on_minus_al = true;
				}
///////////////////////

				can.outID.state.level_first = adc.leak_first_level;

				if(adc.leak_second_level and not wait.isCount()) {
					wait.start(5'000);
				} else if (not adc.leak_second_level and wait.isGreater(200)) {
					wait.stop();
					can.outID.state.level_second = false;
				}

				if(wait.done()) {
					wait.stop();
					can.outID.state.level_second = true;
				}

				can.outID.state.leak_value_1 = adc.lk_value() & 0b0000000011111111;
				can.outID.state.leak_value_0 = (adc.lk_value() >> 8);
				can.outID.state.kz_value_1 = adc.kz_value() & 0b0000000011111111;
				can.outID.state.kz_value_0 = (adc.kz_value() >> 8);

				led_green = true;
				led_red = false;
			} else {
				can.outID.state.kz_on_plus = false;
				can.outID.state.kz_on_minus = false;
				can.outID.state.kz_on_plus_al = false;
				can.outID.state.kz_on_minus_al = false;

				can.outID.state.level_first = false;
				can.outID.state.level_second = false;
				wait.stop();
				wait_kz_minus_al.stop();
				wait_kz_plus_al.stop();
				wait_kz_minus.stop();
				wait_kz_plus.stop();

				can.outID.state.leak_value_1 = adc.lk_value() & 0b0000000011111111;
				can.outID.state.leak_value_0 = (adc.lk_value() >> 8);
				can.outID.state.kz_value_1 = adc.kz_value() & 0b0000000011111111;
				can.outID.state.kz_value_0 = (adc.kz_value() >> 8);
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

#pragma once

#include <adc.h>
#include "can.h"
#include "interrupt.h"
#include "pin.h"

constexpr float k_adc = 3.33 / 4095;
constexpr uint16_t k_div_plus = 200; // делители на плате
constexpr uint16_t k_div_minus = 212;

class Device {

	enum State {delaying, pause, work_troll, work_electro, test_electro_plus, test_electro_minus, testing} state{delaying};

	ADC_& adc;
	CAN<In_id, Out_id, Out_id_2>& can;

	Pin& led_red;
	Pin& led_green;
	Pin& on;
	Pin& relay;
	Pin& shunt_plus;
	Pin& shunt_minus;
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
	Timer delay{10'000};
	Timer blink{500};
	Timer wait;
	Timer wait_kz_plus;
	Timer wait_kz_minus;
	Timer wait_kz_plus_al;
	Timer wait_kz_minus_al;
	Timer shunt_on;
	Timer measure_on;

	bool trigger{false};
//	bool delay_{true};
	bool check{false};
	bool need_check{false};
	bool plus{false};
	bool minus{false};
	bool kz{false};
	bool first_measury{true};
	bool work{false};

	uint16_t resistance_plus{0};
	uint16_t resistance_minus{0};

	uint16_t voltage_plus{0};
	uint16_t voltage_minus{0};
	uint16_t voltage_shunt{0};
	uint16_t voltage_measury{0};

public:

	Device(ADC_& adc, CAN<In_id, Out_id, Out_id_2>& can, Pin& led_red, Pin& led_green, Pin& on, Pin& relay, Pin& shunt_plus, Pin& shunt_minus, Pin& test, Pin& norma, Pin& fb_norma
	     , Pin& first_level, Pin& fb_first, Pin& second_level, Pin& fb_second, Pin& KZ_plus, Pin& fb_KZ_plus
	     , Pin& KZ_minus, Pin& fb_KZ_minus)
	: adc {adc}, can {can}, led_red {led_red}, led_green{led_green}, on {on}, relay{relay}, shunt_plus{shunt_plus}, shunt_minus{shunt_minus}, test{test}
	, norma{norma}, fb_norma{fb_norma}, first_level{first_level}, fb_first{fb_first}
	, second_level{second_level}, fb_second{fb_second}, KZ_plus{KZ_plus}, fb_KZ_plus{fb_KZ_plus}
	, KZ_minus{KZ_minus}, fb_KZ_minus{fb_KZ_minus}
	{
		relay = false; shunt_plus = false; shunt_minus = false;
		measure_on.time_set = 3'000;
	}



	void operator() (){

		voltage_plus  = k_adc * adc.case_plus_value * k_div_plus;
		voltage_minus = k_adc * adc.case_minus_value * k_div_minus;
//		voltage_plus = 300;
//		voltage_minus = 276;

		if(can.is_work()) {
			can.outID.state.test = check;
			can.outID.state.HV_off = can.inID.control.HV_off;
			can.outID.state.ignition = can.inID.control.ignition;
			can.outID.state.level_first = adc.leak_first_level;
			can.outID.state.electrobus = can.inID.control.electrobus;

			can.outID.state.leak_value_1 = adc.lk_value() & 0b0000000011111111;
			can.outID.state.leak_value_0 = (adc.lk_value() >> 8);
			can.outID.state.kz_plus_value_1 = resistance_plus & 0b0000000011111111;
			can.outID.state.kz_plus_value_0 = (resistance_plus >> 8);
			can.outID.state.kz_minus_value_1 = resistance_minus & 0b0000000011111111;
			can.outID.state.kz_minus_value_0 = (resistance_minus >> 8);

			can.outID_2.id_2.u_plus_value_1 = voltage_plus & 0b0000000011111111;
			can.outID_2.id_2.u_plus_value_0 = (voltage_plus >> 8);
			can.outID_2.id_2.u_minus_value_1 = voltage_minus & 0b0000000011111111;
			can.outID_2.id_2.u_minus_value_0 = (voltage_minus >> 8);


			led_green = true;
			led_red = false;
		} else {
			can.outID.state.kz_on_plus = false;
			can.outID.state.kz_on_minus = false;

			can.outID.state.level_first = false;
			can.outID.state.level_second = false;
			wait.stop();
			shunt_minus = false;
			shunt_plus = false;
			need_check = false;
			shunt_on.stop();
			measure_on.stop();

			can.outID.state.leak_value_1 = adc.lk_value() & 0b0000000011111111;
			can.outID.state.leak_value_0 = (adc.lk_value() >> 8);
			can.outID.state.kz_plus_value_1 = resistance_plus & 0b0000000011111111;
			can.outID.state.kz_plus_value_0 = (resistance_plus >> 8);
			can.outID.state.kz_minus_value_1 = resistance_minus & 0b0000000011111111;
			can.outID.state.kz_minus_value_0 = (resistance_minus >> 8);

			can.outID_2.id_2.u_plus_value_1 = voltage_plus & 0b0000000011111111;
			can.outID_2.id_2.u_plus_value_0 = (voltage_plus >> 8);
			can.outID_2.id_2.u_minus_value_1 = voltage_minus & 0b0000000011111111;
			can.outID_2.id_2.u_minus_value_0 = (voltage_minus >> 8);

			led_green = false;
			led_red = true;
		}

//		on ? adc.start() : adc.stop();
		adc.start();

		if (not can.inID.control.HV_off) {

			kz = false;
			can.outID.state.kz_on_plus = false;
			can.outID.state.kz_on_minus = false;

			can.outID.state.level_first = false;
			can.outID.state.level_second = false;

			state = delaying;
		}

		switch (state) {
		case delaying:
			if (delay.done() or work) {
				work = true;
				delay.stop();
				if(can.inID.control.HV_off and can.inID.control.ignition) {
					if(can.inID.control.electrobus) {
						need_check = true;
						state = test_electro_plus;
						if (not shunt_on.isCount()) {
							shunt_on.start(3'000);
						}
					} else
						state = work_troll;
				}
			}
			break;
		case work_troll:
			if(can.inID.control.HV_off and can.inID.control.ignition) {
				on = true;
				if(can.inID.control.test) {
					check = true;
					state = testing;
				} else {
					relay = false;
				}

				if (adc.leak_second_level and not wait.isCount()) {
					wait.start(5'000);
				} else if (not adc.leak_second_level and wait.isGreater(200)) {
					wait.stop();
					can.outID.state.level_second = false;
				}

				if (wait.done()) {
					wait.stop();
					can.outID.state.level_second = true;
				}
			}
			break;
		case test_electro_plus:
			if(can.inID.control.HV_off and can.inID.control.ignition) {
				if (shunt_on.done() and need_check) {
					shunt_on.stop();
					shunt_plus = true;
					voltage_measury = voltage_plus;
					if (not measure_on.isCount())
						measure_on.start(3'000);
				}
				if (shunt_plus) {
					voltage_shunt = voltage_plus;
					resistance_minus = (abs(voltage_measury - voltage_shunt) * 80) / (voltage_shunt + 1); // 80 - 800 k shunt
				}
				if (measure_on.done()) {
					shunt_minus = false;
					shunt_plus = false;
					measure_on.stop();
					if (not shunt_on.isCount()) {
						shunt_on.start(3'000);
					}
					state = test_electro_minus;
				}

			}
			break;
		case test_electro_minus:
			if(can.inID.control.HV_off and can.inID.control.ignition) {
				if (shunt_on.done() and need_check) {
					shunt_on.stop();
					shunt_minus = true;
					voltage_measury = voltage_minus;
					if (not measure_on.isCount())
						measure_on.start(3'000);
				}
				if (shunt_minus) {
					voltage_shunt = voltage_minus;
					resistance_plus = (abs(voltage_measury - voltage_shunt) * 80) / (voltage_shunt + 1);
				}
				if (measure_on.done()) {
					shunt_minus = false;
					shunt_plus = false;
					need_check = false;
					measure_on.stop();
					state = pause;
					if(not shunt_on.isCount()) {
						shunt_on.start(5'000);
					}
//					state = work_electro;
				}
			}
			break;
		case pause:
			if(shunt_on.done()) {
				shunt_on.stop();
				state = work_electro;
			}
			break;
		case work_electro:
			if(can.inID.control.HV_off and can.inID.control.ignition) {
				if (abs(voltage_plus - voltage_minus) > 80 and not need_check and not kz) {

					need_check = true;
					first_measury = false;
					if(not shunt_on.isCount()) {
						shunt_on.start(3'000);
					}

				} else if (abs(voltage_plus - voltage_minus) < 80) {
					shunt_on.stop();
					need_check = false;
//					shunt_plus = false;
//					shunt_minus = false;
//					can.outID.state.kz_on_plus = false;
//					can.outID.state.kz_on_minus = false;
				}


				if (need_check and shunt_on.done()) {

					plus = voltage_plus > voltage_minus;
					minus = voltage_minus > voltage_plus;

					shunt_on.stop();
					kz = true;
					if (plus and not shunt_minus) {
						shunt_plus = true;
						can.outID.state.kz_on_minus = true;
						voltage_measury = plus ? voltage_plus : voltage_minus;
//						voltage_measury = voltage_minus;
//						voltage_shunt = voltage_plus;
						if(not measure_on.isCount()) {

							measure_on.start(3'000);

						}

					} else if (minus and not shunt_plus) {
						shunt_minus = true;
						can.outID.state.kz_on_plus = true;
						voltage_measury = plus ? voltage_plus : voltage_minus;
//						voltage_measury = voltage_plus;
//						voltage_shunt = voltage_minus;
						if(not measure_on.isCount())
							measure_on.start(3'000);
					}

				}


				if (shunt_minus) {
					voltage_shunt = voltage_minus;
					resistance_plus = (abs(voltage_measury - voltage_shunt) * 80) / (voltage_shunt + 1);
				}
				if (shunt_plus) {
					voltage_shunt = voltage_plus;
					resistance_minus = (abs(voltage_measury - voltage_shunt) * 80) / (voltage_shunt + 1); // 80 - 800 k shunt
				}

				if(measure_on.done()) {
					shunt_minus = false;
					shunt_plus = false;
					need_check = false;
					measure_on.stop();
				}


			}
			break;
		case testing:
			if (check) {
				relay = true;
				if (adc.leak_first_level and adc.leak_second_level) {
					can.outID.state.test_OK = true;
					can.outID.state.test_FAILED = false;
					check = false;
				} else {
					can.outID.state.test_OK = false;
					can.outID.state.test_FAILED = true;
					check = false;
				}
				state = work_troll;
			}
			break;
		} // switch (state) {
	}
//				can.outID.state.kz_on_plus = adc.case_plus;
//				can.outID.state.kz_on_minus = adc.case_minus;
//			} // if(not delay_ and can.inID.control.HV_off and can.inID.control.ignition) {

//		} //if(can.is_work()) {
};

//				can.outID.state.leak_value_1 = voltage_plus & 0b0000000011111111;
//				can.outID.state.leak_value_0 = (voltage_plus >> 8);
//				can.outID.state.kz_value_1 = voltage_minus & 0b0000000011111111;
//				can.outID.state.kz_value_0 = (voltage_minus >> 8);




//				can.outID.state.leak_value_1 = voltage_plus & 0b0000000011111111;
//				can.outID.state.leak_value_0 = (voltage_plus >> 8);
//				can.outID.state.kz_value_1 = voltage_minus & 0b0000000011111111;
//				can.outID.state.kz_value_0 = (voltage_minus >> 8);

//else {
//
//			if(not delay_) {
//
//				if(test) {
//					on = true;
//					relay = true;
//					timer.stop();
//				} else {
//	//				if(not timer.isCount()) timer.start();
//	//				on ^= timer.event();
//					relay = false;
//				}
//
//				on = true;
//
//				on ? adc.start() : adc.stop();
//
//				KZ_plus  = adc.case_plus;
//				KZ_minus = adc.case_minus;
//
//				first_level  = adc.leak_first_level;
//				second_level = adc.leak_second_level;
//
//				if(fb_norma ^ norma) {
//					norma = false;
//					trigger = true;
//				} else if(fb_first ^ first_level) {
//					first_level = false;
//					trigger = true;
//				} else if(fb_second ^ second_level) {
//					second_level = false;
//					trigger = true;
//				} else if(fb_KZ_plus ^ KZ_plus) {
//					KZ_plus = false;
//					trigger = true;
//				} else if(fb_KZ_minus ^ KZ_minus) {
//					KZ_minus = false;
//					trigger = true;
//				} else  {
//					trigger = false;
//				}
//
//				if(not trigger)
//					norma = not (adc.case_plus | adc.case_minus | adc.leak_first_level | adc.leak_second_level);
//				else
//					norma ^= blink.event();
//
//				led_red = trigger;
//
//				led_green = not led_red;
//
//			}



///////////////////////
//				if (adc.case_plus_al and not wait_kz_plus_al.isCount()) {
//					wait_kz_plus_al.start(3'000);
//				} else if (not adc.case_plus_al and wait_kz_plus_al.isGreater(200)) {
//					wait_kz_plus_al.stop();
//					can.outID.state.kz_on_plus_al = false;
//				}
//
//				if (wait_kz_plus_al.done()) {
//					wait_kz_plus_al.stop();
//					can.outID.state.kz_on_plus_al = true;
//				}
////////////////////////
//				if (adc.case_minus_al and not wait_kz_minus_al.isCount()) {
//					wait_kz_plus_al.start(3'000);
//				} else if (not adc.case_minus_al and wait_kz_plus_al.isGreater(200)) {
//					wait_kz_minus_al.stop();
//					can.outID.state.kz_on_minus_al = false;
//				}
//
//				if (wait_kz_minus_al.done()) {
//					wait_kz_minus_al.stop();
//					can.outID.state.kz_on_minus_al = true;
//				}
///////////////////////

/////////////////////////
//				if(adc.case_plus and not wait_kz_plus.isCount()) {
//					wait_kz_plus.start(2'000);
//				} else if (not adc.case_plus and wait_kz_plus.isGreater(200)) {
//					wait_kz_plus.stop();
//					can.outID.state.kz_on_plus = false;
//				}
//
//				if(wait_kz_plus.done()) {
//					wait_kz_plus.stop();
//					can.outID.state.kz_on_plus = true;
//				}
////////////////////////
//				if (adc.case_minus and not wait_kz_minus.isCount()) {
//					wait_kz_minus.start(2'000);
//				} else if (not adc.case_minus and wait_kz_minus.isGreater(200)) {
//					wait_kz_minus.stop();
//					can.outID.state.kz_on_minus = false;
//				}
//
//				if (wait_kz_minus.done()) {
//					wait_kz_minus.stop();
//					can.outID.state.kz_on_minus = true;
//				}

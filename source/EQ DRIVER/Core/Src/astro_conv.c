//
// Created by paulo on 22/09/2022.
//

#include "astro_conv.h"

/**
 * @brief Módulo de abstração do motor de passo.
 */
typedef struct {
	stepper_t stp;
	const motor_axis_t axis;
} stepper_axis_t;

/**
 * @brief Estrutura da montagem equatorial.
 */
static struct mount_data {

    int16_t fine_adjusts;
    GNSS_data_t GNSS_data;

    struct {
    	astro_pos_t orientation;
    	ptime_t LST_time;
    } time_reference;

    struct {
    	astro_target_t current_target;
    	reachability_t reachability;
    } target_info;

    struct {
        stepper_axis_t RA;
    	stepper_axis_t DEC;
    } axises;

} self = {
		.fine_adjusts = 0,
		.GNSS_data = {0},
        .time_reference = {0},
		.target_info = {
				.reachability = UNKNOWN_REACH,
		},
        .axises = {
        		.RA = {
        				.axis = Right_Ascension,
						.stp = {
							.info = {
									.position = 0,
									.target_position = 0,
									.on_status = false,
									.is_configured = false,
							},
							.timer_config = {
									.TIM = TIM3,
									.htim = &htim3,
									.TIM_CHANNEL = TIM_CHANNEL_1
							},
							.step_pin = {
									.GPIO = M1_STEP_GPIO_Port,
									.port = M1_STEP_Pin,
							},
							.dir_pin = {
									.GPIO = M1_DIR_GPIO_Port,
									.port = M1_DIR_Pin,
							},
							.enable_pin = {
									.GPIO = M1_ENABLE_GPIO_Port,
									.port = M1_ENABLE_Pin,
							},
        				},
        		},
                .DEC = {
                        .axis = Declination,
                        .stp = {
                        		.info = {
                        				.position = 0,
                        				.target_position = 0,
										.on_status = false,
										.is_configured = false,
                        		},
                        		.timer_config = {
                        					.TIM = TIM2,
											.htim = &htim2,
											.TIM_CHANNEL = TIM_CHANNEL_4,
                        		},
								.step_pin = {
										.GPIO = M2_STEP_GPIO_Port,
										.port = M2_STEP_Pin,
                        		},
								.dir_pin = {
								         .GPIO = M2_DIR_GPIO_Port,
								         .port = M2_DIR_Pin,
								 },
								.enable_pin = {
										.GPIO = M2_ENABLE_GPIO_Port,
										.port = M2_ENABLE_Pin,
								},
                        },
                },
        },
};


static float float_modulus(float num, unsigned int mod) {
	while (num > mod) {
		num -= mod;
	}

	if (num < 0) {
		num += mod; /**< Ensure circular modulus */
	}

	return num;
}

static angle_t get_angle_type(angle_t* angle_var, double decimal_degrees) {

	decimal_degrees = float_modulus(decimal_degrees, 360);

	angle_var->degrees = (uint16_t) decimal_degrees;
	angle_var->minutes = (uint8_t) ((decimal_degrees - angle_var->degrees) * 60);
	angle_var->seconds = (uint8_t) (((decimal_degrees - angle_var->degrees) - (angle_var->minutes/60.0)) * 3600);
}

static void get_time_type(ptime_t* time_var, double decimal_hours) {

	decimal_hours = float_modulus(decimal_hours, 24);

	time_var->hours = (uint8_t) decimal_hours;
	time_var->minutes = (uint8_t) ((decimal_hours - time_var->hours) * 60);
	time_var->seconds = (uint8_t) (((decimal_hours - time_var->hours) - (time_var->minutes/60.0)) * 3600);
}

void astro_update_LMST(void) {

    uint8_t Month;
    uint8_t Minutes;
    uint8_t Seconds;
    uint16_t Year;

    double current_longitude;
    double A;
    double B;
    double C;
    double E;
    double F;
    double T;
    double Hours;
    double Day;
    double current_JDN;
    double theta0;

    Hours = (self.GNSS_data.nmea_utc / 10000);
    Minutes = ((self.GNSS_data.nmea_utc / 100) - (Hours * 100));
    Seconds = (self.GNSS_data.nmea_utc - (Minutes * 100) - (Hours * 10000));

    Day = (self.GNSS_data.nmea_date / 10000);
    Month = ((self.GNSS_data.nmea_date / 100) - (Day * 100));
    Year = (self.GNSS_data.nmea_date - (Month * 100) - (Day * 10000)) + 2000;

    /** longitude filtering in degrees */
    current_longitude = (uint8_t)(self.GNSS_data.nmea_longitude / 100);
    current_longitude += (self.GNSS_data.nmea_longitude - current_longitude * 100) / 60;
    current_longitude *= (self.GNSS_data.longitude_side == 'W' ? -1.0 : 1.0); /**< signed longitude value */

    /** julian calendar correction */
    if (Month < 3) {
        Month += 12;
        Year -= 1;
    }

    /**
     * Julian Day Number calculation (no decimals)
     * A = Y/100
     * B = A/4
     * C = 2-A+B
     * E = 365.25x(Y+4716)
     * F = 30.6001x(M+1)
     * JD= C+D+E+F-1524.5
     */
    A = (int16_t)(Year / 100);
    B = (int16_t)(A / 4);
    C = (int16_t)(2 - A + B);
    E = (int) (365.25 * (Year + 4716));
    F = (int16_t)(30.6001 * (Month + 1));

    //exact amount of day
    Day += (Hours / EARTH_ROTATION_HOURS) + (Minutes / EARTH_ROTATION_MINS) + (Seconds / EARTH_ROTATION_SECS);

    current_JDN = C + Day + E + F - 1524.5;

    T = ((current_JDN - 2451545.0) / 36525);
    theta0 = 280.46061837 + 360.98564736629 * (current_JDN - 2451545.0) + (0.000387933 * T * T) - (T * T * T / 38710000.0);

    theta0 += current_longitude; /**< Shift GMST to LMST */

    /** Local Mean Sideral Time */
    get_time_type(&self.time_reference.LST_time, (theta0/15.0));
}

void astro_update_fine_adjusts(void) {
	static uint8_t raw_reading = 0;
	raw_reading  = fine_adjusts_prescaler_value();

	self.fine_adjusts = raw_reading - 50; //creates scale from -100 to 100
}

static reachability_t check_reachability(ptime_t target_RA) {
	bool is_set;
	bool is_risen;
	uint8_t rise_begin;
	uint8_t set_end;

	set_end = (self.time_reference.LST_time.hours + 6) % EARTH_ROTATION_HOURS;
	rise_begin = (self.time_reference.LST_time.hours - 6) % EARTH_ROTATION_HOURS;
	if (rise_begin < 0) {
		rise_begin += EARTH_ROTATION_HOURS;
	}

	is_risen = (target_RA.hours > rise_begin);
	is_set = (target_RA.hours >= set_end);


	if (!is_risen) {
		return UNREACHABLE_UNRISEN;
	} else if (is_set) {
		return UNREACHABLE_SET;
	} else {
		return REACHABLE;
	}
}

reachability_t astro_set_target(astro_target_t target) {

	self.target_info.current_target = target;

	if (self.GNSS_data.is_valid == VALID_DATA) {
		self.target_info.reachability = check_reachability(target.position.right_ascension);
		return self.target_info.reachability;
	}

	return UNKNOWN_REACH;
}

GNSS_data_t* astro_get_gnss_pointer(void) {
	return &self.GNSS_data;
}

void astro_stepper_position_step(motor_axis_t axis) {
	uint16_t new_pos;


	switch (axis) {
		case Right_Ascension:
			if (!self.axises.RA.stp.info.is_configured || !self.axises.RA.stp.info.on_status) return;
			new_pos = (self.axises.RA.stp.info.position + self.axises.RA.stp.info.direction) % STEPPER_MAX_STEPS;
			if (new_pos < 0) {
				new_pos += STEPPER_MAX_STEPS;
			}

			self.axises.RA.stp.info.position = new_pos;
			break;
		case Declination:
			if (!self.axises.DEC.stp.info.is_configured || !self.axises.DEC.stp.info.on_status) return;
			new_pos = (self.axises.DEC.stp.info.position + self.axises.DEC.stp.info.direction) % STEPPER_MAX_STEPS;
			if (new_pos < 0) {
				new_pos += STEPPER_MAX_STEPS;
			}

			self.axises.DEC.stp.info.position = new_pos;
			break;
		default:
			break;
	}
}

void astro_init(void) {
    stepper_init(&self.axises.RA.stp);
    stepper_init(&self.axises.DEC.stp);

    self.axises.RA.stp.timer_config.pwm_period = TRACKING_SPEED_PULSE_PERIOD_duS;
}

void astro_start_tracking(void) {

    HAL_TIM_PWM_Start_IT(self.axises.RA.stp.timer_config.htim, self.axises.RA.stp.timer_config.TIM_CHANNEL);
    self.axises.RA.stp.timer_config.TIM->ARR = self.axises.RA.stp.timer_config.pwm_period * 2; /**< 10us per unit */
    self.axises.RA.stp.timer_config.TIM->CCR1 = self.axises.RA.stp.timer_config.pwm_period; /**< 5us per unit = 50% duty */
}

void astro_axis_add_fine_adjusts(motor_axis_t axis) {

	astro_update_fine_adjusts();

	switch (axis) {
	case Right_Ascension:
		self.axises.RA.stp.timer_config.pwm_period += self.fine_adjusts;
		break;
	case Declination:
		self.axises.DEC.stp.timer_config.pwm_period += self.fine_adjusts;
		break;
	default:
		break;
	}
}

void astro_full_stop(void) {
	HAL_TIM_PWM_Stop_IT(self.axises.DEC.stp.timer_config.htim, self.axises.DEC.stp.timer_config.TIM_CHANNEL);
    HAL_TIM_PWM_Stop_IT(self.axises.RA.stp.timer_config.htim, self.axises.RA.stp.timer_config.TIM_CHANNEL);
}

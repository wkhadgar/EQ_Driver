//
// Created by paulo on 22/09/2022.
//

#ifndef EQMOUNT_CUSTOM_CONTROLLER_ASTRO_CONV_H
#define EQMOUNT_CUSTOM_CONTROLLER_ASTRO_CONV_H

#include "steppers.h"
#include "adc.h"
#include "PA6H.h"

#define ASTRO_TARGET_NAME_MAX_CHARACTERS 27
#define EARTH_ROTATION_HOURS             24
#define EARTH_ROTATION_MINS              1440.0
#define EARTH_ROTATION_SECS              86400.0
#define SECONDS_PER_HOUR                 3600
#define MINUTES_PER_HOUR                 60

#define WORM_GEAR_RATIO 96
#define STEPPER_FULL_RANGE (STEPPER_MAX_STEPS * WORM_GEAR_RATIO)

#define TRACKING_SPEED_PULSE_PERIOD_duS 28125 /**< in deca-microseconds, us = 281250 */
#define MAX_SPEED_PULSE_PERIOD_duS 187 /**< in deca-microseconds, us = 1870 */

typedef enum {
	UNREACHABLE_SET = 0,
	UNREACHABLE_UNRISEN,
	REACHABLE,
	UNKNOWN_REACH = -1,
} reachability_t;

typedef enum {
	TRACKING = 0,
	GOING_TO = 1,
} movement_t;

/**
 * @brief Time position struct.
 */
typedef struct {
	double decimal_hours;
	uint8_t hours;
	uint8_t minutes;
	uint8_t seconds;
} time__t;

/**
 * @brief Angle storing struct.
 */
typedef struct {
	double decimal_degrees;
	uint16_t degrees;
	uint8_t arc_minutes;
	uint8_t arc_seconds;
} angle_t;

/**
 * @brief Standardized position struct.
 */
typedef struct astro_pos {
	time__t right_ascension;
	angle_t declination;
} astro_pos_t;

/**
 * @brief Target struct.
 */
typedef struct astro_target {
	astro_pos_t position;
	char* name; /** characters limit + '\0' */
} astro_target_t;

/**
 * @brief Initializes the astro modules.
 */
void astro_init(void);

/**
 * @brief Calculates LST by estimating GMST and shifting it to the current longitude.
 *
 * @note Pseudo code for GMST calculation:
 *  midnight = floor(2458484.833333) + 0.5            // J0
 *	days_since_midnight = 2458484.833333 - 2458484.5  //
 *	hours_since_midnight = days_since_midnight * 24   // H
 *	days_since_epoch = 2458484.833333 - 2451545.0     // D
 *	centuries_since_epoch = days_since_epoch / 36525  // T
 *	whole_days_since_epoch = 2458484.5 - 2451545.0    // D0
 *
 *	GMST = 6.697374558 + 0.06570982441908 * whole_days_since_epoch + 1.00273790935 * hours_since_midnight
 *		 + 0.000026 * centuries_since_epoch^2
 *
 *	GMST_hours = 470 % 24 // = 14
 *	GMST_minutes =  floor(0.712605328 * 60) // = 42(.7563197)
 *
 */
void astro_update_LMST(void);

bool astro_is_at_target(void);

void astro_update_raw_fine_adjusts(void);

void astro_update_target(astro_target_t target);

GNSS_data_t* astro_get_gnss_pointer(void);

void astro_start_tracking(movement_t movement);

void astro_goto_target(void);

void astro_go_home(void);

void astro_full_stop(void);

/** Must be called from timer isr, just after ARR reload. */
void astro_stepper_position_step(motor_axis_t axis, movement_t movement);

#endif //EQMOUNT_CUSTOM_CONTROLLER_ASTRO_CONV_H

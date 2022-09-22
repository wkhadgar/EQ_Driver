//
// Created by paulo on 22/09/2022.
//

#ifndef EQMOUNT_CUSTOM_CONTROLLER_ASTRO_CONV_H
#define EQMOUNT_CUSTOM_CONTROLLER_ASTRO_CONV_H

#include "steppers.h"

#define EARTH_ROTATION_MIN 1440
#define EARTH_ROTATION_SECS 86400
#define RA_WORM_RATIO 96
#define LOCAL_LATITUDE_SECS -34396 /**< 09°33'16" S */
#define LOCAL_LONGITUDE_SECS -128789 /**< -035°46'29" E */

void astro_go_to(uint16_t target_pos, stepper_t *right_ascension_stp, stepper_t *declination_stp);

void astro_start_tracking(stepper_t *right_ascension_stp, stepper_t *declination_stp);

void astro_stop_tracking(stepper_t *right_ascension_stp, stepper_t *declination_stp);

#endif //EQMOUNT_CUSTOM_CONTROLLER_ASTRO_CONV_H
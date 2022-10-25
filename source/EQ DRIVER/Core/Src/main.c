/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "stdint.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "variables.h"
#include "steppers.h"
#include "astro_conv.h"
#include "PA6H.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct rotary_data {
    int8_t inc;
    bool was_pressed;
} rotary_data_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ROT_DEBOUNCE_DELAY_MS 10 /** < Delay to accept another rotary move */
#define PUSH_DEBOUNCE_DELAY_MS 300 /** < Delay to accept another button press */
#define V_BAT_MIN 32 /** < 3.2v * 10 */
#define V_BAT_MAX 42 /** < 4.2v * 10 */
#define SCREEN_ROWS 5
#define CHECK_BATTERY_STATUS 0 /** change to 0 if driver is connected without a battery */
#define ENABLE_FAKE_LOAD 0 /** change to 0 if the loading screen must not be shown */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

#define TICKS_NOW HAL_GetTick() /** < Current sys tick value */
#define POSITIVE_MODULUS(value, mod) (((value) < 0) ? ((mod) -1) : ((value) % (mod)))
#define BOOLIFY(value) ((value) >= 1) ? 1 : 0
#define SCROLL_DOWN_MENU(menu_top) (((menu_top) < (MENU_SIZE - SCREEN_ROWS)) ? (menu_top)+1 : (menu_top)) /** < Increases menu head in-bound */
#define SCROLL_UP_MENU(menu_top) (((menu_top) > 0) ? (menu_top)-1 : (menu_top)) /** < Decreases menu head in-bound */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/** Equatorial Mount structure initialization */
static mount_data_t EQM = {
        .orientation = {
                .declination = 0,
                .right_ascension = 12 * 60,
        },
        .axis_stepper = {
                .DEC = {
                        .axis = Declination,
                        .timer_config = {
                                .TIMx = TIM3,
                                .htimx = &htim3,
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
                        }
                },
                .RA = {
                        .axis = Right_Ascension,
                        .timer_config = {
                                .TIMx = TIM2,
                                .htimx = &htim2,
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
                        }
                }
        }
};

/**
 * @brief menu options enum
 */
enum menu_options {
    DEC_ = 0,
    RA,
    hemisphere,
    automatic_mode,
    manual_mode,
    brilho_tela,
    tempo_tela,
    save_configs,
    MENU_SIZE, //must be the last value
};

/**
 * @brief Strings to the menu
 */
const char* menu_str[MENU_SIZE] = {
        [DEC_]           = "DEC",
        [RA]             = "R.A",
        [hemisphere]     = "Hemisferio",
        [automatic_mode] = "Modo automatico",
        [manual_mode]    = "Modo manual",
        [brilho_tela]    = "Contraste",
        [tempo_tela]     = "Luz da tela",
        [save_configs]   = "Salvar configs",
};

uint16_t menu_op_value[MENU_SIZE] = {0};

uint32_t last_move_ticks = 0; // to track time passed in ms with HAL_GetTick()

#if CHECK_BATTERY_STATUS
uint32_t bat_ticks_update = 0;
#endif /** CHECK_BATTERY_STATUS */


uint8_t menu_head = 0;
uint8_t menu_selection = 0;
uint8_t frame = 0;

#if CHECK_BATTERY_STATUS
uint8_t battery_charge;
#endif /** CHECK_BATTERY_STATUS */

int16_t signed_value_preview = 0;
uint16_t value_preview = 0;
uint16_t lock_value = 0;

uint8_t timer_pre_scaler = 1;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

#define CURRENT_SELECTION() (menu_head+menu_selection)

/**
 * @brief Deals with the rotary encoder changes
 *
 * @param rotary_data [in] Ponteiro da struct com as informações dos eventos do rotary encoder
 */
void handle_rotary_events(rotary_data_t* rotary_data);

/**
 * @brief Manage the changes in the menu, updating position variables and value previews.
 *
 * @param current_menu_top [out] Value to be updated of the menu head
 * @param arrow_row  [out] Value of the selection arrow on menu screen
 * @param op_value [out] Value to be updated when not in menu
 * @param increase boolean telling whether to increase or decrease the current screen parameters
 */
void handle_menu_changes(uint8_t* current_menu_top, uint8_t* arrow_row, uint16_t* op_value, int8_t increase);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

//void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
//    if (GPIO_Pin == ROTARY_TRIG_Pin) {
//        set_flag(rotary_triggered);
//        if (ROTARY_CLKW_GPIO_Port->IDR & ROTARY_CLKW_Pin) {
//            set_flag(ccw);
//        }
//    } else if (GPIO_Pin == SELECT_Pin) {
//        set_flag(selected);
//    }
//}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart) {
    if (huart == &huart1) {
        if (GNSS_UART_CallBack(&EQM.GNSS_data)) {
            update_LST(&EQM);
        }
    }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void) {
    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_TIM2_Init();
    MX_ADC1_Init();
    MX_TIM3_Init();
    MX_USART1_UART_Init();
    MX_SPI1_Init();
    MX_TIM4_Init();
    /* USER CODE BEGIN 2 */

    //digitalWrite(, 0); /** emulated gnd */
    //digitalWrite(OUT_VDD_SIG, 1); /** reference voltage */

    HAL_Delay(500);
    //SH1106_cleanInit();
    GNSS_init();

//    /**start logo display */
//    SH1106_drawBitmapFullscreen(eqmount_logo);
//    SH1106_flush();
//    SH1106_clear();
//    HAL_Delay(2500);

#if ENABLE_FAKE_LOAD
    //    /** transition to menu fake load */
    //    SH1106_printStr(16, 15, "Carregando Menu", fnt5x7);
    //    for (uint8_t s = 0; s <= 100; s += 2) {
    //        SH1106_drawRoundRectFill(s, 7, 32, 110, 8);
    //        SH1106_flush();
    //        HAL_Delay(1);
    //    }
#endif /** ENABLE_FAKE_LOAD */

//    SH1106_clear();
    set_flag(on_menu);

    last_move_ticks = TICKS_NOW; /** < start time reference */

    stepper_init(&EQM.axis_stepper.DEC);
    stepper_init(&EQM.axis_stepper.RA);
    HAL_TIM_Base_Start_IT(&htim2);
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1) {
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */

//        SH1106_clear(); /** < clears buffer, to construct new one and flush it later */

        /** horse running easter egg */
//        {
//            if (get_flag(toggle_horse)) {
//                if (frame > 14) frame = 0;
//                SH1106_drawBitmapFullscreen(horse_running[frame++]);
//                SH1106_flush();
//            }
//        }

        /** updates on menu related values*/
        {
//            rotary_data_t rotary_events;
//            handle_rotary_events(&rotary_events);
//
//            if (rotary_events.was_pressed) {
//                if ((lock_value != value_preview) && get_flag(on_menu)) { //if value changed
//                    menu_op_value[CURRENT_SELECTION()] = value_preview; //saves value to menu
//                    lock_value = value_preview;
//
//                    switch (CURRENT_SELECTION()) { /** the given value was updated, then: ... */
//                        case DEC_:
//                            EQM.axis_stepper.DEC.target_position = ((menu_op_value[DEC_] / 60) %
//                                                                    STEPPER_MAX_STEPS); //TODO calcular a proporção correta
//                            break;
//
//                        case hemisphere:
//                            if (menu_op_value[hemisphere]) {
//                                stepper_set_direction(&EQM.axis_stepper.RA, clockwise);
//                            } else {
//                                stepper_set_direction(&EQM.axis_stepper.RA, counter_clockwise);
//                            }
//                            break;
//                    }
//
//                } else if (!get_flag(on_menu)) {
//                    value_preview = menu_op_value[CURRENT_SELECTION()]; //gets value from menu
//                }
//
//            } else if (rotary_events.inc) {
//                handle_menu_changes(&menu_head, &menu_selection, &value_preview, rotary_events.inc);
//            }
//
        }

        /** drawing the menu */
//        {
//            uint16_t current_x;
//            const uint8_t space_pixel_width = 2;
//            if (get_flag(on_menu)) {
//                set_flag(update_display);
//
//
//                static uint8_t pool_delay = 0;
//                if (get_flag(selected) || pool_delay) {
//                    reset_flag(selected);
//                    pool_delay++;
//                    SH1106_drawBitmap(space_pixel_width, 5 + (12 * menu_selection), 5, 8, arrow);
//                    if (pool_delay >= 10) {
//                        pool_delay = 0;
//                        set_flag(selected);
//                    }
//                } else {
//                    SH1106_drawBitmap(0, 5 + (12 * menu_selection), 5, 8, arrow);
//                }
//
//                for (uint8_t current_drawing_row = 0; current_drawing_row < SCREEN_ROWS; current_drawing_row++) {
//                    current_x = 8;
//                    uint8_t current_y = space_pixel_width + 3 + (12 * current_drawing_row);
//
//                    current_x +=
//                            SH1106_printStr(current_x, current_y, menu_str[menu_head + current_drawing_row], fnt5x7) +
//                            space_pixel_width;
//                    current_x +=
//                            SH1106_printChar(current_x, current_y, ':', fnt5x7) + space_pixel_width;
//                    switch (current_drawing_row + menu_head) {
//
//                        case DEC_:
//                            current_x += SH1106_printInt(current_x, current_y, menu_op_value[DEC_] / 60, fnt5x7) + 1;
//                            current_x += SH1106_printChar(current_x, current_y - 2, 'o', fnt5x7) +
//                                         1; //TODO alterar na lib das fontes o °
//                            current_x += SH1106_printInt(current_x, current_y, menu_op_value[DEC_] % 60, fnt5x7);
//                            SH1106_printChar(current_x, current_y, '\'', fnt5x7);
//                            break;
//                        case RA:
//                            current_x += SH1106_printInt(current_x, current_y, menu_op_value[RA] / 60, fnt5x7) + 1;
//                            current_x += SH1106_printChar(current_x, current_y, 'h', fnt5x7) +
//                                         1; //TODO alterar na lib das fontes o °
//                            current_x += SH1106_printInt(current_x, current_y, menu_op_value[RA] % 60, fnt5x7);
//                            SH1106_printChar(current_x, current_y, 'm', fnt5x7);
//                            break;
//                        case hemisphere:
//                            SH1106_printStr(current_x, current_y, menu_op_value[hemisphere] ? "Norte" : "Sul", fnt5x7);
//                            break;
//                        case automatic_mode:
//                            SH1106_printStr(current_x, current_y, menu_op_value[automatic_mode] ? "ON" : "OFF", fnt5x7);
//                            break;
//                        case manual_mode:
//                            SH1106_printStr(current_x, current_y, menu_op_value[manual_mode] ? "ON" : "OFF", fnt5x7);
//                            break;
//                        case brilho_tela:
//                            current_x += SH1106_printInt(current_x, current_y, menu_op_value[brilho_tela], fnt5x7);
//                            SH1106_printChar(current_x, current_y, '%', fnt5x7);
//                            break;
//                        case tempo_tela:
//                            current_x += SH1106_printInt(current_x, current_y, menu_op_value[tempo_tela], fnt5x7);
//                            SH1106_printChar(current_x, current_y, 's', fnt5x7);
//                            break;
//                        case save_configs:
//                            SH1106_printStr(current_x - 8, current_y, "  ", fnt5x7);
//                            break;
//                        default:
//                            SH1106_printInt(current_x + space_pixel_width, current_y,
//                                            menu_op_value[menu_head + current_drawing_row],
//                                            fnt5x7);
//                            break;
//                    }
//                }
//
//            } else { /** drawing the submenu */
//                set_flag(update_display);
//
//                current_x = space_pixel_width;
//
//                current_x += SH1106_printStr(current_x, 2, menu_str[CURRENT_SELECTION], fnt5x7);
//                SH1106_printStr(current_x, 2, ":", fnt5x7);
//
//                current_x = (SCR_W / 2) - 14;
//                switch (CURRENT_SELECTION) {
//
//                    case DEC_:
//                        current_x -= 5;
//                        value_preview = POSITIVE_MODULUS(value_preview, 21600);
//                        current_x += SH1106_printInt(current_x, SCR_H / 2, value_preview / 60, fnt7x10) + 1;
//                        current_x += SH1106_printChar(current_x, (SCR_H / 2) - 5, 'o', fnt7x10) +
//                                     1; //TODO alterar na lib das fontes o °
//                        current_x += SH1106_printInt(current_x, SCR_H / 2, value_preview % 60, fnt7x10);
//                        SH1106_printStr(current_x, SCR_H / 2, "'", fnt7x10);
//                        break;
//                    case RA:
//                        current_x -= 5;
//                        value_preview = POSITIVE_MODULUS(value_preview, 1440);
//                        current_x += SH1106_printInt(current_x, SCR_H / 2, value_preview / 60, fnt7x10) + 1;
//                        current_x += SH1106_printChar(current_x, SCR_H / 2, 'h', fnt7x10) +
//                                     1; //TODO alterar na lib das fontes o °
//                        current_x += SH1106_printInt(current_x, SCR_H / 2, value_preview % 60, fnt7x10);
//                        SH1106_printStr(current_x, SCR_H / 2, "m", fnt7x10);
//                        break;
//                    case hemisphere:
//                        current_x -= 5;
//                        value_preview = BOOLIFY(value_preview);
//                        SH1106_printStr(current_x, SCR_H / 2, value_preview ? "Norte" : "Sul", fnt7x10);
//                        break;
//                    case automatic_mode:
//                        value_preview = BOOLIFY(value_preview);
//                        SH1106_printStr(current_x, SCR_H / 2, value_preview ? "ON" : "OFF", fnt7x10);
//                        break;
//                    case manual_mode:
//                        value_preview = BOOLIFY(value_preview);
//                        SH1106_printStr(current_x, SCR_H / 2, value_preview ? "ON" : "OFF", fnt7x10);
//                        break;
//                    case brilho_tela:
//                        value_preview = POSITIVE_MODULUS(value_preview, 100);
//                        if (value_preview > 100) value_preview = 100;
//                        current_x += SH1106_printInt(current_x, SCR_H / 2, value_preview, fnt7x10);
//                        SH1106_printChar(current_x, SCR_H / 2, '%', fnt7x10);
//                        break;
//                    case tempo_tela:
//                        value_preview = POSITIVE_MODULUS(value_preview, 255);
//                        current_x += SH1106_printInt(current_x, SCR_H / 2, value_preview, fnt7x10);
//                        SH1106_printChar(current_x, SCR_H / 2, 's', fnt7x10);
//                        break;
//                    case save_configs:
//                        set_flag(on_menu);
//                        break;
//                    default:
//                        SH1106_printInt(current_x, SCR_H / 2, value_preview, fnt7x10);
//                }
//
//            }
//        }


        /** < checks if changes to the buffer happened, and if so, flush them */
        if (get_flag(update_display)) {
            reset_flag(update_display);
//            SH1106_flush();
        }
    }
    /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    /** Initializes the RCC Oscillators according to the specified parameters
    * in the RCC_OscInitTypeDef structure.
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
        Error_Handler();
    }
}

/* USER CODE BEGIN 4 */
void handle_rotary_events(rotary_data_t* rotary_data) {

    if (get_flag(rotary_triggered)) { // rotary encoder triggered
        reset_flag(rotary_triggered);
        set_flag(wake);

        if ((TICKS_NOW - last_move_ticks) >= ROT_DEBOUNCE_DELAY_MS) {

            last_move_ticks = TICKS_NOW;

            if (get_flag(ccw)) { //counter-clockwise rotation

                reset_flag(ccw);
                rotary_data->inc = -1;
                return;
            } else { // clockwise rotation
                rotary_data->inc = 1;
                return;
            }
        }
        set_flag(update_display);

    } else if (get_flag(selected)) { // rotary encoder pressed
        reset_flag(selected);
        set_flag(wake);

        if ((TICKS_NOW - last_move_ticks) >= PUSH_DEBOUNCE_DELAY_MS) {

            last_move_ticks = TICKS_NOW;
            set_flag(wake);

            rotary_data->was_pressed = true;

            if (get_flag(on_menu)) reset_flag(on_menu);
            else set_flag(on_menu);

        }
        set_flag(update_display);
        return;
    }

    /** resets all values if nothing happened */
    rotary_data->inc = 0;
    rotary_data->was_pressed = false;
}

void handle_menu_changes(uint8_t* current_menu_top, uint8_t* arrow_row, uint16_t* op_value, int8_t increase) {

    if (increase && (current_menu_top != NULL) && (arrow_row != NULL) && (op_value != NULL)) {
        if (increase == 1) {
            if (get_flag(on_menu)) {
                if (*arrow_row < SCREEN_ROWS
                                 - 1) {
                    (*arrow_row)++;
                } else {
                    *current_menu_top = SCROLL_DOWN_MENU(*current_menu_top);
                }
            } else {
                (*op_value) += 5; //forward increase
            }
        } else {
            if (get_flag(on_menu)) {
                if (*arrow_row > 0) {
                    (*arrow_row)--;
                } else {
                    *current_menu_top = SCROLL_UP_MENU(*current_menu_top);
                }
            } else {
                (*op_value) = (*op_value) > 0 ? ((*op_value) - 1) : (*op_value); //unsigned guarded decrease
            }
        }
    }
}

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim) {
    /* USER CODE BEGIN Callback 0 */
    static uint8_t scaler_counter = 0;
    static bool do_step_increment[3] = {false};
    static uint16_t current_period = BASE_PERIOD * 8;
    /* USER CODE END Callback 0 */
    if (htim->Instance == TIM1) {
        HAL_IncTick();
    }
        /* USER CODE BEGIN Callback 1 */
#ifdef STEP_PIN_AS_GPIO
        else if (htim == &htim2) {
            scaler_counter++;
            if (timer_pre_scaler == scaler_counter) {
                if (RA_STEPPER.on_status && menu_op_value[manual_mode]) {
                    half_step(&RA_STEPPER);
                }
                scaler_counter = 0;
            }
        }
#else /** STEP_PIN_AS_GPIO */
    else if (htim == &htim3) { //M1
        do_step_increment[Right_Ascension] = !do_step_increment[Right_Ascension];
        if (do_step_increment[Right_Ascension]) {
            EQM.axis_stepper.RA.position =
                    (((EQM.axis_stepper.RA.position + EQM.axis_stepper.RA.direction) % STEPPER_MAX_STEPS) +
                     STEPPER_MAX_STEPS) % STEPPER_MAX_STEPS;
        }

        if (!menu_op_value[automatic_mode]) {
            current_period = stepper_to_target_smoothen_period(&EQM.axis_stepper.RA, 1200);
            EQM.axis_stepper.RA.timer_config.TIMx->ARR = current_period;
            EQM.axis_stepper.RA.timer_config.TIMx->CCR1 = current_period / 2;
        }

    } else if (htim == &htim2) { //M2
        do_step_increment[Declination] = !do_step_increment[Declination];
        if (do_step_increment[Declination]) {
            EQM.axis_stepper.DEC.position =
                    (((EQM.axis_stepper.DEC.position + EQM.axis_stepper.DEC.direction) % STEPPER_MAX_STEPS) +
                     STEPPER_MAX_STEPS) % STEPPER_MAX_STEPS;

        }
    }
#endif /** STEP_PIN_AS_GPIO */
    /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void) {
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1);
    /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line
     number, ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
     line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

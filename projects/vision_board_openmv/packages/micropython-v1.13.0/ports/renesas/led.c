/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2016 Damien P. George
 * Copyright (c) 2021,2022 Renesas Electronics Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>

#include "py/runtime.h"
#include "py/mphal.h"
#include "led.h"

#if defined(MICROPY_HW_LED1)

/// \moduleref pyb
/// \class LED - LED object
///
/// The LED object controls an individual LED (Light Emitting Diode).
static const machine_pin_obj_t g_bsp_prv_leds[] =
{
    {.base = {&pyb_led_type}, .name = "LED_BLUE", .pin = MICROPY_HW_LED1, .pin_isr_cb = NULL },
    {.base = {&pyb_led_type}, .name = "LED_GREEN", .pin = MICROPY_HW_LED2, .pin_isr_cb = NULL },
    {.base = {&pyb_led_type}, .name = "LED_RED", .pin = MICROPY_HW_LED3, .pin_isr_cb = NULL },
};

const ra_led_obj_t machine_led_obj[] = {
    {{&pyb_led_type}, 1, &g_bsp_prv_leds[0]},
    #if defined(MICROPY_HW_LED2)
    {{&pyb_led_type}, 2, &g_bsp_prv_leds[1]},
    #if defined(MICROPY_HW_LED3)
    {{&pyb_led_type}, 3, &g_bsp_prv_leds[2]},
    #if defined(MICROPY_HW_LED4)
    {{&pyb_led_type}, 4, &g_bsp_prv_leds[3]},
    #endif
    #endif
    #endif
};

void led_init(void) {
    /* Turn off LEDs and initialize */
    for (int led = 0; led < NUM_LEDS; led++) {
        const machine_pin_obj_t *led_pin = machine_led_obj[led].led_pin;
        mp_hal_pin_output((machine_pin_obj_t *)&led_pin);
    }
}

void led_state(ra_led_t led, int state) {
    if (led < 1 || led > NUM_LEDS) {
        return;
    }

    const machine_pin_obj_t *led_pin = machine_led_obj[led - 1].led_pin;
    if (state == 0) {
        // turn LED off
        rt_pin_write(led_pin->pin, 1);
    } else {
        // turn LED on
        rt_pin_write(led_pin->pin, 0);
    }
}

void led_toggle(ra_led_t led) {
    if (led < 1 || led > NUM_LEDS) {
        return;
    }
    const machine_pin_obj_t *led_pin = machine_led_obj[led - 1].led_pin;
    rt_pin_write(led_pin->pin, ~rt_pin_read(led_pin->pin));
}

void led_debug(int n, int delay) {
    led_state(1, n & 1);
    led_state(2, n & 2);
    led_state(3, n & 4);
    led_state(4, n & 8);
    mp_hal_delay_ms(delay);
}

/******************************************************************************/
/* MicroPython bindings                                                       */

void led_obj_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    ra_led_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "LED(%u)", self->led_id);
}

/// \classmethod \constructor(id)
/// Create an LED object associated with the given LED:
///
///   - `id` is the LED number, 1-4.
STATIC mp_obj_t led_obj_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    // check arguments
    mp_arg_check_num(n_args, n_kw, 1, 1, false);

    // get led number
    mp_int_t led_id = mp_obj_get_int(args[0]);

    // check led number
    if (!(1 <= led_id && led_id <= NUM_LEDS)) {
        mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("LED(%d) doesn't exist"), led_id);
    }

    // return static led object
    return MP_OBJ_FROM_PTR(&machine_led_obj[led_id - 1]);
}

/// \method on()
/// Turn the LED on.
mp_obj_t led_obj_on(mp_obj_t self_in) {
    ra_led_obj_t *self = MP_OBJ_TO_PTR(self_in);
    led_state(self->led_id, 1);
    return mp_const_none;
}

/// \method off()
/// Turn the LED off.
mp_obj_t led_obj_off(mp_obj_t self_in) {
    ra_led_obj_t *self = MP_OBJ_TO_PTR(self_in);
    led_state(self->led_id, 0);
    return mp_const_none;
}

/// \method toggle()
/// Toggle the LED between on and off.
mp_obj_t led_obj_toggle(mp_obj_t self_in) {
    ra_led_obj_t *self = MP_OBJ_TO_PTR(self_in);
    led_toggle(self->led_id);
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(led_obj_on_obj, led_obj_on);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(led_obj_off_obj, led_obj_off);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(led_obj_toggle_obj, led_obj_toggle);

STATIC const mp_rom_map_elem_t led_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_on), MP_ROM_PTR(&led_obj_on_obj) },
    { MP_ROM_QSTR(MP_QSTR_off), MP_ROM_PTR(&led_obj_off_obj) },
    { MP_ROM_QSTR(MP_QSTR_toggle), MP_ROM_PTR(&led_obj_toggle_obj) },
};

STATIC MP_DEFINE_CONST_DICT(led_locals_dict, led_locals_dict_table);

const mp_obj_type_t pyb_led_type = {
    { &mp_type_type },
    .name = MP_QSTR_LED,
    .print = led_obj_print,
    .make_new = led_obj_make_new,
    .locals_dict = (mp_obj_dict_t *)&led_locals_dict,
};

#else
// For boards with no LEDs, we leave an empty function here so that we don't
// have to put conditionals everywhere.
void led_init(void) {
}
void led_state(ra_led_t led, int state) {
}
void led_toggle(ra_led_t led) {
}
#endif  // defined(MICROPY_HW_LED1)

/*****************************************************************************
 *   Copyright(C)2009-2022 by VSF Team                                       *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the "License");          *
 *  you may not use this file except in compliance with the License.         *
 *  You may obtain a copy of the License at                                  *
 *                                                                           *
 *     http://www.apache.org/licenses/LICENSE-2.0                            *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an "AS IS" BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *                                                                           *
 ****************************************************************************/

#define VSF_GPIO_CFG_IMP_PREFIX                 vsf_hw
#define VSF_GPIO_CFG_IMP_UPCASE_PREFIX          VSF_HW

/*============================ INCLUDES ======================================*/

#include "./gpio.h"

#if VSF_HAL_USE_GPIO == ENABLED

#include "./i_reg_gpio.h"

#include "hal/driver/AIC/AIC8800/vendor/plf/aic8800/src/driver/aic1000lite_regs/aic1000Lite_iomux.h"
#include "../io/io.h"

#define VSF_GPIO_CFG_REIMPLEMENT_API_CAPABILITY         ENABLED
#define VSF_GPIO_CFG_CHNAGE_DIR_FIRST                   ENABLED
#define VSF_GPIO_CFG_CAPABILITY_HAS_OUTPUT_AND_SET      1
#define VSF_GPIO_CFG_CAPABILITY_HAS_OUTPUT_AND_CLEAR    1

/*============================ MACROS ========================================*/

#ifndef VSF_HW_GPIO_CFG_MULTI_CLASS
#   define VSF_HW_GPIO_CFG_MULTI_CLASS      VSF_GPIO_CFG_MULTI_CLASS
#endif

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ PROTOTYPES ====================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/

#ifndef VSF_HW_GPIO_CFG_PROTECT_LEVEL
#   define VSF_HW_GPIO_CFG_PROTECT_LEVEL    interrupt
#endif

#define __vsf_gpio_protect                  vsf_protect(VSF_HW_GPIO_CFG_PROTECT_LEVEL)
#define __vsf_gpio_unprotect                vsf_unprotect(VSF_HW_GPIO_CFG_PROTECT_LEVEL)

/*============================ TYPES =========================================*/

typedef struct vsf_hw_gpio_t {
#if VSF_HW_GPIO_CFG_MULTI_CLASS == ENABLED
    vsf_gpio_t vsf_gpio;
#endif

    GPIO_REG_T *GPIO;
    uint32_t output_reg;
    uint8_t is_pmic;
    uint16_t gpio_pin_mask; // in pmic port, must set bit to 1 in MR register

    AIC_IOMUX_TypeDef *IOMUX;
} vsf_hw_gpio_t;

/*============================ IMPLEMENTATION ================================*/

static void __aic8800_gpio_init(vsf_hw_gpio_t *hw_gpio_ptr)
{
    // When the PMIC is used to access the GPIO,
    // critical area protection is already provided,
    // so there is no need to use MR registers for protection.
    // And the MR register should be set to 1 correctly when the IO is a GPIO.

    // This API should be used at initialization time
    if (hw_gpio_ptr->is_pmic && (hw_gpio_ptr->gpio_pin_mask != 0)) {
        PMIC_MEM_MASK_WRITE((unsigned int)&hw_gpio_ptr->GPIO->MR,
                            hw_gpio_ptr->gpio_pin_mask, hw_gpio_ptr->gpio_pin_mask);
    }
}

void __vsf_hw_aic8800_gpio_init(void)
{
    for (int i = 0; i < dimof(vsf_hw_gpios); i++) {
        __aic8800_gpio_init(vsf_hw_gpios[i]);
    }
}

void vsf_hw_gpio_config_pin(vsf_hw_gpio_t *hw_gpio_ptr, vsf_gpio_pin_mask_t pin_mask, vsf_io_feature_t feature)
{
    VSF_HAL_ASSERT(NULL != hw_gpio_ptr);
    VSF_HAL_ASSERT(__AIC8800_IO_IS_VAILID_PIN(pin_mask));
    VSF_HAL_ASSERT(__AIC8800_IO_IS_VAILID_FEATURE(feature));

    for (int i = 0; i < VSF_HW_IO_PIN_COUNT; i++) {
        uint32_t current_pin_mask = 1 << i;
        if (pin_mask & current_pin_mask) {
            __hw_io_reg_mask_write(hw_gpio_ptr->is_pmic, &hw_gpio_ptr->IOMUX->GPCFG[i],
                                  feature, __AIC8800_IO_FEATURE_ALL_BITS);
            if (hw_gpio_ptr->is_pmic) {
                PMIC_MEM_MASK_WRITE((unsigned int)&hw_gpio_ptr->GPIO->MR, current_pin_mask, current_pin_mask);
            }
        }
    }
}

void vsf_hw_gpio_set_direction(vsf_hw_gpio_t *hw_gpio_ptr, vsf_gpio_pin_mask_t pin_mask, vsf_gpio_pin_mask_t direction_mask)
{
    VSF_HAL_ASSERT(NULL != hw_gpio_ptr);
    VSF_HAL_ASSERT(__AIC8800_IO_IS_VAILID_PIN(pin_mask));
    // GPIOB0 can only be used in the input direction
    VSF_HAL_ASSERT(   (hw_gpio_ptr->is_pmic != 1)
                   || ((pin_mask & VSF_PIN0_MASK) != VSF_PIN0_MASK)
                   || ((direction_mask & VSF_PIN0_MASK) != VSF_PIN0_MASK));

    __hw_io_reg_mask_write(hw_gpio_ptr->is_pmic, &hw_gpio_ptr->GPIO->DR, direction_mask, pin_mask);
}

vsf_gpio_pin_mask_t vsf_hw_gpio_get_direction(vsf_hw_gpio_t *hw_gpio_ptr, vsf_gpio_pin_mask_t pin_mask)
{
    VSF_HAL_ASSERT(NULL != hw_gpio_ptr);
    VSF_HAL_ASSERT(__AIC8800_IO_IS_VAILID_PIN(pin_mask));

    return __hw_io_reg_read(hw_gpio_ptr->is_pmic, &hw_gpio_ptr->GPIO->DR) & pin_mask;
}

vsf_gpio_pin_mask_t vsf_hw_gpio_read(vsf_hw_gpio_t *hw_gpio_ptr)
{
    uint32_t origin_pin_mask, pin_value;
    VSF_HAL_ASSERT(NULL != hw_gpio_ptr);

    if (!hw_gpio_ptr->is_pmic) {
        origin_pin_mask = hw_gpio_ptr->GPIO->MR;
        hw_gpio_ptr->GPIO->MR = VSF_HW_GPIO_PIN_MASK;
        pin_value = hw_gpio_ptr->GPIO->VR;
        hw_gpio_ptr->GPIO->MR = origin_pin_mask;
    } else {
        pin_value = PMIC_MEM_READ((unsigned int)&hw_gpio_ptr->GPIO->VR);
    }

    return pin_value;
}

void vsf_hw_gpio_write(vsf_hw_gpio_t *hw_gpio_ptr, vsf_gpio_pin_mask_t pin_mask, vsf_gpio_pin_mask_t value)
{
    uint32_t origin_pin_mask;

    VSF_HAL_ASSERT(NULL != hw_gpio_ptr);
    VSF_HAL_ASSERT(__AIC8800_IO_IS_VAILID_PIN(pin_mask));

    if (!hw_gpio_ptr->is_pmic) {
        origin_pin_mask = hw_gpio_ptr->GPIO->MR;
        hw_gpio_ptr->GPIO->MR = pin_mask;
        hw_gpio_ptr->GPIO->VR = value;
        hw_gpio_ptr->GPIO->MR = origin_pin_mask;
    } else {
        PMIC_MEM_MASK_WRITE((unsigned int)&hw_gpio_ptr->GPIO->VR, value , pin_mask);
    }

    vsf_protect_t orig = __vsf_gpio_protect();
    hw_gpio_ptr->output_reg = (hw_gpio_ptr->output_reg & ~pin_mask) | (value & pin_mask);
    __vsf_gpio_unprotect(orig);
}

void vsf_hw_gpio_toggle(vsf_hw_gpio_t *hw_gpio_ptr, vsf_gpio_pin_mask_t pin_mask)
{
    VSF_HAL_ASSERT(NULL != hw_gpio_ptr);

    vsf_hw_gpio_write(hw_gpio_ptr, pin_mask, ~hw_gpio_ptr->output_reg);
}

vsf_gpio_capability_t vsf_hw_gpio_capability(vsf_hw_gpio_t *hw_gpio_ptr)
{
    vsf_gpio_capability_t gpio_capability = {
        .is_async = hw_gpio_ptr->is_pmic,
        .is_support_output_and_set = VSF_GPIO_CFG_CAPABILITY_HAS_OUTPUT_AND_SET,
        .is_support_output_and_clear = VSF_GPIO_CFG_CAPABILITY_HAS_OUTPUT_AND_CLEAR,
        .pin_count = VSF_HW_GPIO_PIN_COUNT,
        .avail_pin_mask = VSF_HW_GPIO_PIN_MASK,
    };

    return gpio_capability;
}

/*============================ INCLUDES ======================================*/

#define VSF_GPIO_CFG_IMP_LV0(__COUNT, __HAL_OP)                                         \
    vsf_hw_gpio_t vsf_hw_gpio ## __COUNT = {                                            \
        .GPIO = REG_GPIO ## __COUNT,                                                    \
        .IOMUX = ((AIC_IOMUX_TypeDef *)VSF_HW_IO_PORT ## __COUNT ## _IOMUX_REG_BASE),   \
        .is_pmic = VSF_HW_IO_PORT ## __COUNT ## _IS_PMIC,                               \
        .gpio_pin_mask = VSF_HW_IO_PORT ## __COUNT ## _GPIO_PIN_MASK,                   \
        .output_reg = 0,                                                                \
        __HAL_OP                                                                        \
    };

#include "hal/driver/common/gpio/gpio_template.inc"

#endif      // VSF_HAL_USE_GPIO

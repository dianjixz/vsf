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

/*============================ INCLUDES ======================================*/

#include "hal/driver/common/template/vsf_template_hal_driver.h"

/*============================ MACROS ========================================*/
#if VSF_HAL_USE_I2C == ENABLED

#define vsf_real_i2c_t          VSF_MCONNECT(VSF_I2C_CFG_IMP_PREFIX, _i2c_t)
#define vsf_real_master_request VSF_MCONNECT(VSF_I2C_CFG_IMP_PREFIX, _i2c_master_request)
#define vsf_real_i2c_capability VSF_MCONNECT(VSF_I2C_CFG_IMP_PREFIX, _i2c_capability)

#ifndef VSF_I2C_CFG_REIMPLEMENT_API_CAPABILITY
#   define VSF_I2C_CFG_REIMPLEMENT_API_CAPABILITY       DISABLED
#endif

#if VSF_I2C_CFG_REIMPLEMENT_API_CAPABILITY == DISABLED
#   ifndef VSF_I2C_CFG_CAPABILITY_IS_SUPPORT_RESTART
#       define VSF_I2C_CFG_CAPABILITY_IS_SUPPORT_RESTART                1
#   endif
#   ifndef VSF_I2C_CFG_CAPABILITY_IS_SUPPORT_MANUAL_START_STOP
#       define VSF_I2C_CFG_CAPABILITY_IS_SUPPORT_MANUAL_START_STOP      1
#   endif
#   ifndef VSF_I2C_CFG_CAPABILITY_MAX_TRANSFER_SIZE
#       define VSF_I2C_CFG_CAPABILITY_MAX_TRANSFER_SIZE                 0xFFFF
#   endif
#endif

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ IMPLEMENTATION ================================*/

#if VSF_I2C_CFG_REQUEST_TEMPLATE == ENABLED
vsf_err_t vsf_real_i2c_master_request(vsf_real_i2c_t *i2c_ptr, uint16_t address, vsf_i2c_cmd_t cmd, uint16_t count, uint8_t *buffer_ptr)
{
    vsf_real_i2c_t * i2c_type_ptr = (vsf_real_i2c_t *)i2c_ptr;
    return vsf_i2c_request_master_request(i2c_ptr, &i2c_type_ptr->request, address, cmd, count, buffer_ptr);
}
#endif

#if VSF_I2C_CFG_REIMPLEMENT_API_CAPABILITY == DISABLED
vsf_i2c_capability_t vsf_real_i2c_capability(vsf_real_i2c_t *i2c_ptr)
{
    vsf_i2c_capability_t i2c_capability = {
        .is_support_restart = VSF_I2C_CFG_CAPABILITY_IS_SUPPORT_RESTART,
        .is_support_manual_start_stop = VSF_I2C_CFG_CAPABILITY_IS_SUPPORT_MANUAL_START_STOP,
        .max_transfer_size = VSF_I2C_CFG_CAPABILITY_MAX_TRANSFER_SIZE,
    };
    return i2c_capability;
}
#endif

/*============================ GLOBAL VARIABLES ==============================*/

#define __VSF_HAL_TEMPLATE_NAME                     _i2c
#define __VSF_HAL_TEMPLATE_UPCASE_NAME              _I2C

#ifndef VSF_I2C_CFG_IMP_PREFIX
#   error "Please define VSF_I2C_CFG_IMP_PREFIX in i2c driver"
#endif

#ifndef VSF_I2C_CFG_IMP_UPCASE_PREFIX
#   error "Please define VSF_I2C_CFG_IMP_UPCASE_PREFIX    in i2c driver"
#endif

#include "hal/driver/common/template/vsf_template_implementation.h"

#endif // VSF_HAL_USE_I2C == ENABLED

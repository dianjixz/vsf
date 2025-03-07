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

#include "hal/vsf_hal_cfg.h"

#if VSF_HAL_USE_DEBUG_STREAM == ENABLED
#   if VSF_USE_SIMPLE_STREAM == ENABLED
#       define __VSF_SIMPLE_STREAM_CLASS_INHERIT__
#       include "service/vsf_service.h"
#   endif
#endif
#include "hal/arch/vsf_arch.h"
#include <unistd.h>
#include <termios.h>

/*============================ MACROS ========================================*/

#if VSF_HAL_USE_DEBUG_STREAM == ENABLED
#   ifndef VSF_DEBUG_STREAM_CFG_RX_BUF_SIZE
#       define VSF_DEBUG_STREAM_CFG_RX_BUF_SIZE         1024
#   endif
#endif

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ PROTOTYPES ====================================*/

#if VSF_HAL_USE_DEBUG_STREAM == ENABLED
#   if VSF_USE_SIMPLE_STREAM == ENABLED
static void __vsf_linux_debug_stream_tx_init(vsf_stream_t *stream);
static uint_fast32_t __vsf_linux_debug_stream_tx_write(vsf_stream_t* stream,
            uint8_t* buf, uint_fast32_t size);
static uint_fast32_t __vsf_linux_debug_stream_tx_get_data_length(vsf_stream_t* stream);
static uint_fast32_t __vsf_linux_debug_stream_tx_get_avail_length(vsf_stream_t* stream);
#   elif   VSF_USE_STREAM == ENABLED

#   endif
#endif

/*============================ LOCAL VARIABLES ===============================*/

#if VSF_HAL_USE_DEBUG_STREAM == ENABLED
#   if VSF_USE_SIMPLE_STREAM == ENABLED
static const vsf_stream_op_t __vsf_linux_debug_stream_tx_op = {
    .init = __vsf_linux_debug_stream_tx_init,
    .get_data_length = __vsf_linux_debug_stream_tx_get_data_length,
    .get_avail_length = __vsf_linux_debug_stream_tx_get_avail_length,
    .write = __vsf_linux_debug_stream_tx_write,
};
static uint8_t __vsf_linux_debug_stream_rx_buff[VSF_DEBUG_STREAM_CFG_RX_BUF_SIZE];
static vsf_arch_irq_thread_t __vsf_linux_debug_stream_rx_irq;
#   endif
#endif

static struct termios __vsf_linux_original_term;

/*============================ GLOBAL VARIABLES ==============================*/

#if VSF_HAL_USE_DEBUG_STREAM == ENABLED
#   if VSF_USE_SIMPLE_STREAM == ENABLED
vsf_stream_t VSF_DEBUG_STREAM_TX = {
    .op         = &__vsf_linux_debug_stream_tx_op,
};

vsf_mem_stream_t VSF_DEBUG_STREAM_RX = {
    .op         = &vsf_mem_stream_op,
    .buffer     = __vsf_linux_debug_stream_rx_buff,
    .size       = sizeof(__vsf_linux_debug_stream_rx_buff),
};

#   elif   VSF_USE_STREAM == ENABLED

#   endif
#endif

/*============================ IMPLEMENTATION ================================*/

#if VSF_HAL_USE_DEBUG_STREAM == ENABLED
#   if VSF_USE_SIMPLE_STREAM == ENABLED
static void __vsf_linux_debug_stream_tx_init(vsf_stream_t *stream)
{
    vsf_stream_connect_rx(stream);
    vsf_stream_connect_tx(&VSF_DEBUG_STREAM_RX.use_as__vsf_stream_t);
}

static uint_fast32_t __vsf_linux_debug_stream_tx_write(vsf_stream_t *stream,
            uint8_t *buf, uint_fast32_t size)
{
    ssize_t wsize = write(STDOUT_FILENO, buf, size);
    return wsize < 0 ? 0: wsize;
}

static uint_fast32_t __vsf_linux_debug_stream_tx_get_data_length(vsf_stream_t *stream)
{
    return 0;
}

static uint_fast32_t __vsf_linux_debug_stream_tx_get_avail_length(vsf_stream_t *stream)
{
    return 0xFFFFFFFF;
}

static void __vsf_linux_debug_stream_at_exit(void)
{
    tcsetattr(STDIN_FILENO, TCSANOW, &__vsf_linux_original_term);
}

static void __vsf_linux_debug_stream_rx_irqhandler(void *arg)
{
    vsf_arch_irq_thread_t *thread = arg;
    ssize_t rsize;
    char ch;

    __vsf_arch_irq_set_background(thread);

    if (isatty(STDIN_FILENO)) {
        struct termios term;
        int ret = tcgetattr(STDIN_FILENO, &term);
        VSF_HAL_ASSERT(0 == ret);
        __vsf_linux_original_term = term;

        cfmakeraw(&term);
        term.c_oflag |= ONLCR | OPOST;
        ret = tcsetattr(STDIN_FILENO, TCSANOW, &term);
        VSF_HAL_ASSERT(0 == ret);

        atexit(__vsf_linux_debug_stream_at_exit);
    }

    while (1) {
        do {
            rsize = read(STDIN_FILENO, &ch, 1);
        } while (rsize <= 0);

        // check special inputs
        if ('\003' == ch) {
            // EXT
            exit(0);
        }

        __vsf_arch_irq_start(thread);
            VSF_STREAM_WRITE(&VSF_DEBUG_STREAM_RX, (uint8_t *)&ch, 1);
        __vsf_arch_irq_end(thread, false);
    }
}

static void __vsf_linux_debug_stream_init(void)
{
    VSF_STREAM_CONNECT_TX(&VSF_DEBUG_STREAM_RX);
    __vsf_arch_irq_init(&__vsf_linux_debug_stream_rx_irq, "debug_stream_rx",
        __vsf_linux_debug_stream_rx_irqhandler, vsf_arch_prio_0);
}
#   elif   VSF_USE_STREAM == ENABLED
#   endif
#endif

bool vsf_driver_init(void)
{
#if VSF_HAL_USE_DEBUG_STREAM == ENABLED
    __vsf_linux_debug_stream_init();
#endif
    return true;
}

/* EOF */
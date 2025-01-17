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

#ifndef __USE_COMPILER_DETECT_H__
#define __USE_COMPILER_DETECT_H__

#if   defined(__CPU_ARM__)
#   include "./arm/arm_compiler_detect.h"
#elif   defined(__CPU_MCS51__)
#   include "./mcs51/mcs51_compiler_detect.h"
#elif   defined(__CPU_X86__) || defined(__CPU_X64__) || defined(__CPU_WEBASSEMBLY__)
#   include "./x86/x86_compiler_detect.h"
#elif   defined(__CPU_RV__)
#   include "./rv/rv_compiler_detect.h"
#else
//#warning No specified MCU type! use default which support gcc/clang only
#   include "./default/default_compiler_detect.h"
#endif

#endif

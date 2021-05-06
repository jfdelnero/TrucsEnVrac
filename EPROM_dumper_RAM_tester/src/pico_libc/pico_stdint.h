/*
 * Copyright (c) 2004, 2005 by
 * Ralf Corsepius, Ulm/Germany. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

#ifndef _STDINT_H
#define _STDINT_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ___int8_t_defined
typedef char int8_t ;
typedef unsigned char uint8_t ;
#define __int8_t_defined 1
#endif

#ifdef ___int16_t_defined
typedef short int16_t ;
typedef unsigned short uint16_t ;
#define __int16_t_defined 1
#endif

#ifdef ___int32_t_defined
typedef int int32_t ;
typedef unsigned int uint32_t ;
#define __int32_t_defined 1
#endif

#ifdef __cplusplus
}
#endif

#endif /* _STDINT_H */

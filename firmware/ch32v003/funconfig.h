/**
 * @file funconfig.h
 * @brief CH32V003fun configuration for Little-Wire
 * @author kimstik
 * assisted by intelligence
 */

#ifndef _FUNCONFIG_H
#define _FUNCONFIG_H

#define CH32V003                 1
#define FUNCONF_SYSTICK_USE_HCLK 1  // Required by rv003usb
#define FUNCONF_USE_DEBUGPRINTF  0  // Disable debug printf to save flash

#endif

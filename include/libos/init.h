// Copyright 2026 Jannik Laugmand Bülow

#ifndef LIBOS_INIT_H
#define LIBOS_INIT_H 1

#include "libos/api.h"

#ifdef __cplusplus
extern "C" {
#endif

// Initializes all subsystems of libos.
LIBOS_EXPORT void libos_init();

#ifdef __cplusplus
}
#endif

#endif // LIBOS_INIT_H

/* SPDX-License-Identifier: MIT */
/* Copyright © 2005-2020 Rich Felker, et al.
 * Copyright (c) 2023, Unikraft GmbH and The Unikraft Authors.
 * Licensed under the MIT License (the "License", see COPYING.md).
 * You may not use this file except in compliance with the License.
 */
/*
 * The code in this file was derived from musl 1.2.3:
 * Source: https://www.musl-libc.org/
 * File: src/env/getenv.c
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "environ.h"
#include <sfork.h>

char *getenv_kernel(const char *name)
{
	size_t l = strchrnul(name, '=') - name;
	if (l && !name[l] && __environ)
		for (char **e = __environ; *e; e++)
			if (!strncmp(name, *e, l) && l[*e] == '=')
				return *e + l+1;
	return 0;
}

char *getenv(const char *name) __attribute__((section(".app_libs_text")))
{
	if (sfork_syscalls.getenv)
		return sfork_syscalls.getenv(name);
	else 
		return getenv_kernel(name);
}
/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * libnewlib glue code
 *
 * Authors: Felipe Huici <felipe.huici@neclab.eu>
 * Authors: Florian Schmidt <florian.schmidt@neclab.eu>
 *
 * Copyright (c) 2017, NEC Europe Ltd., NEC Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * THIS HEADER MAY NOT BE EXTRACTED OR MODIFIED IN ANY WAY.
 */

#include <errno.h>
#include <sys/types.h>
#include <reent.h>
#include <sfork.h>
#include <assert.h>
#include <stdio.h>

int getentropy(void *buf __unused, size_t buflen __unused)
{
	/* We don't have a source for high-quality random data,
	 * at least not yet. So we return ENOSYS, as the spec suggests.
	 */
	errno = ENOSYS;
	return -1;
}

#include <uk/plat/bootstrap.h>

void abort(void)
{
	ukplat_terminate(UKPLAT_CRASH);
}

void _exit(int status)
{
	/* To do: convert status to UKPLAT codes */
	ukplat_terminate(status);
}

void srandom_kernel(unsigned int seed)
{
  struct _reent *reent = _REENT;

  _REENT_CHECK_RAND48(reent);
  _REENT_RAND_NEXT(reent) = seed;
}

void srandom(unsigned int seed)__attribute__((section(".app_libs_text")))
{
 	if (sfork_syscalls.srandom)
		return sfork_syscalls.srandom(seed);
	else 
		return srandom_kernel(seed);
}

char * getenv_kernel (const char *name)
{
#ifdef CONFIG_LIBSASSYFORK_TOCTTOU
	char* name_old = name;
	name = sfork_copy_from_user(name, strlen(name)+1);
#endif
  int offset;

  char* ret = _findenv_r (_REENT, name, &offset);
#ifdef CONFIG_LIBSASSYFORK_TOCTTOU
	name = sfork_copy_to_user(name, name_old, strlen(name_old)+1);
	//pathname = pathname_old;
	//uk_free(k_alloc, pathname_old);
#endif
  return ret;
}

char * getenv (const char *name)__attribute__((section(".app_libs_text")))
{
  	if (sfork_syscalls.getenv)
		return sfork_syscalls.getenv(name);
	else 
		return getenv_kernel(name);
}

time_t time_kernel(time_t * t)
{
  struct timeval now;

  if (_gettimeofday_r (_REENT, &now, NULL) >= 0)
    {
      if (t)
	*t = now.tv_sec;
      return now.tv_sec;
    }
  return -1;
}

time_t time(time_t * t)__attribute__((section(".app_libs_text")))
{
	if (sfork_syscalls.time)
		return sfork_syscalls.time(t);
	else 
		return time_kernel(t);
}

#include "local.h"

struct tm *
_DEFUN (localtime_r, (tim_p, res),
	_CONST time_t *__restrict tim_p _AND
	struct tm *__restrict res)
{
  long offset;
  int hours, mins, secs;
  int year;
  __tzinfo_type *_CONST tz = __gettzinfo ();
  _CONST int *ip;

  res = gmtime_r (tim_p, res);

  year = res->tm_year + YEAR_BASE;
  ip = __month_lengths[isleap(year)];

  TZ_LOCK;
  _tzset_unlocked ();
  if (_daylight)
    {
      if (year == tz->__tzyear || __tzcalc_limits (year))
	res->tm_isdst = (tz->__tznorth
	  ? (*tim_p >= tz->__tzrule[0].change
	  && *tim_p < tz->__tzrule[1].change)
	  : (*tim_p >= tz->__tzrule[0].change
	  || *tim_p < tz->__tzrule[1].change));
      else
	res->tm_isdst = -1;
    }
  else
    res->tm_isdst = 0;

  offset = (res->tm_isdst == 1
    ? tz->__tzrule[1].offset
    : tz->__tzrule[0].offset);

  hours = (int) (offset / SECSPERHOUR);
  offset = offset % SECSPERHOUR;

  mins = (int) (offset / SECSPERMIN);
  secs = (int) (offset % SECSPERMIN);

  res->tm_sec -= secs;
  res->tm_min -= mins;
  res->tm_hour -= hours;

  if (res->tm_sec >= SECSPERMIN)
    {
      res->tm_min += 1;
      res->tm_sec -= SECSPERMIN;
    }
  else if (res->tm_sec < 0)
    {
      res->tm_min -= 1;
      res->tm_sec += SECSPERMIN;
    }
  if (res->tm_min >= MINSPERHOUR)
    {
      res->tm_hour += 1;
      res->tm_min -= MINSPERHOUR;
    }
  else if (res->tm_min < 0)
    {
      res->tm_hour -= 1;
      res->tm_min += MINSPERHOUR;
    }
  if (res->tm_hour >= HOURSPERDAY)
    {
      ++res->tm_yday;
      ++res->tm_wday;
      if (res->tm_wday > 6)
	res->tm_wday = 0;
      ++res->tm_mday;
      res->tm_hour -= HOURSPERDAY;
      if (res->tm_mday > ip[res->tm_mon])
	{
	  res->tm_mday -= ip[res->tm_mon];
	  res->tm_mon += 1;
	  if (res->tm_mon == 12)
	    {
	      res->tm_mon = 0;
	      res->tm_year += 1;
	      res->tm_yday = 0;
	    }
	}
    }
  else if (res->tm_hour < 0)
    {
      res->tm_yday -= 1;
      res->tm_wday -= 1;
      if (res->tm_wday < 0)
	res->tm_wday = 6;
      res->tm_mday -= 1;
      res->tm_hour += 24;
      if (res->tm_mday == 0)
	{
	  res->tm_mon -= 1;
	  if (res->tm_mon < 0)
	    {
	      res->tm_mon = 11;
	      res->tm_year -= 1;
	      res->tm_yday = 364 + isleap(res->tm_year + YEAR_BASE);
	    }
	  res->tm_mday = ip[res->tm_mon];
	}
    }
  TZ_UNLOCK;

  return (res);
}

struct tm * localtime_kernel(const time_t * tim_p)
{
  struct _reent *reent = _REENT;

  _REENT_CHECK_TM(reent);
  return localtime_r (tim_p, (struct tm *)_REENT_TM(reent));
}

struct tm * localtime(const time_t * tim_p)__attribute__((section(".app_libs_text")))
{
	if (sfork_syscalls.localtime) {
		//sfork_syscalls.printf("timp  %p\n", tim_p);
		struct tm* tim = (struct tm *) sfork_syscalls.localtime(tim_p);
		//sfork_syscalls.printf("tim  %p\n", tim);
		return tim;
	}
	else 
		return localtime_kernel(tim_p);
}


int *
__errno_kernel ()
{
  return &_REENT->_errno;
}

int *
__errno ()__attribute__((section(".app_libs_text")))
{
 	if (sfork_syscalls.err)
		return sfork_syscalls.err();
	else 
		return __errno_kernel();
}


void __assert_kernel (const char *file, int line, const char *failedexpr)
{
   __assert_func (file, line, NULL, failedexpr);
  /* NOTREACHED */
}

void __assert (const char *file, int line, const char *failedexpr)__attribute__((section(".app_libs_text")))
{
 	if (sfork_syscalls.asser)
		 sfork_syscalls.asser(file, line, failedexpr);
	else 
		return __assert_kernel(file, line, failedexpr);
}

void srand_kernel(unsigned int seed)
{
  struct _reent *reent = _REENT;

  _REENT_CHECK_RAND48(reent);
  _REENT_RAND_NEXT(reent) = seed;
}

void srand(unsigned int seed)__attribute__((section(".app_libs_text")))
{
 	if (sfork_syscalls.srand)
		return sfork_syscalls.srand(seed);
	else 
		return srand_kernel(seed);
}

char *setlocale_kernel(int category, const char *locale)
{
#ifdef CONFIG_LIBSASSYFORK_TOCTTOU
	char* locale_old = locale;
	locale = sfork_copy_from_user(locale, strlen(locale)+1);
#endif
  char* ret =  _setlocale_r (_REENT, category, locale);
#ifdef CONFIG_LIBSASSYFORK_TOCTTOU
	locale = sfork_copy_to_user(locale, locale_old, strlen(locale_old)+1);
#endif
  return ret;
}

char *setlocale(int category, const char *locale)__attribute__((section(".app_libs_text")))
{
 	if (sfork_syscalls.setlocale)
		return sfork_syscalls.setlocale(category, locale);
	else 
		return setlocale_kernel(category, locale);
}

FILE * fopen_kernel(const char *file, const char *mode)
{
#ifdef CONFIG_LIBSASSYFORK_TOCTTOU
	char* file_old = file;
	file = sfork_copy_from_user(file, strlen(file)+1);
  char* mode_old = mode;
	mode = sfork_copy_from_user(mode, strlen(mode)+1);
#endif
  FILE* ret = _fopen_r (_REENT, file, mode);
#ifdef CONFIG_LIBSASSYFORK_TOCTTOU
	file = sfork_copy_to_user(file, file_old, strlen(file_old)+1);
  mode = sfork_copy_to_user(mode, mode_old, strlen(mode_old)+1);
#endif
  return ret;
}

FILE * fopen(_CONST char *file, _CONST char *mode)__attribute__((section(".app_libs_text")))
{
 	if (sfork_syscalls.fopen)
		return sfork_syscalls.fopen(file, mode);
	else 
		return fopen_kernel(file, mode);
}

char * fgets_kernel(char *__restrict buf, int n, FILE *__restrict fp)
{
#ifdef CONFIG_LIBSASSYFORK_TOCTTOU
	char* buf_old = buf;
	buf = sfork_copy_from_user(buf, strlen(buf)+1);
#endif
  char* ret = _fgets_r (_REENT, buf, n, fp);
#ifdef CONFIG_LIBSASSYFORK_TOCTTOU
	buf = sfork_copy_to_user(buf, buf_old, strlen(buf_old)+1);
#endif
  return ret;
}

char * fgets(char *__restrict buf, int n, FILE *__restrict fp)__attribute__((section(".app_libs_text")))
{
 	if (sfork_syscalls.fgets)
		return sfork_syscalls.fgets(buf, n, fp);
	else 
		return fgets_kernel(buf, n, fp);
}




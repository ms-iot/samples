//******************************************************************
//
// Copyright 2014 Samsung Electronics All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#ifndef TIMER_H_
#define TIMER_H_

#if !defined(WITH_ARDUINO) && !defined(WIN32)
#include <sys/time.h>
#else
#include <Time.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include <math.h>

#ifndef WITH_ARDUINO
#define SECS_PER_MIN  (60L)
#define SECS_PER_HOUR (SECS_PER_MIN * 60L)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24L)
#define DAYS_PER_WEEK (7L)
#define SECS_PER_WEEK (SECS_PER_DAY * DAYS_PER_WEEK)
#define SECS_PER_YEAR (SECS_PER_WEEK * 52L)
#define SECS_YR_2000  (946684800L)
#endif

time_t timespec_diff(const time_t after, const time_t before);
void timespec_add(time_t * to, const time_t seconds);
void checkTimeout();

#ifndef WITH_ARDUINO
long int getSeconds(struct tm* tp);
long int getRelativeIntervalOfWeek(struct tm* tp);
long int getSecondsFromAbsTime(struct tm* tp);

int initThread();
void *loop(void *threadid);
time_t registerTimer(const time_t seconds, int *id, void *cb);
void unregisterTimer(int id);

#else

time_t timeToSecondsFromNow(tmElements_t *t);
time_t registerTimer(const time_t seconds, int *id,  void (*cb)());
void unregisterTimer(int id);


#endif

#ifdef __cplusplus
}
#endif
#endif /* TIMER_H_ */


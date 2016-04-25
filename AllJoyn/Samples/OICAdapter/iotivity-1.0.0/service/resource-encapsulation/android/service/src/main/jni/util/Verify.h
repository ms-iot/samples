/******************************************************************
 *
 * Copyright 2015 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************/

#ifndef RCS_JNI_VERIFY_H_
#define RCS_JNI_VERIFY_H_

#define VERIFY_NO_EXC(ENV) do { \
    if ((ENV)->ExceptionCheck()) { (ENV)->ExceptionDescribe(); return; } } while(false)

#define VERIFY_NO_EXC_RET(ENV, RET) do { \
    if ((ENV)->ExceptionCheck()) { (ENV)->ExceptionDescribe(); return RET; } } while(false)

#define VERIFY_NO_EXC_RET_DEF(ENV) VERIFY_NO_EXC_RET(ENV, { })

#endif // RCS_JNI_VERIFY_H_

#/******************************************************************
#*
#* Copyright 2015 Samsung Electronics All Rights Reserved.
#*
#*
#*
#* Licensed under the Apache License, Version 2.0 (the "License");
#* you may not use this file except in compliance with the License.
#* You may obtain a copy of the License at
#*
#*      http://www.apache.org/licenses/LICENSE-2.0
#*
#* Unless required by applicable law or agreed to in writing, software
#* distributed under the License is distributed on an "AS IS" BASIS,
#* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#* See the License for the specific language governing permissions and
#* limitations under the License.
#*
#******************************************************************/

#Specify Android.mk path w.r.t APPLICATION_BUILD in the Makefile
APP_PROJECT_PATH = ./

APP_STL = gnustl_shared
APP_PLATFORM = android-21
APP_CPPFLAGS += -fexceptions
APP_CPPFLAGS += -frtti += -Wno-error=format-security
APP_CFLAGS = -Wno-error=format-security

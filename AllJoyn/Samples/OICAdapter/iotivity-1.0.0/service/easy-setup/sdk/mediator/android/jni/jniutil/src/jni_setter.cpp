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
#include "jni_setter.h"

#include <string>

#include "JniOcResource.h"

#define LOG_TAG "TM_JSetter"

bool JSetter::setJStringField(JNIEnv *env, jobject &object,
		const char *fieldName, const char *value) {
	if (NULL == env || NULL == fieldName) {
		LOGE("setJStringField invalid parameters");
		return false;
	}

	jclass clazz = env->GetObjectClass(object);
	if (NULL == clazz) {
		LOGE("GetObjectClass failed");
		return false;
	}

	jfieldID fieldID = env->GetFieldID(clazz, fieldName, "Ljava/lang/String;");
	if (0 == fieldID) {
		LOGE("GetFieldID failed [%s]", fieldName);
		env->DeleteLocalRef(clazz);
		return false;
	}

	jstring jvalue;
	if (value != NULL && strlen(value) > 0) {
		jclass strClass = env->FindClass("java/lang/String");
		jmethodID ctorID = env->GetMethodID(strClass, "<init>",
				"([BLjava/lang/String;)V");
		jbyteArray bytes = env->NewByteArray(strlen(value));
		env->SetByteArrayRegion(bytes, 0, strlen(value), (jbyte *) value);
		jstring encoding = env->NewStringUTF("utf-8");
		jvalue = (jstring) env->NewObject(strClass, ctorID, bytes, encoding);
		env->DeleteLocalRef(strClass);
		env->DeleteLocalRef(bytes);
		env->DeleteLocalRef(encoding);
	} else {
		jvalue = env->NewStringUTF("");
	}

	env->SetObjectField(object, fieldID, jvalue);

	env->DeleteLocalRef(jvalue);
	env->DeleteLocalRef(clazz);

	return true;
}

bool JSetter::setJIntField(JNIEnv *env, jobject &object, const char *fieldName,
		int value) {
	if (NULL == env || NULL == fieldName) {
		LOGE("setJIntField invalid paramter");
		return false;
	}

	jclass clazz = env->GetObjectClass(object);
	if (NULL == clazz) {
		LOGE("GetObjectClass failed");
		return false;
	}

	jfieldID fieldID = env->GetFieldID(clazz, fieldName, "I");
	if (0 == fieldID) {
		LOGE("GetFieldID failed [%s]", fieldName);
		env->DeleteLocalRef(clazz);
		return false;
	}
	env->SetIntField(object, fieldID, value);

	env->DeleteLocalRef(clazz);

	return true;
}

bool JSetter::setJLongField(JNIEnv *env, jobject &object, const char *fieldName,
		jlong value) {
	if (NULL == env || NULL == fieldName) {
		LOGE("setJLongField invalid parameters");
		return false;
	}

	jclass clazz = env->GetObjectClass(object);
	if (NULL == clazz) {
		LOGE("GetObjectClass failed");
		return false;
	}

	jfieldID fieldID = env->GetFieldID(clazz, fieldName, "J");
	if (0 == fieldID) {
		LOGE("GetFieldID failed [%s]", fieldName);
		env->DeleteLocalRef(clazz);
		return false;
	}
	env->SetLongField(object, fieldID, value);

	env->DeleteLocalRef(clazz);

	return true;
}

bool JSetter::setJBoolField(JNIEnv *env, jobject &object, const char *fieldName,
		bool value) {
	if (NULL == env || NULL == fieldName) {
		LOGE("setJBoolField invalid parameters");
		return false;
	}

	jclass clazz = env->GetObjectClass(object);
	if (NULL == clazz) {
		LOGE("GetObjectClass failed");
		return false;
	}

	jfieldID fieldID = env->GetFieldID(clazz, fieldName, "Z");
	if (0 == fieldID) {
		LOGE("GetFieldID failed [%s]", fieldName);
		env->DeleteLocalRef(clazz);
		return false;
	}
	env->SetBooleanField(object, fieldID, value);

	env->DeleteLocalRef(clazz);

	return true;
}

bool JSetter::setJObjectField(JNIEnv *env, jobject &object,
		const char *fieldName, const char *fieldType, const jobject value) {
	if (NULL == env || NULL == fieldName) {
		LOGE("setJBoolField invalid parameters");
		return false;
	}

	jclass clazz = env->GetObjectClass(object);
	if (NULL == clazz) {
		LOGE("GetObjectClass failed");
		return false;
	}

	jfieldID fieldID = env->GetFieldID(clazz, fieldName, fieldType);
	if (0 == fieldID) {
		LOGE("GetFieldID failed [%s] [%s]", fieldName, fieldType);
		env->DeleteLocalRef(clazz);
		return false;
	}
	env->SetObjectField(object, fieldID, value);

	env->DeleteLocalRef(clazz);

	return true;
}


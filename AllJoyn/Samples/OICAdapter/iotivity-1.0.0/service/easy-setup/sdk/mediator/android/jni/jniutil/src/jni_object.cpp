#include "jni_object.h"

//#define NULL 0
#define LOG_TAG "TM_JObject"

JObject::JObject(JNIEnv *env) :
		m_pEnv(env), m_pObject(NULL), m_pClazz(NULL), m_fIsNewObject(true) {
}

JObject::JObject(JNIEnv *env, jobject obj) :
		m_pEnv(NULL), m_pObject(NULL), m_pClazz(NULL), m_fIsNewObject(false) {
	if (NULL == env || NULL == obj) {
		return;
	}

	m_pEnv = env;
	m_pObject = obj;
	m_pClazz = m_pEnv->GetObjectClass(obj);
}

JObject::JObject(JNIEnv *env, const char *classPath) :
		m_pEnv(NULL), m_pObject(NULL), m_pClazz(NULL), m_fIsNewObject(true) {
	if (NULL == env || NULL == classPath) {
		LOGI("JObject Invalid parameters");
		return;
	}

	m_pEnv = env;
	//m_pClazz = GetJClass( classPath );

	if (NULL == m_pClazz) {
		LOGE("GetJClass failed [%s]", classPath);
		return;
	}

	jmethodID mid = env->GetMethodID(m_pClazz, "<init>", "()V");
	if (NULL == mid) {
		LOGE("GetMethodID failed [%s]", classPath);
		return;
	}

	m_pObject = env->NewObject(m_pClazz, mid);
}

JObject::~JObject() {
	if (m_pEnv) {
		if (m_pObject && m_fIsNewObject) {
			m_pEnv->DeleteLocalRef(m_pObject);
		}

		if (m_pClazz && !m_fIsNewObject) {
			m_pEnv->DeleteLocalRef(m_pClazz);
		}
	}
}

jobject JObject::getObject() const {
	return m_pObject;
}

void JObject::detachObject() {
	if (m_fIsNewObject) {
		m_fIsNewObject = false;
		m_pClazz = NULL;
	}
}


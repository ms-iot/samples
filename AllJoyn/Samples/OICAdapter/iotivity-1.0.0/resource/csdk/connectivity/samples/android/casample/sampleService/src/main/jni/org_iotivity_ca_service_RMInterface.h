#include <jni.h>
/* Header for class org_iotivity_ca_service_RMInterface */

#ifndef _Included_org_iotivity_ca_service_RMInterface
#define _Included_org_iotivity_ca_service_RMInterface
#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT void JNICALL Java_org_iotivity_ca_service_RMInterface_setNativeResponseListener
  (JNIEnv *, jobject, jobject);
/*
 * Class:     org_iotivity_ca_service_RMInterface
 * Method:    RMInitialize
 * Signature: (Landroid/content/Context;)V
 */
JNIEXPORT void JNICALL Java_org_iotivity_ca_service_RMInterface_RMInitialize
  (JNIEnv *, jobject, jobject);

/*
 * Class:     org_iotivity_ca_service_RMInterface
 * Method:    RMTerminate
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_iotivity_ca_service_RMInterface_RMTerminate
  (JNIEnv *, jobject);

/*
 * Class:     org_iotivity_ca_service_RMInterface
 * Method:    RMStartListeningServer
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_iotivity_ca_service_RMInterface_RMStartListeningServer
  (JNIEnv *, jobject);

/*
 * Class:     org_iotivity_ca_service_RMInterface
 * Method:    RMStartDiscoveryServer
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_iotivity_ca_service_RMInterface_RMStartDiscoveryServer
  (JNIEnv *, jobject);

/*
 * Class:     org_iotivity_ca_service_RMInterface
 * Method:    RMRegisterHandler
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_iotivity_ca_service_RMInterface_RMRegisterHandler
  (JNIEnv *, jobject);

/*
 * Class:     org_iotivity_ca_service_RMInterface
 * Method:    RMSendRequest
 * Signature: (Ljava/lang/String;Ljava/lang/String;III)V
 */
JNIEXPORT void JNICALL Java_org_iotivity_ca_service_RMInterface_RMSendRequest
  (JNIEnv *, jobject, jstring, jstring, jint, jint, jint, jboolean);

/*
 * Class:     org_iotivity_ca_service_RMInterface
 * Method:    RMSendReqestToAll
 * Signature: (Ljava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_org_iotivity_ca_service_RMInterface_RMSendReqestToAll
  (JNIEnv *, jobject, jstring, jint);

/*
 * Class:     org_iotivity_ca_service_RMInterface
 * Method:    RMSendResponse
 * Signature: (IIII)V
 */
JNIEXPORT void JNICALL Java_org_iotivity_ca_service_RMInterface_RMSendResponse
  (JNIEnv *, jobject, jint, jint, jint, jint);

/*
 * Class:     org_iotivity_ca_service_RMInterface
 * Method:    RMSendNotification
 * Signature: (Ljava/lang/String;Ljava/lang/String;IIII)V
 */
JNIEXPORT void JNICALL Java_org_iotivity_ca_service_RMInterface_RMSendNotification
  (JNIEnv *, jobject, jstring, jstring, jint, jint, jint);

/*
 * Class:     org_iotivity_ca_service_RMInterface
 * Method:    RMSelectNetwork
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_org_iotivity_ca_service_RMInterface_RMSelectNetwork
  (JNIEnv *, jobject, jint);

/*
 * Class:     org_iotivity_ca_service_RMInterface
 * Method:    RMUnSelectNetwork
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_org_iotivity_ca_service_RMInterface_RMUnSelectNetwork
  (JNIEnv *, jobject, jint);

/*
 * Class:     org_iotivity_ca_service_RMInterface
 * Method:    RMGetNetworkInfomation
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_iotivity_ca_service_RMInterface_RMGetNetworkInfomation
  (JNIEnv *, jobject);

/*
 * Class:     org_iotivity_ca_service_RMInterface
 * Method:    RMHandleRequestResponse
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_iotivity_ca_service_RMInterface_RMHandleRequestResponse
  (JNIEnv *, jobject);

#ifdef __cplusplus
}
#endif
#endif

/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class edu_cs300_MessageJNI */

#ifndef _Included_edu_cs300_MessageJNI
#define _Included_edu_cs300_MessageJNI
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     edu_cs300_MessageJNI
 * Method:    readStringMsg
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_edu_cs300_MessageJNI_readStringMsg
  (JNIEnv *, jclass);

/*
 * Class:     edu_cs300_MessageJNI
 * Method:    readPrefixRequestMsg
 * Signature: ()Ledu/cs300/SearchRequest;
 */
JNIEXPORT jobject JNICALL Java_edu_cs300_MessageJNI_readPrefixRequestMsg
  (JNIEnv *, jclass);

/*
 * Class:     edu_cs300_MessageJNI
 * Method:    writeLongestWordResponseMsg
 * Signature: (ILjava/lang/String;ILjava/lang/String;Ljava/lang/String;II)V
 */
JNIEXPORT void JNICALL Java_edu_cs300_MessageJNI_writeLongestWordResponseMsg
  (JNIEnv *, jclass, jint, jstring, jint, jstring, jstring, jint, jint);

#ifdef __cplusplus
}
#endif
#endif

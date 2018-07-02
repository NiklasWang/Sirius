#include <jni.h>
#include <android/log.h>
#include <utils/misc.h>
#include <android_runtime/AndroidRuntime.h>
#include "Sirius.h"

Sirius *gSirius;
#if 0
#ifdef __cplusplus
extern "C" {
#endif
#endif

//#define CLASS_NAME "com/example/sirius/MainActivity"
#define CLASS_NAME "com/zui/camera/one/v2/MacroOneCameraImpl"

struct fields_t {
    jfieldID    context;
    jmethodID   post_event;
};

static fields_t fields;
JavaVM *gJavaVm;
static jclass gAppClass;
//static jclass gPointClass;
#define TAG "liujianlong"
using namespace android;

JNIEXPORT jint JNICALL JNI_UnOnLoad(JavaVM *vm, void *reserved)
{
    (void)vm;
    (void)reserved;

    ALOGE("liujianlong: unload jni and delete sirius");
    if (gSirius) {
        delete gSirius;
        gSirius = NULL;
    }
    return 0;
}
#if 0
void initClassHelper(JNIEnv *env, const char *path, jobject *objptr)
{
    jclass cls = env->FindClass(path);
    if (!cls) {
        ALOGE("%s: find class error", __func__);
        return;
    }

    jmethodID constr = env->GetMethodID(cls, "<init>", "()V");
    if (!constr) {
        ALOGE("%s: failed to getmethod id", __func__);
        return;
    }

    jobject obj = env->NewObject(cls, constr);
    if (!obj) {
        ALOGE("%s: failed to new object", __func__);
        return;
    }

    (*objptr) = env->NewGlobalRef(obj);
}
#endif

int32_t dataCallback(RequestType type, void * /*buf*/)
{
   ALOGE("%s: liujianlong received data", __func__);
   JNIEnv *env;
   if (gJavaVm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK) {
       ALOGE("Failed to get the environment using GetEnv()");
       return -1;
   }
#if 0
   jclass appClass = env->GetObjectClass(gAppClass);
   if(!appClass) {
        ALOGE("%s: liujianlong getobjectclass error", __func__);
        return -1;
   }
#endif
   env->CallStaticVoidMethod(gAppClass, fields.post_event,
                             type, type, type, NULL);

   return 0;
}
/*
 * Class:     com_example_sirius_MainActivity
 * Method:    sirius_init
 * Signature: (I)I
 */
static int Java_com_example_sirius_MainActivity_sirius_init
  (JNIEnv *, jclass, jint) {
    gSirius = new Sirius();
    if (gSirius != NULL)
        ALOGE("liujianlong jni call ok");

    gSirius->setCallback(dataCallback);
    return 0;
}

/*
 * Class:     com_example_sirius_MainActivity
 * Method:    sirius_destroy
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_com_example_sirius_MainActivity_sirius_destroy
  (JNIEnv *, jclass, jint) {
    ALOGE("liujianlong: destroy and delete sirius");
    if (gSirius) {
        delete gSirius;
        gSirius = NULL;
    }
return 0;
}

/*
 * Class:     com_example_sirius_MainActivity
 * Method:    sirius_addCallbackBuffer
 * Signature: ([BI)I
 */
JNIEXPORT jint JNICALL Java_com_example_sirius_MainActivity_sirius_addCallbackBuffer
  (JNIEnv *env, jclass /*cls*/, jbyteArray pixelBuffer, jint msgType){
      int32_t bufSize = static_cast<int32_t>(env->GetArrayLength(pixelBuffer));
      jbyte* pixels = env->GetByteArrayElements(pixelBuffer, /*is_copy*/NULL);

      gSirius->enqueueBuf((RequestType)msgType, (unsigned char *)pixels, bufSize);

      gSirius->request(PREVIEW_NV21);

      env->ReleaseByteArrayElements(pixelBuffer, pixels, JNI_ABORT);
      return 0;
}

static const JNINativeMethod methods[] = {
    {
        "siriusInit",
        "(I)I",
        (void*)Java_com_example_sirius_MainActivity_sirius_init
    },
    {
        "siriusAddCallbackBuffer",
        "([BI)I",
        (void*)Java_com_example_sirius_MainActivity_sirius_addCallbackBuffer
    },
    {
        "siriusDestroy",
        "(I)I",
        (void*)Java_com_example_sirius_MainActivity_sirius_destroy
    }
};

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
{
    ALOGE("%s liujianlong ++onload", __func__);
    (void)reserved;
    gJavaVm = vm;
    JNIEnv *env;
    if (vm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK) {
        ALOGE("Failed to get the environment using GetEnv()");
        return -1;
    }

    //initClassHelper(env, CLASS_NAME, &gAppClass);
    jclass cls = env->FindClass(CLASS_NAME);
    if (!cls) {
        ALOGE("failed to find class");
        return -1;
    }
    gAppClass = (jclass) env->NewGlobalRef(cls);
    #if 1
    fields.post_event = env->GetStaticMethodID(cls, "postEventFromNative",
              "(IIILjava/lang/Object;)V");

    if (AndroidRuntime::registerNativeMethods(env,
        CLASS_NAME, methods, NELEM(methods)) != JNI_OK) {
        ALOGE("Failed to register native methods");
        return -1;
    }
        #endif
    return JNI_VERSION_1_4;
}
#if 0
#ifdef __cplusplus
}
#endif
#endif

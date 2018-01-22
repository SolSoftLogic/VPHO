#include <string.h>
#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include <android/log.h>

#define  LOG_TAG    "qt2jni"
#if 1
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#else
#define  LOGI(...)  while(0){;}
#define  LOGE(...)  while(0){;}
#endif

extern volatile char *camerabuffer ;
extern volatile char *tmpcamerabuffer ;

extern volatile int cameracaptured ;
extern volatile int cameraflushed  ;
extern volatile int camerax;
extern volatile int cameray;
extern volatile int cameraformat ;
extern volatile int startcapture;

extern volatile int  audiocaptured ;

extern volatile char *audioinbuffer ;
extern volatile char *tmpaudioinbuffer ;
extern volatile char *tmpaudiooutbuffer ;

extern volatile int commandtothread;
extern volatile int audioframesize;

extern volatile int	    bytes_received;
extern volatile int	    audioreceived ;
#include <pthread.h>

extern  volatile pthread_mutex_t javacs;
extern  volatile pthread_mutex_t javacmd;

#define EnterCriticalSection(a) pthread_mutex_lock(a)                                                                          
#define LeaveCriticalSection(a) pthread_mutex_unlock(a)                                                                        
#define DeleteCriticalSection(a) pthread_mutex_destroy(a) 

void InitCriticalSection(pthread_mutex_t *a)
{
//	static const pthread_mutex_t tmp = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
	static const pthread_mutex_t tmp = PTHREAD_RECURSIVE_MUTEX_INITIALIZER;
	
	*a = tmp;
	pthread_mutex_init(a, 0);
}


extern   void   VPSIMPLEAUDIO_Send(void *dummy ,int size);
extern  void SendToTrack(char *buffer);

jint Java_com_nokia_qt_android_CmdThread_GetNativeCommand(JNIEnv* env, jclass clazz)
{
    jint cmd;
    
//    VPSIMPLEAUDIO_Send(&tmpaudioinbuffer,0);
    if (commandtothread >0)
    {
	cmd = commandtothread;
	LOGI("JINI COMAMND PROCESSED ------ %i\n",cmd);
	return cmd;
    }
}

jint Java_com_nokia_qt_android_CmdThread_ClearNativeCommand(JNIEnv* env, jclass clazz)
{
    jint cmd;
    if (commandtothread >0)
    {
	cmd = commandtothread;
	commandtothread = 0;
	LOGI("JINI COMAMND Cleared ------ %i\n",cmd);
	return cmd;
    }
}

void Java_com_nokia_qt_android_CmdThread_InitCs(JNIEnv* env, jclass clazz)
{
    InitCriticalSection(&javacmd);
}


void Java_com_nokia_qt_android_medium_PutVideoFrame(JNIEnv* env, jclass clazz,jbyteArray jvideoData,jint framesize , jint x, jint y)
{
    if (startcapture)
    {
	jbyte* pjVideoData = (*env)->GetByteArrayElements(env, jvideoData, 0);
//    PutCameraFrame(pjVideoData);
//#if VIDEOLOOPBACK
	if (!cameracaptured)
	{
	    camerax = x;
	    cameray = y;
	    memcpy((void *)tmpcamerabuffer,pjVideoData , framesize);
	    cameracaptured = 1;
	}
//#endif
    (*env)->ReleaseByteArrayElements(env, jvideoData, pjVideoData,0);
    
    }
}

void Java_com_nokia_qt_android_AudioIn_PutAudioFrame(JNIEnv* env, jclass clazz,jbyteArray jvideoData,jint framesize , jint x, jint y)
{
    jbyte* pjVideoData = (*env)->GetByteArrayElements(env, jvideoData, 0);
    audioframesize = framesize;
    memcpy((void *)&tmpaudioinbuffer,pjVideoData , framesize);
    VPSIMPLEAUDIO_Send(&tmpaudioinbuffer,framesize);
    (*env)->ReleaseByteArrayElements(env, jvideoData, pjVideoData,0);
}


jbyteArray Java_com_nokia_qt_android_AudioIn_GetAudioFrame(JNIEnv* env, jclass clazz)
{
    jbyteArray jbOut;//

    if (audioreceived)
    {
        jbOut = (*env)->NewByteArray(env, bytes_received );
        (*env)->SetByteArrayRegion(env, jbOut, 0,  bytes_received, (jbyte *)&tmpaudiooutbuffer);
        audioreceived = 0;
        return jbOut;
    }
    else
    {
	jbOut = (*env)->NewByteArray(env, 0 );
	return jbOut;
    }
}

jbyteArray Java_com_nokia_qt_android_AudioOut_GetAudioFrame(JNIEnv* env, jclass clazz)
{
    jbyteArray jbOut;//
	if (audioreceived)
	{
	    jbOut = (*env)->NewByteArray(env, bytes_received );
	    LOGI("GetAudioFrame JINI AUDIO Frame  NewByteArray ok %i\n",bytes_received);
    	    (*env)->SetByteArrayRegion(env, jbOut, 0,  bytes_received, (jbyte *)&tmpaudiooutbuffer);
    	    audioreceived = 0;
    	    return jbOut;
	}else
	{
	    jbOut = (*env)->NewByteArray(env, 0 );
	    return jbOut;
	}
}




JNIEnv* menv;
jobject mobj;
int trackinited = 0;
jmethodID audioCBID;
jbyteArray jData;

void SendToTrack(char *buffer){
    jbyteArray jbOut;//
    if (trackinited)
    {

	(*menv)->SetByteArrayRegion(menv, jData, 0,  640, (jbyte *)&tmpaudiooutbuffer);
        (*menv)->CallVoidMethod(menv,mobj, audioCBID,jData);
    }
    if (audioreceived)
    {
	audioreceived = 0;
    }
    
}

void Java_com_nokia_qt_android_AudioOut_setEnv(JNIEnv* env, jobject obj)
{
    menv = env;
    mobj = obj;


    LOGI("ENV called\n");
    
    jData = (*env)->NewByteArray(env, 640 );
    jclass cls = (*env)->GetObjectClass(env,obj);
    audioCBID = (*env)->GetMethodID(env,cls, "audioCB", "([B)V");

    if ( !audioCBID )
    {
        return;
    }

    trackinited = 1;
    LOGI("TRY TO SLEEP\n");
    EnterCriticalSection(&javacs);
    LOGI("WAKE UP\n");
    LeaveCriticalSection(&javacs);
    trackinited = 0;
    LOGI("ENV Exited\n");

}
JNIEXPORT void Java_com_nokia_qt_android_AudioOut_audioFunc(JNIEnv* env, jobject obj,jbyteArray jData)
{
    jclass cls = (*env)->GetObjectClass(env,obj);

    jmethodID audioCBID = (*env)->GetMethodID(env,cls, "audioCB", "()V");

    if ( !audioCBID )
    {
        return;
    }

    (*env)->CallVoidMethod(env,obj, audioCBID,jData);
}

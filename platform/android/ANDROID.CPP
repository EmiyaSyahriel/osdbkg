#include "../../src/OSDBKG.HPP"
#include <android/log.h>
#include <jni.h>
#include <cstdarg>

#define EXPORT(ret) extern "C" JNIEXPORT ret JNICALL

void osd_log_write(const char* t, const char* tag, const char* fmt, va_list va){
    android_LogPriority prio = android_LogPriority::ANDROID_LOG_DEFAULT;
    bool print = true;
    switch(t[0]){
        case 'D':
            prio = android_LogPriority ::ANDROID_LOG_DEBUG;
            break;
        case 'E':
            prio = android_LogPriority ::ANDROID_LOG_ERROR;
            break;
        case 'W':
            prio = android_LogPriority ::ANDROID_LOG_WARN;
            break;
        case 'I':
            prio = android_LogPriority ::ANDROID_LOG_INFO;
            break;
        default:
            // Do not print if unknown
            print = false;
            break;
    }
    if(print){
        __android_log_vprint(prio, tag, fmt, va);
    }
}

EXPORT(void) Java_id_psw_osdbkg_LibOsdBkg_start(JNIEnv *env, jobject thiz) {
    osd_init();
}

EXPORT(void) Java_id_psw_osdbkg_LibOsdBkg_resize(JNIEnv *env, jobject thiz, int w, int h) {
    glViewport(0,0,w,h);
    states.screen.w = w;
    states.screen.h = h;
    states.screen.update = true;
}

EXPORT(void) Java_id_psw_osdbkg_LibOsdBkg_draw(JNIEnv *env, jobject thiz, float ms) {
    osd_draw(ms);
    states.screen.update = false;
}
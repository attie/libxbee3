#include <stdio.h>
#include "jni.h"

#include "../../xbee.h"

#include "uk_co_attie_libxbee_libxbee.h"
#include "uk_co_attie_libxbee_libxbee_err.h"

/* useful websites:
  	http://java.sun.com/docs/books/jni/html/objtypes.html
  	http://www.javaworld.com/javaworld/javatips/jw-javatip54.html
*/

JNIEXPORT jint JNICALL Java_uk_co_attie_libxbee_libxbee_xbee_1setup(JNIEnv *env, jobject obj, jobject retHandle) {
	printf("hello\n");
	return 0;
}

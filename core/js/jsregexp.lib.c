#include "jsstddef.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "jstypes.h"
#include "jsarena.h" /* Added by JSIFY */
#include "jsutil.h" /* Added by JSIFY */
#include "jsapi.h"
#include "jsarray.h"
#include "jsatom.h"
#include "jscntxt.h"
#include "jsconfig.h"
#include "jsfun.h"
#include "jsgc.h"
#include "jsinterp.h"
#include "jslock.h"
#include "jsnum.h"
#include "jsobj.h"
#include "jsopcode.h"
#include "jsregexp.h"
#include "jsscan.h"
#include "jsscope.h"
#include "jsstr.h"

JSRegExp * js_NewRegExpOpt(JSContext *cx, JSString *str, JSString *opt, JSBool flat){
	return NULL;
}
JSBool js_SetLastIndex(JSContext *cx, JSObject *obj, jsdouble lastIndex){
    return JS_TRUE;
}
void js_DestroyRegExp(JSContext *cx, JSRegExp *re){
}
JSBool js_ExecuteRegExp(JSContext *cx, JSRegExp *re, JSString *str, size_t *indexp,JSBool test, jsval *rval){
    return JS_TRUE;
}
JSBool js_InitRegExpStatics(JSContext *cx, JSRegExpStatics *res){
    return JS_TRUE;
}
void js_FreeRegExpStatics(JSContext *cx, JSRegExpStatics *res){}
JSClass js_RegExpClass = {
    js_RegExp_str,
    JSCLASS_HAS_PRIVATE | JSCLASS_HAS_RESERVED_SLOTS(1) |
    JSCLASS_MARK_IS_TRACE | JSCLASS_HAS_CACHED_PROTO(JSProto_RegExp),
    JS_PropertyStub,    JS_PropertyStub,
    JS_PropertyStub,    JS_PropertyStub,
    JS_EnumerateStub,   JS_ResolveStub,
    JS_ConvertStub,     JS_FinalizeStub,
    NULL,NULL,NULL,NULL,NULL,NULL,NULL,0
};
JSBool js_regexp_toString(JSContext *cx, JSObject *obj, jsval *vp){
    return JS_TRUE;
}
JSObject * js_InitRegExpClass(JSContext *cx, JSObject *obj){
    return NULL;
}
JSObject * js_NewRegExpObject(JSContext *cx, JSTokenStream *ts,jschar *chars, size_t length, uintN flags){
	return NULL;
}
JSObject * js_CloneRegExpObject(JSContext *cx, JSObject *obj, JSObject *parent){
    return NULL;
}


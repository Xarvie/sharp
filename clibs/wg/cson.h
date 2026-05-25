/*
 * cson.h  —  C JSON Library  (header-only, C11)
 *
 * Usage:
 *   #define CSON_IMPLEMENTATION   ← in exactly ONE .c file
 *   #include "cson.h"
 *
 * Features:
 *   - Full RFC 8259 parser  (string escapes, \uXXXX, all number forms)
 *   - Programmatic builder  (object / array / string / number / bool / null)
 *   - Serialiser            (compact or pretty-printed)
 *   - Path accessor         cson_get(obj, "key")  /  cson_idx(arr, i)
 *   - Deep copy, merge
 *   - Zero external dependencies, no dynamic linking
 */

#ifndef CSON_H
#define CSON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* =========================================================================
 * Types
 * ========================================================================= */

typedef enum {
    CSON_NULL   = 0,
    CSON_BOOL,
    CSON_NUMBER,
    CSON_STRING,
    CSON_ARRAY,
    CSON_OBJECT
} cson_type_t;

typedef struct cson cson_t;

/* =========================================================================
 * Lifecycle
 * ========================================================================= */

/** Parse a JSON string. Returns NULL on error. Caller owns result → cson_free(). */
cson_t     *cson_parse   (const char *json);
cson_t     *cson_parse_n (const char *json, size_t len);   /* bounded */

/** Deep-free a node and all children. */
void        cson_free    (cson_t *node);

/** Deep copy. */
cson_t     *cson_clone   (const cson_t *node);

/* =========================================================================
 * Construction
 * ========================================================================= */

cson_t *cson_null   (void);
cson_t *cson_bool   (bool v);
cson_t *cson_number (double v);
cson_t *cson_integer(int64_t v);       /* convenience, stored as double */
cson_t *cson_string (const char *s);   /* copies s */
cson_t *cson_string_n(const char *s, size_t len);
cson_t *cson_array  (void);
cson_t *cson_object (void);

/* =========================================================================
 * Array mutation
 * ========================================================================= */

/** Append item to array.  Returns 0 on success, -1 on OOM. */
int    cson_push  (cson_t *arr, cson_t *item);
int    cson_insert(cson_t *arr, int idx, cson_t *item);
void   cson_remove(cson_t *arr, int idx);   /* frees item */
/** Pop last element; caller owns returned node. */
cson_t *cson_pop  (cson_t *arr);

/* =========================================================================
 * Object mutation
 * ========================================================================= */

/** Set key → value (copies key, takes ownership of value).
 *  Overwrites existing key if present. Returns 0 / -1. */
int  cson_set  (cson_t *obj, const char *key, cson_t *value);
int  cson_set_n(cson_t *obj, const char *key, size_t klen, cson_t *value);

/** Remove key. Returns 0 if found, -1 if not. */
int  cson_del  (cson_t *obj, const char *key);

/** Merge src object into dst (shallow, overwrites on collision). */
int  cson_merge(cson_t *dst, const cson_t *src);

/* =========================================================================
 * Access
 * ========================================================================= */

cson_type_t  cson_type  (const cson_t *n);
bool         cson_is_null(const cson_t *n);

/** Object: get value for key (returns NULL if not found). */
cson_t      *cson_get   (const cson_t *obj, const char *key);

/** Array: get element at index. */
cson_t      *cson_idx   (const cson_t *arr, int i);

/** Number of elements (array) or keys (object). */
int          cson_len   (const cson_t *n);

/* Typed value extraction — return default if type mismatch */
bool         cson_bool_v  (const cson_t *n);
double       cson_num_v   (const cson_t *n);
int64_t      cson_int_v   (const cson_t *n);
const char  *cson_str_v   (const cson_t *n);    /* NULL if not string */

/** Object key iteration.  *key and *value are valid until next mutation.
 *  idx in [0, cson_len(obj)). */
void cson_obj_iter(const cson_t *obj, int idx, const char **key, cson_t **value);

/* =========================================================================
 * Serialisation
 * ========================================================================= */

/** Returns newly malloc'd JSON string. Caller must free(). NULL on OOM. */
char  *cson_dumps  (const cson_t *node);

/** Pretty-printed with given indent (e.g. "  " for 2 spaces). */
char  *cson_dumps_pp(const cson_t *node, const char *indent);

/** Write into caller-supplied buffer. Returns bytes written (excl. NUL),
 *  or required size if buf==NULL or buf_sz==0. */
size_t cson_write  (const cson_t *node, char *buf, size_t buf_sz);

/* =========================================================================
 * Convenience builders  (all return fresh cson_t* or NULL on OOM)
 * ========================================================================= */

/** Build {"key1":v1, "key2":v2, ... } from pairs.  Terminated by NULL key. */
cson_t *cson_obj1(const char *k, cson_t *v, ...);     /* vararg, NULL sentinel */

/** Quick string-value object: cson_obj_kv("type","patch", "id","lbl", NULL) */
cson_t *cson_obj_kv(const char *k, const char *v, ...); /* NULL sentinel */

#ifdef __cplusplus
}
#endif

/* =========================================================================
 * IMPLEMENTATION
 * ========================================================================= */

#ifdef CSON_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <errno.h>

/* ---- Internal node structure ---- */

typedef struct { char *key; cson_t *val; } cson_kv_t;

struct cson {
    cson_type_t type;
    union {
        bool      boolean;
        double    number;
        char     *string;
        struct { cson_t   **items; int len, cap; } arr;
        struct { cson_kv_t *pairs; int len, cap; } obj;
    };
};

/* =========================================================================
 * Allocator helpers
 * ========================================================================= */

static cson_t *cn_new(cson_type_t t){
    cson_t *n=(cson_t*)calloc(1,sizeof(cson_t)); if(n)n->type=t; return n;
}
static char *cn_strdup(const char *s, size_t len){
    char *d=(char*)malloc(len+1); if(d){memcpy(d,s,len);d[len]='\0';} return d;
}

/* =========================================================================
 * Construction
 * ========================================================================= */

cson_t *cson_null(void)           { return cn_new(CSON_NULL); }
cson_t *cson_bool(bool v)         { cson_t *n=cn_new(CSON_BOOL); if(n)n->boolean=v; return n; }
cson_t *cson_number(double v)     { cson_t *n=cn_new(CSON_NUMBER); if(n)n->number=v; return n; }
cson_t *cson_integer(int64_t v)   { return cson_number((double)v); }
cson_t *cson_string_n(const char *s,size_t l){
    cson_t *n=cn_new(CSON_STRING); if(!n)return NULL;
    n->string=cn_strdup(s,l); if(!n->string){free(n);return NULL;} return n;
}
cson_t *cson_string(const char *s){ return s ? cson_string_n(s,strlen(s)) : cson_null(); }
cson_t *cson_array(void)          { return cn_new(CSON_ARRAY); }
cson_t *cson_object(void)         { return cn_new(CSON_OBJECT); }

/* =========================================================================
 * Array mutation
 * ========================================================================= */

int cson_push(cson_t *a, cson_t *item){
    if(!a||a->type!=CSON_ARRAY||!item)return -1;
    if(a->arr.len>=a->arr.cap){
        int nc=a->arr.cap?a->arr.cap*2:4;
        cson_t **np=(cson_t**)realloc(a->arr.items,nc*sizeof(cson_t*));
        if(!np)return -1; a->arr.items=np; a->arr.cap=nc;
    }
    a->arr.items[a->arr.len++]=item; return 0;
}
int cson_insert(cson_t *a,int idx,cson_t *item){
    if(!a||a->type!=CSON_ARRAY||!item||idx<0||idx>a->arr.len)return -1;
    if(cson_push(a,NULL)<0)return -1;   /* grow */
    a->arr.len--;                        /* undo count increment */
    memmove(a->arr.items+idx+1,a->arr.items+idx,(size_t)(a->arr.len-idx)*sizeof(cson_t*));
    a->arr.items[idx]=item; a->arr.len++; return 0;
}
void cson_remove(cson_t *a, int idx){
    if(!a||a->type!=CSON_ARRAY||idx<0||idx>=a->arr.len)return;
    cson_free(a->arr.items[idx]);
    memmove(a->arr.items+idx,a->arr.items+idx+1,(size_t)(a->arr.len-idx-1)*sizeof(cson_t*));
    a->arr.len--;
}
cson_t *cson_pop(cson_t *a){
    if(!a||a->type!=CSON_ARRAY||!a->arr.len)return NULL;
    return a->arr.items[--a->arr.len];
}

/* =========================================================================
 * Object mutation
 * ========================================================================= */

int cson_set_n(cson_t *o, const char *key, size_t klen, cson_t *val){
    if(!o||o->type!=CSON_OBJECT||!key||!val)return -1;
    /* Overwrite if exists */
    for(int i=0;i<o->obj.len;i++){
        if(strlen(o->obj.pairs[i].key)==klen &&
           memcmp(o->obj.pairs[i].key,key,klen)==0){
            cson_free(o->obj.pairs[i].val);
            o->obj.pairs[i].val=val; return 0;
        }
    }
    /* Insert new */
    if(o->obj.len>=o->obj.cap){
        int nc=o->obj.cap?o->obj.cap*2:4;
        cson_kv_t *np=(cson_kv_t*)realloc(o->obj.pairs,nc*sizeof(cson_kv_t));
        if(!np)return -1; o->obj.pairs=np; o->obj.cap=nc;
    }
    char *k=cn_strdup(key,klen); if(!k)return -1;
    o->obj.pairs[o->obj.len].key=k;
    o->obj.pairs[o->obj.len].val=val;
    o->obj.len++; return 0;
}
int cson_set(cson_t *o, const char *key, cson_t *val){
    return key ? cson_set_n(o,key,strlen(key),val) : -1;
}
int cson_del(cson_t *o, const char *key){
    if(!o||o->type!=CSON_OBJECT||!key)return -1;
    for(int i=0;i<o->obj.len;i++){
        if(strcmp(o->obj.pairs[i].key,key)==0){
            free(o->obj.pairs[i].key); cson_free(o->obj.pairs[i].val);
            memmove(o->obj.pairs+i,o->obj.pairs+i+1,(size_t)(o->obj.len-i-1)*sizeof(cson_kv_t));
            o->obj.len--; return 0;
        }
    }
    return -1;
}
int cson_merge(cson_t *dst, const cson_t *src){
    if(!dst||dst->type!=CSON_OBJECT||!src||src->type!=CSON_OBJECT)return -1;
    for(int i=0;i<src->obj.len;i++){
        cson_t *v=cson_clone(src->obj.pairs[i].val);
        if(!v||cson_set(dst,src->obj.pairs[i].key,v)<0){ cson_free(v); return -1; }
    }
    return 0;
}

/* =========================================================================
 * Lifecycle
 * ========================================================================= */

void cson_free(cson_t *n){
    if(!n)return;
    switch(n->type){
    case CSON_STRING: free(n->string); break;
    case CSON_ARRAY:
        for(int i=0;i<n->arr.len;i++) cson_free(n->arr.items[i]);
        free(n->arr.items); break;
    case CSON_OBJECT:
        for(int i=0;i<n->obj.len;i++){
            free(n->obj.pairs[i].key); cson_free(n->obj.pairs[i].val);
        }
        free(n->obj.pairs); break;
    default: break;
    }
    free(n);
}

cson_t *cson_clone(const cson_t *n){
    if(!n) return NULL;
    switch(n->type){
    case CSON_NULL:   return cson_null();
    case CSON_BOOL:   return cson_bool(n->boolean);
    case CSON_NUMBER: return cson_number(n->number);
    case CSON_STRING: return cson_string(n->string);
    case CSON_ARRAY:{
        cson_t *a=cson_array(); if(!a)return NULL;
        for(int i=0;i<n->arr.len;i++){
            cson_t *c=cson_clone(n->arr.items[i]);
            if(!c||cson_push(a,c)<0){cson_free(c);cson_free(a);return NULL;}
        }
        return a;
    }
    case CSON_OBJECT:{
        cson_t *o=cson_object(); if(!o)return NULL;
        for(int i=0;i<n->obj.len;i++){
            cson_t *c=cson_clone(n->obj.pairs[i].val);
            if(!c||cson_set(o,n->obj.pairs[i].key,c)<0){cson_free(c);cson_free(o);return NULL;}
        }
        return o;
    }
    }
    return NULL;
}

/* =========================================================================
 * Access
 * ========================================================================= */

cson_type_t cson_type (const cson_t *n){ return n?n->type:CSON_NULL; }
bool        cson_is_null(const cson_t *n){ return !n||n->type==CSON_NULL; }

cson_t *cson_get(const cson_t *o, const char *key){
    if(!o||o->type!=CSON_OBJECT||!key)return NULL;
    for(int i=0;i<o->obj.len;i++)
        if(strcmp(o->obj.pairs[i].key,key)==0) return o->obj.pairs[i].val;
    return NULL;
}
cson_t *cson_idx(const cson_t *a, int i){
    if(!a||a->type!=CSON_ARRAY||i<0||i>=a->arr.len)return NULL;
    return a->arr.items[i];
}
int cson_len(const cson_t *n){
    if(!n)return 0;
    if(n->type==CSON_ARRAY) return n->arr.len;
    if(n->type==CSON_OBJECT)return n->obj.len;
    return 0;
}

bool       cson_bool_v(const cson_t *n){ return (n&&n->type==CSON_BOOL)?n->boolean:false; }
double     cson_num_v (const cson_t *n){ return (n&&n->type==CSON_NUMBER)?n->number:0.0; }
int64_t    cson_int_v (const cson_t *n){ return (int64_t)cson_num_v(n); }
const char*cson_str_v (const cson_t *n){ return (n&&n->type==CSON_STRING)?n->string:NULL; }

void cson_obj_iter(const cson_t *o,int idx,const char **key,cson_t **val){
    if(!o||o->type!=CSON_OBJECT||idx<0||idx>=o->obj.len){if(key)*key=NULL;if(val)*val=NULL;return;}
    if(key)*key=o->obj.pairs[idx].key;
    if(val)*val=o->obj.pairs[idx].val;
}

/* =========================================================================
 * PARSER  (recursive descent, RFC 8259)
 * ========================================================================= */

typedef struct { const char *p, *end; char errbuf[64]; } ps_t;

static void ps_skip_ws(ps_t *ps){
    while(ps->p<ps->end && (*ps->p==' '||*ps->p=='\t'||*ps->p=='\r'||*ps->p=='\n'))
        ps->p++;
}
static cson_t *ps_value(ps_t *ps);  /* forward decl */

static uint32_t ps_hex4(ps_t *ps){
    uint32_t v=0;
    for(int i=0;i<4;i++){
        if(ps->p>=ps->end)return 0;
        char c=*ps->p++;
        if(c>='0'&&c<='9')v=(v<<4)+(uint32_t)(c-'0');
        else if(c>='a'&&c<='f')v=(v<<4)+(uint32_t)(c-'a'+10);
        else if(c>='A'&&c<='F')v=(v<<4)+(uint32_t)(c-'A'+10);
        else return 0;
    }
    return v;
}

/* Encode a Unicode codepoint to UTF-8, returns bytes written */
static int utf8_encode(uint32_t cp, char *out){
    if(cp<0x80){out[0]=(char)cp;return 1;}
    if(cp<0x800){out[0]=(char)(0xC0|(cp>>6));out[1]=(char)(0x80|(cp&0x3F));return 2;}
    if(cp<0x10000){out[0]=(char)(0xE0|(cp>>12));out[1]=(char)(0x80|((cp>>6)&0x3F));out[2]=(char)(0x80|(cp&0x3F));return 3;}
    out[0]=(char)(0xF0|(cp>>18));out[1]=(char)(0x80|((cp>>12)&0x3F));
    out[2]=(char)(0x80|((cp>>6)&0x3F));out[3]=(char)(0x80|(cp&0x3F));return 4;
}

static cson_t *ps_string(ps_t *ps){
    if(ps->p>=ps->end||*ps->p!='"')return NULL;
    ps->p++;
    /* Two-pass: measure, then copy */
    const char *start=ps->p; size_t cap=0;
    const char *q=ps->p;
    while(q<ps->end&&*q!='"'){
        if(*q=='\\'){q++;if(q<ps->end){q+=(*q=='u')?5:1;cap+=4;continue;}}
        cap++; q++;
    }
    char *buf=(char*)malloc(cap+1); if(!buf)return NULL;
    size_t len=0;
    ps->p=start;
    while(ps->p<ps->end&&*ps->p!='"'){
        if(*ps->p!='\\'){buf[len++]=(char)*ps->p++;continue;}
        ps->p++;
        if(ps->p>=ps->end)break;
        switch(*ps->p++){
        case '"':  buf[len++]='"';  break;
        case '\\': buf[len++]='\\'; break;
        case '/':  buf[len++]='/';  break;
        case 'b':  buf[len++]='\b'; break;
        case 'f':  buf[len++]='\f'; break;
        case 'n':  buf[len++]='\n'; break;
        case 'r':  buf[len++]='\r'; break;
        case 't':  buf[len++]='\t'; break;
        case 'u':{
            uint32_t cp=ps_hex4(ps);
            /* surrogate pair */
            if(cp>=0xD800&&cp<=0xDBFF&&ps->p+1<ps->end&&*ps->p=='\\'&&*(ps->p+1)=='u'){
                ps->p+=2; uint32_t lo=ps_hex4(ps);
                if(lo>=0xDC00&&lo<=0xDFFF) cp=0x10000+((cp-0xD800)<<10)+(lo-0xDC00);
            }
            len+=(size_t)utf8_encode(cp,buf+len);
            break;
        }
        default: buf[len++]='?'; break;
        }
    }
    if(ps->p<ps->end&&*ps->p=='"') ps->p++;
    buf[len]='\0';
    cson_t *n=cson_string_n(buf,len); free(buf); return n;
}

static cson_t *ps_number(ps_t *ps){
    const char *s=ps->p;
    if(*ps->p=='-')ps->p++;
    while(ps->p<ps->end&&(*ps->p>='0'&&*ps->p<='9'))ps->p++;
    if(ps->p<ps->end&&*ps->p=='.'){ps->p++;while(ps->p<ps->end&&(*ps->p>='0'&&*ps->p<='9'))ps->p++;}
    if(ps->p<ps->end&&(*ps->p=='e'||*ps->p=='E')){
        ps->p++;
        if(ps->p<ps->end&&(*ps->p=='+'||*ps->p=='-'))ps->p++;
        while(ps->p<ps->end&&(*ps->p>='0'&&*ps->p<='9'))ps->p++;
    }
    char tmp[64]; size_t l=(size_t)(ps->p-s);
    if(l>=sizeof(tmp))return NULL;
    memcpy(tmp,s,l);tmp[l]='\0';
    char *ep; double v=strtod(tmp,&ep);
    return cson_number(v);
}

static cson_t *ps_array(ps_t *ps){
    ps->p++; /* skip '[' */
    cson_t *a=cson_array(); if(!a)return NULL;
    ps_skip_ws(ps);
    if(ps->p<ps->end&&*ps->p==']'){ps->p++;return a;}
    while(ps->p<ps->end){
        ps_skip_ws(ps);
        cson_t *item=ps_value(ps);
        if(!item){cson_free(a);return NULL;}
        if(cson_push(a,item)<0){cson_free(item);cson_free(a);return NULL;}
        ps_skip_ws(ps);
        if(ps->p>=ps->end)break;
        if(*ps->p==']'){ps->p++;return a;}
        if(*ps->p!=','){cson_free(a);return NULL;}
        ps->p++;
    }
    cson_free(a); return NULL;
}

static cson_t *ps_object(ps_t *ps){
    ps->p++; /* skip '{' */
    cson_t *o=cson_object(); if(!o)return NULL;
    ps_skip_ws(ps);
    if(ps->p<ps->end&&*ps->p=='}'){ps->p++;return o;}
    while(ps->p<ps->end){
        ps_skip_ws(ps);
        if(*ps->p!='"'){cson_free(o);return NULL;}
        cson_t *ks=ps_string(ps); if(!ks){cson_free(o);return NULL;}
        char *key=cn_strdup(ks->string,strlen(ks->string)); cson_free(ks);
        if(!key){cson_free(o);return NULL;}
        ps_skip_ws(ps);
        if(ps->p>=ps->end||*ps->p!=':'){free(key);cson_free(o);return NULL;}
        ps->p++;
        ps_skip_ws(ps);
        cson_t *val=ps_value(ps);
        if(!val){free(key);cson_free(o);return NULL;}
        int r=cson_set(o,key,val); free(key);
        if(r<0){cson_free(val);cson_free(o);return NULL;}
        ps_skip_ws(ps);
        if(ps->p>=ps->end)break;
        if(*ps->p=='}'){ps->p++;return o;}
        if(*ps->p!=','){cson_free(o);return NULL;}
        ps->p++;
    }
    cson_free(o); return NULL;
}

static cson_t *ps_value(ps_t *ps){
    ps_skip_ws(ps);
    if(ps->p>=ps->end) return NULL;
    char c=*ps->p;
    if(c=='"') return ps_string(ps);
    if(c=='{') return ps_object(ps);
    if(c=='[') return ps_array(ps);
    if(c=='t'&&ps->end-ps->p>=4&&memcmp(ps->p,"true",4)==0){ps->p+=4;return cson_bool(true);}
    if(c=='f'&&ps->end-ps->p>=5&&memcmp(ps->p,"false",5)==0){ps->p+=5;return cson_bool(false);}
    if(c=='n'&&ps->end-ps->p>=4&&memcmp(ps->p,"null",4)==0){ps->p+=4;return cson_null();}
    if(c=='-'||(c>='0'&&c<='9')) return ps_number(ps);
    return NULL;
}

cson_t *cson_parse_n(const char *json, size_t len){
    if(!json)return NULL;
    ps_t ps={json,json+len,{0}};
    cson_t *n=ps_value(&ps);
    return n;
}
cson_t *cson_parse(const char *json){
    return json ? cson_parse_n(json,strlen(json)) : NULL;
}

/* =========================================================================
 * SERIALISER
 * ========================================================================= */

typedef struct { char *buf; size_t len, cap; int oom; } sb_t;

static void sb_grow(sb_t *s, size_t need){
    if(s->oom||s->len+need<=s->cap)return;
    size_t nc=s->cap?s->cap*2:256;
    while(nc<s->len+need)nc*=2;
    char *np=(char*)realloc(s->buf,nc);
    if(!np){s->oom=1;return;} s->buf=np; s->cap=nc;
}
static void sb_char(sb_t *s, char c){ sb_grow(s,1); if(!s->oom)s->buf[s->len++]=c; }
static void sb_str (sb_t *s, const char *p, size_t l){ sb_grow(s,l); if(!s->oom){memcpy(s->buf+s->len,p,l);s->len+=l;} }
static void sb_cstr(sb_t *s, const char *p){ sb_str(s,p,strlen(p)); }

static void sb_json_string(sb_t *s, const char *p){
    sb_char(s,'"');
    while(*p){
        unsigned char c=(unsigned char)*p++;
        if(c=='"') sb_str(s,"\\\"",2);
        else if(c=='\\')sb_str(s,"\\\\",2);
        else if(c=='\n')sb_str(s,"\\n",2);
        else if(c=='\r')sb_str(s,"\\r",2);
        else if(c=='\t')sb_str(s,"\\t",2);
        else if(c<0x20){ char tmp[8]; snprintf(tmp,sizeof(tmp),"\\u%04x",c); sb_cstr(s,tmp); }
        else sb_char(s,(char)c);
    }
    sb_char(s,'"');
}

static void sb_node(sb_t *s, const cson_t *n, const char *indent, int depth){
    if(!n){sb_cstr(s,"null");return;}
    switch(n->type){
    case CSON_NULL:   sb_cstr(s,"null"); break;
    case CSON_BOOL:   sb_cstr(s,n->boolean?"true":"false"); break;
    case CSON_NUMBER:{
        char tmp[64];
        double v=n->number;
        if(v==(int64_t)v&&!isinf(v)&&!isnan(v))snprintf(tmp,sizeof(tmp),"%lld",(long long)(int64_t)v);
        else snprintf(tmp,sizeof(tmp),"%.17g",v);
        sb_cstr(s,tmp); break;
    }
    case CSON_STRING: sb_json_string(s,n->string); break;
    case CSON_ARRAY:{
        sb_char(s,'[');
        for(int i=0;i<n->arr.len;i++){
            if(indent){
                sb_char(s,'\n');
                for(int d=0;d<=depth;d++)sb_cstr(s,indent);
            }
            sb_node(s,n->arr.items[i],indent,depth+1);
            if(i<n->arr.len-1){ sb_char(s,','); if(!indent)sb_char(s,' '); }
        }
        if(indent&&n->arr.len){sb_char(s,'\n');for(int d=0;d<depth;d++)sb_cstr(s,indent);}
        sb_char(s,']'); break;
    }
    case CSON_OBJECT:{
        sb_char(s,'{');
        for(int i=0;i<n->obj.len;i++){
            if(indent){
                sb_char(s,'\n');
                for(int d=0;d<=depth;d++)sb_cstr(s,indent);
            }
            sb_json_string(s,n->obj.pairs[i].key);
            sb_char(s,':');
            if(indent)sb_char(s,' ');
            sb_node(s,n->obj.pairs[i].val,indent,depth+1);
            if(i<n->obj.len-1){ sb_char(s,','); if(!indent)sb_char(s,' '); }
        }
        if(indent&&n->obj.len){sb_char(s,'\n');for(int d=0;d<depth;d++)sb_cstr(s,indent);}
        sb_char(s,'}'); break;
    }
    }
}

char *cson_dumps(const cson_t *n){
    sb_t s={0}; sb_node(&s,n,NULL,0);
    if(s.oom){free(s.buf);return NULL;}
    sb_char(&s,'\0'); return s.buf;
}
char *cson_dumps_pp(const cson_t *n, const char *indent){
    sb_t s={0}; sb_node(&s,n,indent?indent:"  ",0);
    if(s.oom){free(s.buf);return NULL;}
    sb_char(&s,'\0'); return s.buf;
}
size_t cson_write(const cson_t *n, char *buf, size_t sz){
    char *s=cson_dumps(n); if(!s)return 0;
    size_t l=strlen(s);
    if(buf&&sz>0){size_t cp=l<sz-1?l:sz-1;memcpy(buf,s,cp);buf[cp]='\0';}
    free(s); return l;
}

/* =========================================================================
 * Convenience builders
 * ========================================================================= */

cson_t *cson_obj1(const char *k, cson_t *v, ...){
    cson_t *o=cson_object(); if(!o)return NULL;
    va_list ap; va_start(ap,v);
    const char *key=k; cson_t *val=v;
    while(key){
        if(!val||cson_set(o,key,val)<0){cson_free(val);cson_free(o);va_end(ap);return NULL;}
        key=va_arg(ap,const char*);
        if(!key)break;
        val=va_arg(ap,cson_t*);
    }
    va_end(ap); return o;
}

cson_t *cson_obj_kv(const char *k, const char *v, ...){
    cson_t *o=cson_object(); if(!o)return NULL;
    va_list ap; va_start(ap,v);
    const char *key=k, *val=v;
    while(key){
        cson_t *vs=cson_string(val);
        if(!vs||cson_set(o,key,vs)<0){cson_free(vs);cson_free(o);va_end(ap);return NULL;}
        key=va_arg(ap,const char*);
        if(!key)break;
        val=va_arg(ap,const char*);
    }
    va_end(ap); return o;
}

#endif /* CSON_IMPLEMENTATION */
#endif /* CSON_H */

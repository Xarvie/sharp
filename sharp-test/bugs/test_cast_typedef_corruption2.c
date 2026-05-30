#define REPRO_INTERNAL

typedef unsigned char Uint8;
typedef unsigned int Uint32;
typedef int Bool;

typedef struct MyRect { int x, y; int w, h; } MyRect;

#ifndef REPRO_INTERNAL
struct MySurface { int flags; int format; int w, h; };
#endif
typedef struct MySurface MySurface;

typedef struct {
    MySurface *src_surface; Uint8 *src; int src_w;
    MySurface *dst_surface; Uint8 *dst; int dst_w;
    Uint8 *table; int flags; Uint32 colorkey; Uint8 r, g, b, a;
} MyBlitInfo;

typedef void (*MyBlitFunc)(MyBlitInfo *info);

typedef Bool (*MyBlit)(struct MySurface *src, const MyRect *srcrect,
                        struct MySurface *dst, const MyRect *dstrect);

typedef struct {
    int identity; MyBlit blit; void *data; MyBlitInfo info;
    Uint32 dst_ver; Uint32 src_ver;
} MyBlitMap;

struct MySurface {
    int flags; int format; int w, h; int pitch;
    void *pixels; int refcount; void *reserved;
    int internal_flags; MyRect clip_rect; MyBlitMap map;
};

static void dummy(MyBlitInfo *info) { (void)info; }

MyBlitFunc bug_trigger = (MyBlitFunc)dummy;

Uint8 test_val;
MyRect test_rect;

int test_original(void) {
    test_val = 42;
    test_rect.x = 1;
    return (int)test_val + test_rect.x;
}

typedef struct { int a; Uint32 b; } InnerA;
typedef struct { InnerA x; Uint8 y; } InnerB;
typedef InnerB *(*TransformFunc)(InnerA *a);

static InnerB *transform(InnerA *a) {
    static InnerB r;
    r.x = *a;
    r.y = 0;
    return &r;
}

TransformFunc tfn = (TransformFunc)transform;

int test_nested_anon_struct(void) {
    InnerA a;
    a.a = 10;
    a.b = 20;
    return a.a + (int)a.b;
}

typedef unsigned long long Uint64;

typedef struct {
    Uint64 low;
    Uint64 high;
} Pair64;

typedef Pair64 (*CombineFunc)(Uint64 a, Uint64 b);
typedef struct { CombineFunc fn; Pair64 last; } Combiner;

static Pair64 simple_combine(Uint64 a, Uint64 b) {
    Pair64 p;
    p.low = a;
    p.high = b;
    return p;
}

Combiner comb = { (CombineFunc)simple_combine, {0, 0} };

int test_deep_chain(void) {
    return comb.last.low == 0 ? 1 : 0;
}

typedef struct Node Node;
typedef struct { Node *head; Uint32 count; } NodeList;
struct Node { int value; Node *next; };

static void clear_list(NodeList *l) { (void)l; }

typedef void (*ListOp)(NodeList *);
ListOp list_op = (ListOp)clear_list;

int test_forward_typedef_chain(void) {
    NodeList l;
    l.head = 0;
    l.count = 0;
    return l.count == 0 ? 1 : 0;
}

typedef enum { MODE_A, MODE_B, MODE_C } OpMode;
typedef struct { OpMode mode; Uint32 flags; } Config;
typedef Config *(*MakeConfig)(OpMode m);

static Config *make_config(OpMode m) {
    static Config c;
    c.mode = m;
    c.flags = 0;
    return &c;
}

MakeConfig mcfg = (MakeConfig)make_config;

int test_enum_typedef_in_struct(void) {
    Config c;
    c.mode = MODE_A;
    c.flags = 42;
    return (int)c.flags;
}

typedef struct { Uint8 data[4]; } SmallBuf;
typedef struct { SmallBuf buf; Uint32 crc; } Packet;
typedef void (*SendFunc)(Packet *p);

static void send_packet(Packet *p) { (void)p; }

SendFunc sender = (SendFunc)send_packet;

int test_array_typedef_in_struct(void) {
    Packet p;
    p.crc = 99;
    return (int)p.crc;
}

typedef Bool (*Validator)(const MyRect *r);
typedef struct { Validator v; MyRect bounds; } Viewport;

static Bool always_valid(const MyRect *r) { (void)r; return 1; }

Viewport vp = { (Validator)always_valid, {0, 0, 100, 100} };

int test_fn_ptr_typedef_with_const_param(void) {
    return vp.bounds.w;
}

typedef struct { int ref; } RefCounted;
typedef RefCounted *(*CreateFunc)(void);
typedef struct { CreateFunc create; RefCounted instance; } Factory;

static RefCounted *create_ref(void) {
    static RefCounted r;
    r.ref = 1;
    return &r;
}

Factory fac = { (CreateFunc)create_ref, {0} };

int test_typedef_chain_with_return_struct(void) {
    return fac.instance.ref;
}

typedef void (*Callback)(void);
typedef struct { Callback cb; Uint32 id; } Handler;
typedef Handler *(*MakeHandler)(Callback cb, Uint32 id);

static Handler *make_handler(Callback cb, Uint32 id) {
    static Handler h;
    h.cb = cb;
    h.id = id;
    return &h;
}

MakeHandler mh = (MakeHandler)make_handler;

int test_multiple_fn_ptr_typedefs(void) {
    return mh != 0 ? 1 : 0;
}

typedef struct { Uint32 width; Uint32 height; } Dimensions;
typedef struct { Dimensions dim; Uint8 *pixels; } Image;
typedef Image *(*ResizeFunc)(const Image *src, Dimensions new_dim);

static Image *resize_image(const Image *src, Dimensions new_dim) {
    (void)src;
    static Image img;
    img.dim = new_dim;
    img.pixels = 0;
    return &img;
}

ResizeFunc resizer = (ResizeFunc)resize_image;

int test_typedef_with_pointer_field(void) {
    Dimensions d;
    d.width = 640;
    d.height = 480;
    return (int)d.width + (int)d.height;
}

typedef struct Window Window;
typedef struct { Window *parent; Uint32 style; } WinAttr;
typedef WinAttr *(*GetWinAttr)(Window *w);
struct Window { WinAttr attr; Window *next; };

static WinAttr *get_attr(Window *w) {
    static WinAttr a;
    a.parent = 0;
    a.style = 0;
    return &a;
}

GetWinAttr gwa = (GetWinAttr)get_attr;

int test_mutual_forward_typedef(void) {
    WinAttr a;
    a.parent = 0;
    a.style = 77;
    return (int)a.style;
}

typedef struct Conn Conn;
typedef struct { Conn *peer; Uint32 timeout; } ConnOpts;
typedef Bool (*ConnHandler)(Conn *c, ConnOpts *opts);
typedef struct { ConnHandler on_connect; ConnOpts defaults; } ConnManager;
struct Conn { ConnManager *mgr; int fd; };

static Bool on_conn(Conn *c, ConnOpts *opts) { (void)c; (void)opts; return 1; }

ConnManager cmgr = { (ConnHandler)on_conn, {0, 30} };

int test_triple_interdependent_typedef(void) {
    return (int)cmgr.defaults.timeout;
}

typedef struct Slot Slot;
typedef void (*SlotFunc)(Slot *s, Uint32 data);
typedef struct { SlotFunc fn; Uint32 id; } SlotBinding;
struct Slot { SlotBinding binding; Slot *next; };

static void slot_handler(Slot *s, Uint32 data) { (void)s; (void)data; }

SlotBinding sb = { (SlotFunc)slot_handler, 55 };

int test_self_referential_struct_via_fn_ptr(void) {
    return (int)sb.id;
}

typedef struct Device Device;
typedef struct { Device *dev; Uint8 status; } DevInfo;
typedef DevInfo *(*ProbeFunc)(Device *d);
typedef struct { ProbeFunc probe; DevInfo info; } DevManager;
struct Device { DevManager *mgr; int id; };

static DevInfo *probe_dev(Device *d) {
    static DevInfo di;
    di.dev = d;
    di.status = 1;
    return &di;
}

DevManager dmgr = { (ProbeFunc)probe_dev, {0, 0} };

int test_typedef_in_field_and_fnptr_param(void) {
    DevInfo di;
    di.dev = 0;
    di.status = 88;
    return (int)di.status;
}

typedef struct Channel Channel;
typedef void (*ChanCallback)(Channel *ch, Uint32 event);
typedef struct { ChanCallback cb; Uint32 mask; } ChanFilter;
typedef ChanFilter *(*ChanCreateFilter)(Uint32 mask);
struct Channel { ChanFilter *filter; int id; };

static ChanFilter *create_filter(Uint32 mask) {
    static ChanFilter f;
    f.cb = 0;
    f.mask = mask;
    return &f;
}

ChanCreateFilter ccf = (ChanCreateFilter)create_filter;

int test_deep_fn_ptr_typedef_chain(void) {
    ChanFilter f;
    f.cb = 0;
    f.mask = 255;
    return (int)f.mask;
}

typedef struct Task Task;
typedef Task *(*TaskCreator)(Uint32 id, Uint32 prio);
typedef struct { TaskCreator create; Uint32 count; } TaskPool;
struct Task { Uint32 id; Uint32 prio; Task *next; };

static Task *create_task(Uint32 id, Uint32 prio) {
    static Task t;
    t.id = id;
    t.prio = prio;
    t.next = 0;
    return &t;
}

TaskPool tpool = { (TaskCreator)create_task, 0 };

int test_multiple_casts_same_tu(void) {
    TaskPool p;
    p.count = 5;
    return (int)p.count;
}

typedef struct Buffer Buffer;
typedef struct { Buffer *buf; Uint32 len; } BufView;
typedef BufView *(*BufMapFunc)(Buffer *b, Uint32 offset);
typedef struct { BufMapFunc map; BufView view; } BufAdapter;
struct Buffer { Uint8 *data; Uint32 size; BufAdapter adapter; };

static BufView *map_buf(Buffer *b, Uint32 offset) {
    static BufView v;
    v.buf = b;
    v.len = offset;
    return &v;
}

BufAdapter badapter = { (BufMapFunc)map_buf, {0, 0} };

int test_struct_containing_adapter_with_fnptr(void) {
    BufView v;
    v.buf = 0;
    v.len = 123;
    return (int)v.len;
}

typedef struct Event Event;
typedef Bool (*EventFilter)(Event *e);
typedef struct { EventFilter filter; Uint32 type; } EventRule;
typedef struct { EventRule *rules; Uint32 count; } EventSystem;
struct Event { Uint32 type; EventSystem *sys; };

static Bool default_filter(Event *e) { (void)e; return 1; }

EventRule erule = { (EventFilter)default_filter, 42 };

int test_event_system_pattern(void) {
    EventRule r;
    r.filter = 0;
    r.type = 99;
    return (int)r.type;
}

typedef struct Plugin Plugin;
typedef struct { Plugin *next; Uint32 api_version; } PluginInfo;
typedef PluginInfo *(*PluginInit)(Uint32 version);
typedef struct { PluginInit init; PluginInfo info; } PluginHost;
struct Plugin { PluginHost *host; char name[32]; };

static PluginInfo *plugin_init(Uint32 version) {
    static PluginInfo pi;
    pi.next = 0;
    pi.api_version = version;
    return &pi;
}

PluginHost phost = { (PluginInit)plugin_init, {0, 0} };

int test_plugin_system_pattern(void) {
    PluginInfo pi;
    pi.next = 0;
    pi.api_version = 200;
    return (int)pi.api_version;
}

int main(void) {
    int result = 0;
    result += test_original();
    result += test_nested_anon_struct();
    result += test_deep_chain();
    result += test_forward_typedef_chain();
    result += test_enum_typedef_in_struct();
    result += test_array_typedef_in_struct();
    result += test_fn_ptr_typedef_with_const_param();
    result += test_typedef_chain_with_return_struct();
    result += test_multiple_fn_ptr_typedefs();
    result += test_typedef_with_pointer_field();
    result += test_mutual_forward_typedef();
    result += test_triple_interdependent_typedef();
    result += test_self_referential_struct_via_fn_ptr();
    result += test_typedef_in_field_and_fnptr_param();
    result += test_deep_fn_ptr_typedef_chain();
    result += test_multiple_casts_same_tu();
    result += test_struct_containing_adapter_with_fnptr();
    result += test_event_system_pattern();
    result += test_plugin_system_pattern();
    return result;
}

#define TValuefields	Value value_; lu_byte tt_

typedef struct TValue {
  TValuefields;
} TValue;

#define settt_(o,t)	((o)->tt_=(t))

#define checkliveness(L,obj) \
	((void)L, ((void)0))

#define setobj(L,obj1,obj2) \
	{ TValue *io1=(obj1); const TValue *io2=(obj2); \
          io1->value_ = io2->value_; settt_(io1, io2->tt_); \
	  checkliveness(L,io1); }

void test(TValue *a, TValue *b) {
    setobj(0, a, b);
}

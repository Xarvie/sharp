
#line 3 "sharp-test/unit/defer/defer_nested.ce"
int log_buf[64];

#line 4 "sharp-test/unit/defer/defer_nested.ce"
int logn = 0;

#line 197 "sharp-test/unit/defer/defer_nested.ce"
typedef struct Resource Resource;
struct Resource {
    int handle;
    int closed;
};

#line 202 "sharp-test/unit/defer/defer_nested.ce"
Resource g_res;

#line 265 "sharp-test/unit/defer/defer_nested.ce"
int exec_counter;

#line 5 "sharp-test/unit/defer/defer_nested.ce"
void mark(int v) {
    if (logn < 64) {
        log_buf[logn] = v;
        logn = logn + 1;
    }
}

#line 8 "sharp-test/unit/defer/defer_nested.ce"
int test_lifo_sequence() {

#line 9 "sharp-test/unit/defer/defer_nested.ce"
    logn = 0;

#line 10 "sharp-test/unit/defer/defer_nested.ce"
{

#line 16 "sharp-test/unit/defer/defer_nested.ce"
        mark(0);
        
#line 15 "sharp-test/unit/defer/defer_nested.ce"
        mark(5);
        
#line 14 "sharp-test/unit/defer/defer_nested.ce"
        mark(4);
        
#line 13 "sharp-test/unit/defer/defer_nested.ce"
        mark(3);
        
#line 12 "sharp-test/unit/defer/defer_nested.ce"
        mark(2);
        
#line 11 "sharp-test/unit/defer/defer_nested.ce"
        mark(1);
    }

#line 18 "sharp-test/unit/defer/defer_nested.ce"
    if (logn != 6) 
        return 1;

#line 19 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[0] != 0) 
        return 2;

#line 20 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[1] != 5) 
        return 3;

#line 21 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[2] != 4) 
        return 4;

#line 22 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[3] != 3) 
        return 5;

#line 23 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[4] != 2) 
        return 6;

#line 24 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[5] != 1) 
        return 7;

#line 25 "sharp-test/unit/defer/defer_nested.ce"
    return 0;
}

#line 29 "sharp-test/unit/defer/defer_nested.ce"
int test_defer_if_else(int branch) {

#line 30 "sharp-test/unit/defer/defer_nested.ce"
    logn = 0;

#line 31 "sharp-test/unit/defer/defer_nested.ce"
    if (branch == 1) {

#line 33 "sharp-test/unit/defer/defer_nested.ce"
        mark(10);
        
#line 32 "sharp-test/unit/defer/defer_nested.ce"
        mark(100);
    }
    else 

#line 34 "sharp-test/unit/defer/defer_nested.ce"
        if (branch == 2) {

#line 36 "sharp-test/unit/defer/defer_nested.ce"
            mark(20);
            
#line 35 "sharp-test/unit/defer/defer_nested.ce"
            mark(200);
        }
        else {

#line 39 "sharp-test/unit/defer/defer_nested.ce"
            mark(30);
            
#line 38 "sharp-test/unit/defer/defer_nested.ce"
            mark(300);
        }

#line 41 "sharp-test/unit/defer/defer_nested.ce"
    mark(99);

#line 42 "sharp-test/unit/defer/defer_nested.ce"
    return 0;
}

#line 45 "sharp-test/unit/defer/defer_nested.ce"
int test_nested_if_defer() {

#line 46 "sharp-test/unit/defer/defer_nested.ce"
    logn = 0;

#line 47 "sharp-test/unit/defer/defer_nested.ce"
    int x = 5;

#line 48 "sharp-test/unit/defer/defer_nested.ce"
    if (x > 0) {

#line 50 "sharp-test/unit/defer/defer_nested.ce"
        if (x > 10) {

#line 52 "sharp-test/unit/defer/defer_nested.ce"
            mark(3);
            
#line 51 "sharp-test/unit/defer/defer_nested.ce"
            mark(2);
        }
        else {

#line 55 "sharp-test/unit/defer/defer_nested.ce"
            mark(5);
            
#line 54 "sharp-test/unit/defer/defer_nested.ce"
            mark(4);
        }

#line 57 "sharp-test/unit/defer/defer_nested.ce"
        mark(6);
        
#line 49 "sharp-test/unit/defer/defer_nested.ce"
        mark(1);
    }

#line 59 "sharp-test/unit/defer/defer_nested.ce"
    mark(7);

#line 60 "sharp-test/unit/defer/defer_nested.ce"
    if (logn != 5) 
        return 1;

#line 61 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[0] != 5) 
        return 2;

#line 62 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[1] != 4) 
        return 3;

#line 63 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[2] != 6) 
        return 4;

#line 64 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[3] != 1) 
        return 5;

#line 65 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[4] != 7) 
        return 6;

#line 66 "sharp-test/unit/defer/defer_nested.ce"
    return 0;
}

#line 70 "sharp-test/unit/defer/defer_nested.ce"
int test_defer_for_loop() {

#line 71 "sharp-test/unit/defer/defer_nested.ce"
    logn = 0;

#line 72 "sharp-test/unit/defer/defer_nested.ce"
    for (int i = 0; i < 3; i = i + 1) {

#line 74 "sharp-test/unit/defer/defer_nested.ce"
        mark(i);
        
#line 73 "sharp-test/unit/defer/defer_nested.ce"
        mark(i + 10);
    }

#line 76 "sharp-test/unit/defer/defer_nested.ce"
    mark(99);

#line 77 "sharp-test/unit/defer/defer_nested.ce"
    if (logn != 7) 
        return 1;

#line 78 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[0] != 0) 
        return 2;

#line 79 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[1] != 10) 
        return 3;

#line 80 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[2] != 1) 
        return 4;

#line 81 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[3] != 11) 
        return 5;

#line 82 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[4] != 2) 
        return 6;

#line 83 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[5] != 12) 
        return 7;

#line 84 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[6] != 99) 
        return 8;

#line 85 "sharp-test/unit/defer/defer_nested.ce"
    return 0;
}

#line 88 "sharp-test/unit/defer/defer_nested.ce"
int test_defer_while_loop() {

#line 89 "sharp-test/unit/defer/defer_nested.ce"
    logn = 0;

#line 90 "sharp-test/unit/defer/defer_nested.ce"
    int i = 0;

#line 91 "sharp-test/unit/defer/defer_nested.ce"
    while (i < 3) {

#line 93 "sharp-test/unit/defer/defer_nested.ce"
        mark(i);

#line 94 "sharp-test/unit/defer/defer_nested.ce"
        i = i + 1;
        
#line 92 "sharp-test/unit/defer/defer_nested.ce"
        mark(i + 20);
    }

#line 96 "sharp-test/unit/defer/defer_nested.ce"
    mark(99);

#line 97 "sharp-test/unit/defer/defer_nested.ce"
    if (logn != 7) 
        return 1;

#line 98 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[0] != 0) 
        return 2;

#line 99 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[1] != 21) 
        return 3;

#line 100 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[2] != 1) 
        return 4;

#line 101 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[3] != 22) 
        return 5;

#line 102 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[4] != 2) 
        return 6;

#line 103 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[5] != 23) 
        return 7;

#line 104 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[6] != 99) 
        return 8;

#line 105 "sharp-test/unit/defer/defer_nested.ce"
    return 0;
}

#line 108 "sharp-test/unit/defer/defer_nested.ce"
int test_defer_nested_loop() {

#line 109 "sharp-test/unit/defer/defer_nested.ce"
    logn = 0;

#line 110 "sharp-test/unit/defer/defer_nested.ce"
    for (int a = 0; a < 2; a = a + 1) {

#line 112 "sharp-test/unit/defer/defer_nested.ce"
        for (int b = 0; b < 2; b = b + 1) {

#line 114 "sharp-test/unit/defer/defer_nested.ce"
            mark(a * 10 + b);
            
#line 113 "sharp-test/unit/defer/defer_nested.ce"
            mark(b + 40);
        }
        
#line 111 "sharp-test/unit/defer/defer_nested.ce"
        mark(a + 30);
    }

#line 117 "sharp-test/unit/defer/defer_nested.ce"
    mark(99);

#line 118 "sharp-test/unit/defer/defer_nested.ce"
    if (logn != 11) 
        return 1;

#line 119 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[0] != 0) 
        return 2;

#line 120 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[1] != 40) 
        return 3;

#line 121 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[2] != 1) 
        return 4;

#line 122 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[3] != 41) 
        return 5;

#line 123 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[4] != 30) 
        return 6;

#line 124 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[5] != 10) 
        return 7;

#line 125 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[6] != 40) 
        return 8;

#line 126 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[7] != 11) 
        return 9;

#line 127 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[8] != 41) 
        return 10;

#line 128 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[9] != 31) 
        return 11;

#line 129 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[10] != 99) 
        return 12;

#line 130 "sharp-test/unit/defer/defer_nested.ce"
    return 0;
}

#line 134 "sharp-test/unit/defer/defer_nested.ce"
int test_deep_nested_blocks() {

#line 135 "sharp-test/unit/defer/defer_nested.ce"
    logn = 0;

#line 136 "sharp-test/unit/defer/defer_nested.ce"
    mark(1);

#line 137 "sharp-test/unit/defer/defer_nested.ce"
{

#line 139 "sharp-test/unit/defer/defer_nested.ce"
        mark(2);

#line 140 "sharp-test/unit/defer/defer_nested.ce"
{

#line 142 "sharp-test/unit/defer/defer_nested.ce"
            mark(3);

#line 143 "sharp-test/unit/defer/defer_nested.ce"
{

#line 145 "sharp-test/unit/defer/defer_nested.ce"
                mark(4);
                
#line 144 "sharp-test/unit/defer/defer_nested.ce"
                mark(30);
            }

#line 147 "sharp-test/unit/defer/defer_nested.ce"
            mark(5);
            
#line 141 "sharp-test/unit/defer/defer_nested.ce"
            mark(20);
        }

#line 149 "sharp-test/unit/defer/defer_nested.ce"
        mark(6);
        
#line 138 "sharp-test/unit/defer/defer_nested.ce"
        mark(10);
    }

#line 151 "sharp-test/unit/defer/defer_nested.ce"
    mark(7);

#line 152 "sharp-test/unit/defer/defer_nested.ce"
    if (logn != 10) 
        return 1;

#line 153 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[0] != 1) 
        return 2;

#line 154 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[1] != 2) 
        return 3;

#line 155 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[2] != 3) 
        return 4;

#line 156 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[3] != 4) 
        return 5;

#line 157 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[4] != 30) 
        return 6;

#line 158 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[5] != 5) 
        return 7;

#line 159 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[6] != 20) 
        return 8;

#line 160 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[7] != 6) 
        return 9;

#line 161 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[8] != 10) 
        return 10;

#line 162 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[9] != 7) 
        return 11;

#line 163 "sharp-test/unit/defer/defer_nested.ce"
    return 0;
}

#line 166 "sharp-test/unit/defer/defer_nested.ce"
int test_nested_block_with_early_exit() {

#line 167 "sharp-test/unit/defer/defer_nested.ce"
    logn = 0;

#line 168 "sharp-test/unit/defer/defer_nested.ce"
{

#line 170 "sharp-test/unit/defer/defer_nested.ce"
        mark(1);

#line 171 "sharp-test/unit/defer/defer_nested.ce"
{

#line 173 "sharp-test/unit/defer/defer_nested.ce"
            mark(2);

#line 174 "sharp-test/unit/defer/defer_nested.ce"
            if (1) {

#line 176 "sharp-test/unit/defer/defer_nested.ce"
                mark(3);
                
#line 175 "sharp-test/unit/defer/defer_nested.ce"
                mark(300);
            }

#line 178 "sharp-test/unit/defer/defer_nested.ce"
            mark(4);
            
#line 172 "sharp-test/unit/defer/defer_nested.ce"
            mark(200);
        }

#line 180 "sharp-test/unit/defer/defer_nested.ce"
        mark(5);
        
#line 169 "sharp-test/unit/defer/defer_nested.ce"
        mark(100);
    }

#line 182 "sharp-test/unit/defer/defer_nested.ce"
    mark(6);

#line 183 "sharp-test/unit/defer/defer_nested.ce"
    if (logn != 9) 
        return 1;

#line 184 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[0] != 1) 
        return 2;

#line 185 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[1] != 2) 
        return 3;

#line 186 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[2] != 3) 
        return 4;

#line 187 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[3] != 300) 
        return 5;

#line 188 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[4] != 4) 
        return 6;

#line 189 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[5] != 200) 
        return 7;

#line 190 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[6] != 5) 
        return 8;

#line 191 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[7] != 100) 
        return 9;

#line 192 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[8] != 6) 
        return 10;

#line 193 "sharp-test/unit/defer/defer_nested.ce"
    return 0;
}

#line 204 "sharp-test/unit/defer/defer_nested.ce"
void res_open(int h) {

#line 205 "sharp-test/unit/defer/defer_nested.ce"
    g_res.handle = h;

#line 206 "sharp-test/unit/defer/defer_nested.ce"
    g_res.closed = 0;
}

#line 209 "sharp-test/unit/defer/defer_nested.ce"
void res_close() {

#line 210 "sharp-test/unit/defer/defer_nested.ce"
    g_res.closed = 1;
}

#line 213 "sharp-test/unit/defer/defer_nested.ce"
int test_resource_pattern() {

#line 214 "sharp-test/unit/defer/defer_nested.ce"
    logn = 0;

#line 215 "sharp-test/unit/defer/defer_nested.ce"
    res_open(42);

#line 216 "sharp-test/unit/defer/defer_nested.ce"
{

#line 218 "sharp-test/unit/defer/defer_nested.ce"
        mark(g_res.handle);

#line 219 "sharp-test/unit/defer/defer_nested.ce"
        if (g_res.closed != 0) {
            int __sharp_ret = 1;
            
#line 217 "sharp-test/unit/defer/defer_nested.ce"
{
                res_close();
                mark(g_res.handle);
            }
            return __sharp_ret;
        }
        {
            res_close();
            mark(g_res.handle);
        }
    }

#line 221 "sharp-test/unit/defer/defer_nested.ce"
    if (g_res.closed != 1) 
        return 2;

#line 222 "sharp-test/unit/defer/defer_nested.ce"
    if (logn != 2) 
        return 3;

#line 223 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[0] != 42) 
        return 4;

#line 224 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[1] != 42) 
        return 5;

#line 225 "sharp-test/unit/defer/defer_nested.ce"
    return 0;
}

#line 228 "sharp-test/unit/defer/defer_nested.ce"
int test_multi_resource_pattern() {

#line 229 "sharp-test/unit/defer/defer_nested.ce"
    logn = 0;

#line 230 "sharp-test/unit/defer/defer_nested.ce"
    int handles[3];

#line 231 "sharp-test/unit/defer/defer_nested.ce"
    int states[3];

#line 232 "sharp-test/unit/defer/defer_nested.ce"
    handles[0] = 0;

#line 233 "sharp-test/unit/defer/defer_nested.ce"
    handles[1] = 0;

#line 234 "sharp-test/unit/defer/defer_nested.ce"
    handles[2] = 0;

#line 235 "sharp-test/unit/defer/defer_nested.ce"
    states[0] = 0;

#line 236 "sharp-test/unit/defer/defer_nested.ce"
    states[1] = 0;

#line 237 "sharp-test/unit/defer/defer_nested.ce"
    states[2] = 0;

#line 238 "sharp-test/unit/defer/defer_nested.ce"
{

#line 239 "sharp-test/unit/defer/defer_nested.ce"
        handles[0] = 1;

#line 241 "sharp-test/unit/defer/defer_nested.ce"
{

#line 242 "sharp-test/unit/defer/defer_nested.ce"
            handles[1] = 2;

#line 244 "sharp-test/unit/defer/defer_nested.ce"
{

#line 245 "sharp-test/unit/defer/defer_nested.ce"
                handles[2] = 3;

#line 247 "sharp-test/unit/defer/defer_nested.ce"
                mark(0);
                
#line 246 "sharp-test/unit/defer/defer_nested.ce"
{
                    states[2] = 1;
                    mark(30);
                }
            }

#line 249 "sharp-test/unit/defer/defer_nested.ce"
            if (states[2] != 1) {
                int __sharp_ret = 1;
                
#line 243 "sharp-test/unit/defer/defer_nested.ce"
{
                    states[1] = 1;
                    mark(20);
                }
                return __sharp_ret;
            }

#line 250 "sharp-test/unit/defer/defer_nested.ce"
            if (states[1] != 0) {
                int __sharp_ret = 2;
                
#line 243 "sharp-test/unit/defer/defer_nested.ce"
{
                    states[1] = 1;
                    mark(20);
                }
                return __sharp_ret;
            }
            {
                states[1] = 1;
                mark(20);
            }
        }

#line 252 "sharp-test/unit/defer/defer_nested.ce"
        if (states[1] != 1) {
            int __sharp_ret = 3;
            
#line 240 "sharp-test/unit/defer/defer_nested.ce"
{
                states[0] = 1;
                mark(10);
            }
            return __sharp_ret;
        }

#line 253 "sharp-test/unit/defer/defer_nested.ce"
        if (states[0] != 0) {
            int __sharp_ret = 4;
            
#line 240 "sharp-test/unit/defer/defer_nested.ce"
{
                states[0] = 1;
                mark(10);
            }
            return __sharp_ret;
        }
        {
            states[0] = 1;
            mark(10);
        }
    }

#line 255 "sharp-test/unit/defer/defer_nested.ce"
    if (states[0] != 1) 
        return 5;

#line 256 "sharp-test/unit/defer/defer_nested.ce"
    if (logn != 4) 
        return 6;

#line 257 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[0] != 0) 
        return 7;

#line 258 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[1] != 30) 
        return 8;

#line 259 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[2] != 20) 
        return 9;

#line 260 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[3] != 10) 
        return 10;

#line 261 "sharp-test/unit/defer/defer_nested.ce"
    return 0;
}

#line 267 "sharp-test/unit/defer/defer_nested.ce"
int next_counter() {

#line 268 "sharp-test/unit/defer/defer_nested.ce"
    exec_counter = exec_counter + 1;

#line 269 "sharp-test/unit/defer/defer_nested.ce"
    return exec_counter;
}

#line 272 "sharp-test/unit/defer/defer_nested.ce"
int test_counter_lifo() {

#line 273 "sharp-test/unit/defer/defer_nested.ce"
    int order[6];

#line 274 "sharp-test/unit/defer/defer_nested.ce"
    exec_counter = 0;

#line 275 "sharp-test/unit/defer/defer_nested.ce"
{

#line 281 "sharp-test/unit/defer/defer_nested.ce"
        order[5] = next_counter();
        
#line 280 "sharp-test/unit/defer/defer_nested.ce"
{
            order[4] = next_counter();
        }
        
#line 279 "sharp-test/unit/defer/defer_nested.ce"
{
            order[3] = next_counter();
        }
        
#line 278 "sharp-test/unit/defer/defer_nested.ce"
{
            order[2] = next_counter();
        }
        
#line 277 "sharp-test/unit/defer/defer_nested.ce"
{
            order[1] = next_counter();
        }
        
#line 276 "sharp-test/unit/defer/defer_nested.ce"
{
            order[0] = next_counter();
        }
    }

#line 283 "sharp-test/unit/defer/defer_nested.ce"
    if (order[5] != 1) 
        return 1;

#line 284 "sharp-test/unit/defer/defer_nested.ce"
    if (order[4] != 2) 
        return 2;

#line 285 "sharp-test/unit/defer/defer_nested.ce"
    if (order[3] != 3) 
        return 3;

#line 286 "sharp-test/unit/defer/defer_nested.ce"
    if (order[2] != 4) 
        return 4;

#line 287 "sharp-test/unit/defer/defer_nested.ce"
    if (order[1] != 5) 
        return 5;

#line 288 "sharp-test/unit/defer/defer_nested.ce"
    if (order[0] != 6) 
        return 6;

#line 289 "sharp-test/unit/defer/defer_nested.ce"
    return 0;
}

#line 292 "sharp-test/unit/defer/defer_nested.ce"
int test_counter_nested() {

#line 293 "sharp-test/unit/defer/defer_nested.ce"
    int order[8];

#line 294 "sharp-test/unit/defer/defer_nested.ce"
    exec_counter = 0;

#line 295 "sharp-test/unit/defer/defer_nested.ce"
{

#line 297 "sharp-test/unit/defer/defer_nested.ce"
        order[1] = next_counter();

#line 298 "sharp-test/unit/defer/defer_nested.ce"
{

#line 300 "sharp-test/unit/defer/defer_nested.ce"
            order[3] = next_counter();

#line 301 "sharp-test/unit/defer/defer_nested.ce"
{

#line 303 "sharp-test/unit/defer/defer_nested.ce"
                order[5] = next_counter();
                
#line 302 "sharp-test/unit/defer/defer_nested.ce"
{
                    order[4] = next_counter();
                }
            }

#line 305 "sharp-test/unit/defer/defer_nested.ce"
            order[6] = next_counter();
            
#line 299 "sharp-test/unit/defer/defer_nested.ce"
{
                order[2] = next_counter();
            }
        }

#line 307 "sharp-test/unit/defer/defer_nested.ce"
        order[7] = next_counter();
        
#line 296 "sharp-test/unit/defer/defer_nested.ce"
{
            order[0] = next_counter();
        }
    }

#line 309 "sharp-test/unit/defer/defer_nested.ce"
    if (order[1] != 1) 
        return 1;

#line 310 "sharp-test/unit/defer/defer_nested.ce"
    if (order[3] != 2) 
        return 2;

#line 311 "sharp-test/unit/defer/defer_nested.ce"
    if (order[5] != 3) 
        return 3;

#line 312 "sharp-test/unit/defer/defer_nested.ce"
    if (order[4] != 4) 
        return 4;

#line 313 "sharp-test/unit/defer/defer_nested.ce"
    if (order[6] != 5) 
        return 5;

#line 314 "sharp-test/unit/defer/defer_nested.ce"
    if (order[2] != 6) 
        return 6;

#line 315 "sharp-test/unit/defer/defer_nested.ce"
    if (order[7] != 7) 
        return 7;

#line 316 "sharp-test/unit/defer/defer_nested.ce"
    if (order[0] != 8) 
        return 8;

#line 317 "sharp-test/unit/defer/defer_nested.ce"
    return 0;
}

#line 320 "sharp-test/unit/defer/defer_nested.ce"
int test_counter_loop() {

#line 321 "sharp-test/unit/defer/defer_nested.ce"
    int order[7];

#line 322 "sharp-test/unit/defer/defer_nested.ce"
    exec_counter = 0;

#line 323 "sharp-test/unit/defer/defer_nested.ce"
    for (int i = 0; i < 3; i = i + 1) {

#line 325 "sharp-test/unit/defer/defer_nested.ce"
        order[i * 2 + 1] = next_counter();
        
#line 324 "sharp-test/unit/defer/defer_nested.ce"
{
            order[i * 2] = next_counter();
        }
    }

#line 327 "sharp-test/unit/defer/defer_nested.ce"
    if (order[1] != 1) 
        return 1;

#line 328 "sharp-test/unit/defer/defer_nested.ce"
    if (order[0] != 2) 
        return 2;

#line 329 "sharp-test/unit/defer/defer_nested.ce"
    if (order[3] != 3) 
        return 3;

#line 330 "sharp-test/unit/defer/defer_nested.ce"
    if (order[2] != 4) 
        return 4;

#line 331 "sharp-test/unit/defer/defer_nested.ce"
    if (order[5] != 5) 
        return 5;

#line 332 "sharp-test/unit/defer/defer_nested.ce"
    if (order[4] != 6) 
        return 6;

#line 333 "sharp-test/unit/defer/defer_nested.ce"
    return 0;
}

#line 336 "sharp-test/unit/defer/defer_nested.ce"
int main(void) {

#line 337 "sharp-test/unit/defer/defer_nested.ce"
    int r;

#line 340 "sharp-test/unit/defer/defer_nested.ce"
    r = test_lifo_sequence();

#line 341 "sharp-test/unit/defer/defer_nested.ce"
    if (r != 0) 
        return r;

#line 344 "sharp-test/unit/defer/defer_nested.ce"
    logn = 0;

#line 345 "sharp-test/unit/defer/defer_nested.ce"
    test_defer_if_else(1);

#line 346 "sharp-test/unit/defer/defer_nested.ce"
    if (logn != 3) 
        return 10;

#line 347 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[0] != 10 || log_buf[1] != 100 || log_buf[2] != 99) 
        return 11;

#line 349 "sharp-test/unit/defer/defer_nested.ce"
    logn = 0;

#line 350 "sharp-test/unit/defer/defer_nested.ce"
    test_defer_if_else(2);

#line 351 "sharp-test/unit/defer/defer_nested.ce"
    if (logn != 3) 
        return 12;

#line 352 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[0] != 20 || log_buf[1] != 200 || log_buf[2] != 99) 
        return 13;

#line 354 "sharp-test/unit/defer/defer_nested.ce"
    logn = 0;

#line 355 "sharp-test/unit/defer/defer_nested.ce"
    test_defer_if_else(3);

#line 356 "sharp-test/unit/defer/defer_nested.ce"
    if (logn != 3) 
        return 14;

#line 357 "sharp-test/unit/defer/defer_nested.ce"
    if (log_buf[0] != 30 || log_buf[1] != 300 || log_buf[2] != 99) 
        return 15;

#line 359 "sharp-test/unit/defer/defer_nested.ce"
    r = test_nested_if_defer();

#line 360 "sharp-test/unit/defer/defer_nested.ce"
    if (r != 0) 
        return 100 + r;

#line 363 "sharp-test/unit/defer/defer_nested.ce"
    r = test_defer_for_loop();

#line 364 "sharp-test/unit/defer/defer_nested.ce"
    if (r != 0) 
        return 200 + r;

#line 366 "sharp-test/unit/defer/defer_nested.ce"
    r = test_defer_while_loop();

#line 367 "sharp-test/unit/defer/defer_nested.ce"
    if (r != 0) 
        return 210 + r;

#line 369 "sharp-test/unit/defer/defer_nested.ce"
    r = test_defer_nested_loop();

#line 370 "sharp-test/unit/defer/defer_nested.ce"
    if (r != 0) 
        return 220 + r;

#line 373 "sharp-test/unit/defer/defer_nested.ce"
    r = test_deep_nested_blocks();

#line 374 "sharp-test/unit/defer/defer_nested.ce"
    if (r != 0) 
        return 300 + r;

#line 376 "sharp-test/unit/defer/defer_nested.ce"
    r = test_nested_block_with_early_exit();

#line 377 "sharp-test/unit/defer/defer_nested.ce"
    if (r != 0) 
        return 310 + r;

#line 380 "sharp-test/unit/defer/defer_nested.ce"
    r = test_resource_pattern();

#line 381 "sharp-test/unit/defer/defer_nested.ce"
    if (r != 0) 
        return 400 + r;

#line 383 "sharp-test/unit/defer/defer_nested.ce"
    r = test_multi_resource_pattern();

#line 384 "sharp-test/unit/defer/defer_nested.ce"
    if (r != 0) 
        return 410 + r;

#line 387 "sharp-test/unit/defer/defer_nested.ce"
    r = test_counter_lifo();

#line 388 "sharp-test/unit/defer/defer_nested.ce"
    if (r != 0) 
        return 500 + r;

#line 390 "sharp-test/unit/defer/defer_nested.ce"
    r = test_counter_nested();

#line 391 "sharp-test/unit/defer/defer_nested.ce"
    if (r != 0) 
        return 510 + r;

#line 393 "sharp-test/unit/defer/defer_nested.ce"
    r = test_counter_loop();

#line 394 "sharp-test/unit/defer/defer_nested.ce"
    if (r != 0) 
        return 520 + r;

#line 396 "sharp-test/unit/defer/defer_nested.ce"
    return 0;
}

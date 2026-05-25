/*
 * smoke.c — Quick smoke test for timer, wg_post, and broadcast.
 *
 * Behavior:
 *   - A "tick" counter increments via a 100ms repeating timer.
 *   - A background thread calls wg_post every 200ms to update a second label.
 *   - Both updates broadcast to all connected clients.
 */

#define _POSIX_C_SOURCE 200809L

#include "webgui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

static int tick_count = 0;
static int post_count = 0;
static volatile int worker_running = 1;

static void on_tick(wg_app_t *app, void *ud) {
    (void)ud;
    tick_count++;
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", tick_count);
    wg_set_text(wg_find(app, "tick"), buf);
}

static void on_post(wg_app_t *app, void *ud) {
    (void)ud;
    post_count++;
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", post_count);
    wg_set_text(wg_find(app, "post"), buf);
}

static void *worker(void *arg) {
    wg_app_t *app = (wg_app_t *)arg;
    struct timespec ts = { .tv_sec = 0, .tv_nsec = 200 * 1000 * 1000 };
    while (worker_running) {
        nanosleep(&ts, NULL);
        if (worker_running) wg_post(app, on_post, NULL);
    }
    return NULL;
}

static void on_stop(const wg_event_t *ev) {
    worker_running = 0;
    wg_stop(ev->app);
}

int main(void) {
    wg_node_t *root = wg_container("root");
    wg_append_n(root, (wg_node_t *[]){
        wg_heading("h", 1, "Smoke"),
        wg_text("tick_label", "ticks: "),
        wg_text("tick", "0"),
        wg_separator("s"),
        wg_text("post_label", "posts: "),
        wg_text("post", "0"),
        wg_separator("s2"),
        wg_button("stop", "Stop"),
    }, 8);

    wg_app_t *app = wg_app_create(root);
    if (!app) { fprintf(stderr, "create failed\n"); return 1; }
    wg_set_title(app, "Smoke");
    if (wg_bind_tcp(app, NULL, 9003) != WG_OK) {
        fprintf(stderr, "bind failed\n");
        wg_app_destroy(app);
        return 1;
    }

    wg_on(app, "stop", "click", on_stop, NULL);

    int tid = wg_set_timer(app, 100, 1, on_tick, NULL);
    if (tid < 0) { fprintf(stderr, "timer failed: %s\n", wg_strerror(tid)); }

    pthread_t th;
    pthread_create(&th, NULL, worker, app);

    printf("Smoke test: http://localhost:9003  (will auto-stop after running tests)\n");
    fflush(stdout);

    wg_run(app);

    worker_running = 0;
    pthread_join(th, NULL);

    printf("Ticks fired: %d  Posts fired: %d\n", tick_count, post_count);
    wg_app_destroy(app);
    return 0;
}

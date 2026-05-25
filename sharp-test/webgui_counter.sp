#include <stdio.h>
#include "webgui.sph"

static int counter = 0;

static void on_click(const wg_event_t *ev) {
    counter++;
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", counter);
    wg_node_t *n = wg_find(ev->app, "count");
    if (n) wg_set_text(n, buf);
}

static void on_reset(const wg_event_t *ev) {
    counter = 0;
    wg_node_t *n = wg_find(ev->app, "count");
    if (n) wg_set_text(n, "0");
}

int main(void) {
    wg_node_t *root = wg_container("root");
    wg_node_t *title = wg_heading("title", 1, "Counter");
    wg_node_t *count = wg_text("count", "0");
    wg_node_t *btn   = wg_button("inc", "Increment");
    wg_node_t *reset = wg_button("reset", "Reset");

    wg_append(root, title);
    wg_append(root, count);
    wg_append(root, btn);
    wg_append(root, reset);

    wg_app_t *app = wg_app_create(root);
    wg_bind_tcp(app, NULL, 9002);
    wg_set_title(app, "Counter Demo");

    wg_on(app, "inc", "click", on_click, NULL);
    wg_on(app, "reset", "click", on_reset, NULL);

    return wg_run(app);
}

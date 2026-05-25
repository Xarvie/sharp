/*
 * counter.c — A minimal WebGUI demo.
 *
 * Build:    make counter           (from project root)
 * Run:      ./counter
 * Open:     http://localhost:9001  in a browser
 */

#include "webgui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int counter = 0;

/* Refresh the counter display and the disabled state of the "-" button. */
static void refresh(wg_app_t *app) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", counter);
    wg_set_text(wg_find(app, "display"), buf);
    wg_set_disabled(wg_find(app, "dec"), counter <= 0);
}

static void on_inc(const wg_event_t *ev) {
    counter++;
    refresh(ev->app);
}

static void on_dec(const wg_event_t *ev) {
    if (counter > 0) counter--;
    refresh(ev->app);
}

static void on_reset(const wg_event_t *ev) {
    counter = 0;
    refresh(ev->app);
}

static void on_set(const wg_event_t *ev) {
    const char *v = wg_form_get(ev, "field");
    if (v && *v) {
        char *end = NULL;
        long n = strtol(v, &end, 10);
        if (end && *end == '\0' && n >= 0) counter = (int)n;
    }
    /* Clear the input field for next entry. */
    wg_set_value(wg_find(ev->app, "field"), "");
    refresh(ev->app);
}

int main(void) {
    /* ---------- Build the tree ---------- */

    wg_node_t *root = wg_container("root");
    if (!root) {
        fprintf(stderr, "wg_container: %s\n", wg_strerror(wg_errno()));
        return 1;
    }

    wg_node_t *controls = wg_container("controls");
    wg_set_attr(controls, "style", "display:flex;gap:8px;align-items:center;");

    wg_node_t *dec     = wg_button("dec", "-");
    wg_node_t *display = wg_text  ("display", "0");
    wg_node_t *inc     = wg_button("inc", "+");
    wg_node_t *reset   = wg_button("reset", "Reset");

    wg_set_attr(display, "style",
                "font-size:24px;min-width:60px;text-align:center;");

    wg_append_n(controls, (wg_node_t *[]){ dec, display, inc, reset }, 4);

    /* Form: Set the counter to a specific value. */
    wg_node_t *form    = wg_form  ("form");
    wg_node_t *field   = wg_input ("field");
    wg_node_t *submit  = wg_button("submit", "Set");

    wg_set_attr(field, "type", "number");
    wg_set_attr(field, "min", "0");
    wg_set_attr(field, "placeholder", "value");
    wg_set_attr(submit, "role", "submit");
    wg_set_attr(form, "style", "display:flex;gap:8px;margin-top:8px;");

    wg_append_n(form, (wg_node_t *[]){ field, submit }, 2);

    /* Compose the root. */
    wg_append_n(root, (wg_node_t *[]){
        wg_heading("title", 1, "WebGUI Counter"),
        wg_text   ("hint",  "Click + and - to change the counter."),
        wg_separator("sep1"),
        controls,
        form,
    }, 5);

    /* ---------- Create the app ---------- */

    wg_app_t *app = wg_app_create(root);
    if (!app) {
        fprintf(stderr, "wg_app_create: %s\n", wg_strerror(wg_errno()));
        return 1;
    }

    wg_set_title(app, "Counter");

    int rc = wg_bind_tcp(app, NULL, 9001);
    if (rc != WG_OK) {
        fprintf(stderr, "wg_bind_tcp: %s\n", wg_strerror(rc));
        wg_app_destroy(app);
        return 1;
    }

    /* ---------- Wire events ---------- */

    wg_on(app, "inc",   "click",  on_inc,   NULL);
    wg_on(app, "dec",   "click",  on_dec,   NULL);
    wg_on(app, "reset", "click",  on_reset, NULL);
    wg_on(app, "form",  "submit", on_set,   NULL);

    refresh(app);   /* sets initial disabled state for the - button */

    printf("Open http://localhost:9001 in a browser. Ctrl-C to quit.\n");
    fflush(stdout);

    /* ---------- Run ---------- */

    rc = wg_run(app);
    wg_app_destroy(app);
    return rc < 0 ? 1 : 0;
}

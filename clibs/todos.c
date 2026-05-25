/*
 * todos.c — Dynamic add/remove demo.
 *
 * Demonstrates: wg_append/wg_remove on a mounted tree, wg_post via a
 * worker thread, wg_set_timer.
 *
 * Build:    make todos
 * Run:      ./todos
 * Open:     http://localhost:9002
 */

#include "webgui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int next_todo_id = 1;
static int todo_count = 0;

static char *str_dup_(const char *s) {
    size_t n = strlen(s);
    char *d = malloc(n + 1);
    if (d) memcpy(d, s, n + 1);
    return d;
}

static void update_status(wg_app_t *app) {
    char buf[64];
    snprintf(buf, sizeof(buf), "%d item%s", todo_count, todo_count == 1 ? "" : "s");
    wg_set_text(wg_find(app, "status"), buf);
}

static void on_remove(const wg_event_t *ev) {
    /* The button id is "rm_<n>"; the row id is "row_<n>". */
    if (strncmp(ev->id, "rm_", 3) != 0) return;
    char row_id[64];
    snprintf(row_id, sizeof(row_id), "row_%s", ev->id + 3);

    wg_node_t *row = wg_find(ev->app, row_id);
    if (!row) return;
    wg_node_t *list = wg_find(ev->app, "list");
    if (wg_remove(list, row) == WG_OK) {
        wg_free(row);
        todo_count--;
        update_status(ev->app);
        /* Unregister the click handler so it doesn't linger. */
        wg_off(ev->app, ev->id, "click");
    }
}

static void on_add(const wg_event_t *ev) {
    const char *text = wg_form_get(ev, "input");
    if (!text || !*text) return;

    int id = next_todo_id++;

    char row_id[32], cb_id[32], lbl_id[32], rm_id[32];
    snprintf(row_id, sizeof(row_id), "row_%d", id);
    snprintf(cb_id,  sizeof(cb_id),  "cb_%d",  id);
    snprintf(lbl_id, sizeof(lbl_id), "lbl_%d", id);
    snprintf(rm_id,  sizeof(rm_id),  "rm_%d",  id);

    wg_node_t *row = wg_container(row_id);
    wg_set_attr(row, "style",
                "display:flex;gap:8px;align-items:center;padding:4px 0;");

    wg_node_t *cb  = wg_checkbox(cb_id, "");
    wg_node_t *lbl = wg_text(lbl_id, text);
    wg_node_t *rm  = wg_button(rm_id, "x");
    wg_set_attr(lbl, "style", "flex:1;");
    wg_set_attr(rm,  "style", "padding:2px 8px;");

    wg_append_n(row, (wg_node_t *[]){ cb, lbl, rm }, 3);

    wg_node_t *list = wg_find(ev->app, "list");
    if (wg_append(list, row) != WG_OK) {
        wg_free(row);
        return;
    }
    wg_on(ev->app, rm_id, "click", on_remove, NULL);

    todo_count++;
    update_status(ev->app);

    /* Clear the input. */
    wg_set_value(wg_find(ev->app, "input"), "");
}

int main(void) {
    wg_node_t *root = wg_container("root");

    wg_append_n(root, (wg_node_t *[]){
        wg_heading("title", 1, "Todos"),
        wg_text("status", "0 items"),
        wg_separator("sep1"),
    }, 3);

    /* Add-form. */
    wg_node_t *form = wg_form("form");
    wg_node_t *in   = wg_input("input");
    wg_node_t *btn  = wg_button("add", "Add");
    wg_set_attr(in,  "placeholder", "What needs doing?");
    wg_set_attr(in,  "style", "flex:1;");
    wg_set_attr(btn, "role", "submit");
    wg_set_attr(form, "style", "display:flex;gap:8px;margin-bottom:12px;");
    wg_append_n(form, (wg_node_t *[]){ in, btn }, 2);
    wg_append(root, form);

    /* The list itself. */
    wg_append(root, wg_container("list"));

    /* App. */
    wg_app_t *app = wg_app_create(root);
    if (!app) {
        fprintf(stderr, "wg_app_create: %s\n", wg_strerror(wg_errno()));
        return 1;
    }
    wg_set_title(app, "Todos");

    int rc = wg_bind_tcp(app, NULL, 9002);
    if (rc != WG_OK) {
        fprintf(stderr, "wg_bind_tcp: %s\n", wg_strerror(rc));
        wg_app_destroy(app);
        return 1;
    }

    wg_on(app, "form", "submit", on_add, NULL);

    printf("Open http://localhost:9002 in a browser. Ctrl-C to quit.\n");
    fflush(stdout);

    rc = wg_run(app);
    wg_app_destroy(app);
    (void)str_dup_;
    return rc < 0 ? 1 : 0;
}

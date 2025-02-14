#include <cstdio>
#include <FreeRTOS.h>
#include <task.h>
#include <mongoose.h>
#include <tusb.h>
#include "eth.h"
#include "usb.h"
#include "usr.h"

static void fn(struct mg_connection* c, int ev, void* ev_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = reinterpret_cast<struct mg_http_message*>(ev_data);
        if (mg_match(hm->uri, mg_str("/"), NULL) || mg_match(hm->uri, mg_str("/index.html"), NULL)) {
            mg_http_reply(c, 200, "", "hello there! %d\r\n", usr_btn());
        } else {
            mg_http_reply(c, 404, "", "");
        }
        // browsers keep socket open, preventing new clients
        // TODO close connection after done
    }
}

void app_main(void*) {
    eth_init();
    usb_init();
    printf("booted!\r\n");

    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    mg_http_listen(&mgr, "http://0.0.0.0:80", fn, NULL);

    bool r = 1, g = 0, b = 0;
    while (1) {
        mg_mgr_poll(&mgr, 500);
        usr_rgb(r, g, b);
        bool t = r;
        r = g; g = b; b = t;
    }
    mg_mgr_free(&mgr);
    vTaskDelete(NULL);
}

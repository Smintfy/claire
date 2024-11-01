#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/ssl.h>
#include <libwebsockets.h>
#include "include/nobu.h"

#include "lib/dotenv.h"

#define DISCORD_GATEWAY_PROTOCOL "wss"
#define DISCORD_GATEWAY_URL "gateway.discord.gg"
#define DISCORD_GATEWAY_PORT 443
#define DISCORD_GATEWAY_PATH "/?v=10&encoding=json"

static struct lws_context *context = NULL;
static int interrupt = 0;
// char *bot_token;

static void sigint_handler(int sig) { interrupt = 1;}

static int
callback_discord_gateway(struct lws *wsi, enum lws_callback_reasons reason,
                        void *user, void *in, size_t len);

static struct lws_protocols protocols[] = {
    { "discord-gateway", callback_discord_gateway, 0, 4096 },
    { NULL, NULL, 0, 0 }  /* terminator */
};

int main(void)
{
    // env_load(".", false);
    // bot_token = getenv("BOT_TOKEN");

    signal(SIGINT, sigint_handler);
    signal(SIGTERM, sigint_handler);

    struct lws_context_creation_info context_info;
    memset(&context_info, 0, sizeof(context_info));
    context_info.port = CONTEXT_PORT_NO_LISTEN;
    context_info.protocols = protocols;
    context_info.gid = -1;
    context_info.uid = -1;
    context_info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;

    struct lws_context *context = lws_create_context(&context_info);
    if (!context) {
        LOG(NOBU_ERROR, "failed to create WebSocket context.\n", NULL);
        return 1;
    }

    struct lws_client_connect_info connect_info;
    memset(&connect_info, 0, sizeof(connect_info));
    connect_info.context = context;
    connect_info.address = DISCORD_GATEWAY_URL;
    connect_info.port = DISCORD_GATEWAY_PORT;
    connect_info.path = DISCORD_GATEWAY_PATH;
    connect_info.host = DISCORD_GATEWAY_URL;
    connect_info.origin = DISCORD_GATEWAY_URL;
    connect_info.ssl_connection = LCCSCF_USE_SSL;
    connect_info.protocol = protocols[0].name;

    struct lws *wsi = lws_client_connect_via_info(&connect_info);
    if (!wsi) {
        LOG(NOBU_ERROR, "WebSocket connection failed.\n", NULL);
        lws_context_destroy(context);
        return 1;
    }

    while (!interrupt) lws_service(context, 500); /* timeout in ms */

    lws_context_destroy(context);
    lwsl_notice("Disconnected and cleaned up\n");

    return 0;
}

static int callback_discord_gateway(struct lws *wsi, enum lws_callback_reasons reason,
                                    void *user, void *in, size_t len)
{
    switch (reason) {
        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            lwsl_err("Connection Error\n");
            interrupt = 1;
            break;
        case LWS_CALLBACK_CLIENT_CLOSED:
            lwsl_notice("Connection Closed\n");
            interrupt = 1;
            break;
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            lwsl_notice("Connection Established\n");
            break;
        default:
            break;
    }

    return 0;
}

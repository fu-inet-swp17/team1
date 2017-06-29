#include <stdio.h>
#include <coap.h>
#include <string.h>
#include <led.h>

#define MAX_RESPONSE_LEN 500
static uint8_t response[MAX_RESPONSE_LEN] = { 0 };

extern char * get_name_of_player(void);
extern void start_game(void);

static int handle_toggle_led(coap_rw_buffer_t *scratch,
        const coap_packet_t *inpkt, coap_packet_t *outpkt,
        uint8_t id_hi, uint8_t id_lo);
static int handle_request_name(coap_rw_buffer_t *scratch,
        const coap_packet_t *inpkt, coap_packet_t *outpkt,
        uint8_t id_hi, uint8_t id_lo);
static int handle_start_game(coap_rw_buffer_t *scratch,
        const coap_packet_t *inpkt, coap_packet_t *outpkt,
        uint8_t id_hi, uint8_t id_lo);
/*static int handle_request_outcome(coap_rw_buffer_t *scratch,
        const coap_packet_t *inpkt, coap_packet_t *outpkt,
        uint8_t id_hi, uint8_t id_lo);*/

static const coap_endpoint_path_t path_toggle_led =
        { 2, { "toggle", "led" } };
static const coap_endpoint_path_t path_request_name =
        { 2, { "request", "name" } };
static const coap_endpoint_path_t path_start_game =
        { 2, { "start", "game" } };
/*static const coap_endpoint_path_t path_request_outcome =
        { 2, { "request", "outcome" } };*/

const coap_endpoint_t endpoints[] =
{
    { COAP_METHOD_GET,  handle_toggle_led,
        &path_toggle_led,    "ct=0"  },
    { COAP_METHOD_GET,  handle_request_name,
        &path_request_name,    "ct=0"  },
    { COAP_METHOD_GET,  handle_start_game,
        &path_start_game,    "ct=0"  },
    /*{ COAP_METHOD_GET,  handle_request_outcome,
        &path_toggle_led,    "ct=0"  },*/

    /* marks the end of the endpoints array: */
    { (coap_method_t)0, NULL, NULL, NULL }
};

static int handle_toggle_led(coap_rw_buffer_t *scratch,
        const coap_packet_t *inpkt, coap_packet_t *outpkt,
        uint8_t id_hi, uint8_t id_lo)
{
    const char *str = "LED was toggled";
    int len = strlen("LED was toggled");

    LED0_TOGGLE;

    memcpy(response, str, len);

    return coap_make_response(scratch, outpkt, (const uint8_t *)response, len,
                              id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT,
                              COAP_CONTENTTYPE_TEXT_PLAIN);
}

static int handle_request_name(coap_rw_buffer_t *scratch,
        const coap_packet_t *inpkt, coap_packet_t *outpkt,
        uint8_t id_hi, uint8_t id_lo)
{
    const char *player_name = get_name_of_player();
    const char *str = player_name;
    int len = strlen(player_name);

    memcpy(response, str, len);

    return coap_make_response(scratch, outpkt, (const uint8_t *)response, len,
                              id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT,
                              COAP_CONTENTTYPE_TEXT_PLAIN);
}

static int handle_start_game(coap_rw_buffer_t *scratch,
        const coap_packet_t *inpkt, coap_packet_t *outpkt,
        uint8_t id_hi, uint8_t id_lo)
{
    start_game();

    const char *str = "Game was started";
    int len = strlen("Game was started");

    memcpy(response, str, len);

    return coap_make_response(scratch, outpkt, (const uint8_t *)response, len,
                              id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT,
                              COAP_CONTENTTYPE_TEXT_PLAIN);
}

/*static int handle_request_outcome(coap_rw_buffer_t *scratch,
        const coap_packet_t *inpkt, coap_packet_t *outpkt,
        uint8_t id_hi, uint8_t id_lo)
{
    outcome = get_outcome();

    response = outcome;

    return coap_make_response(scratch, outpkt, (const uint8_t *)response, len,
                              id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT,
                              COAP_CONTENTTYPE_TEXT_PLAIN);
}*/
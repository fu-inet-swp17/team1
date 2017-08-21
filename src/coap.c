#include <stdio.h>
#include <coap.h>
#include <string.h>
#include <led.h>

#define MAX_RESPONSE_LEN 500
static uint8_t response[MAX_RESPONSE_LEN] = { 0 };

extern char * is_initalized(void);
extern char * get_name_of_player(void);
extern char * start_game(void);
extern char * get_result(void);

static int handle_toggle_led(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt,
        uint8_t id_hi, uint8_t id_lo);
static int handle_init_status(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt,
        uint8_t id_hi, uint8_t id_lo);
static int handle_request_name(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt,
        uint8_t id_hi, uint8_t id_lo);
static int handle_start_game(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt,
        uint8_t id_hi, uint8_t id_lo);
static int handle_request_result(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt,
        uint8_t id_hi, uint8_t id_lo);

static const coap_endpoint_path_t path_toggle_led =
        { 2, { "toggle", "led" } };
static const coap_endpoint_path_t path_init_status =
        { 2, { "init", "status" } };
static const coap_endpoint_path_t path_request_name =
        { 2, { "request", "name" } };
static const coap_endpoint_path_t path_start_game =
        { 2, { "start", "game" } };
static const coap_endpoint_path_t path_request_result =
        { 2, { "request", "result" } };

const coap_endpoint_t endpoints[] =
{
    { COAP_METHOD_GET,  handle_toggle_led,
        &path_toggle_led,    "ct=0"  },
    { COAP_METHOD_GET,  handle_init_status,
        &path_init_status,    "ct=0"  },
    { COAP_METHOD_GET,  handle_request_name,
        &path_request_name,    "ct=0"  },
    { COAP_METHOD_GET,  handle_start_game,
        &path_start_game,    "ct=0"  },
    { COAP_METHOD_GET,  handle_request_result,
        &path_request_result,    "ct=0"  },
    /* marks the end of the endpoints array: */
    { (coap_method_t)0, NULL, NULL, NULL }
};

static int handle_toggle_led(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt,
        uint8_t id_hi, uint8_t id_lo) {
    const char *str = "LED was toggled";
    int len = strlen("LED was toggled");

    LED0_TOGGLE;

    memcpy(response, str, len);

    return coap_make_response(scratch, outpkt, (const uint8_t *)response, len, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT,
                              COAP_CONTENTTYPE_TEXT_PLAIN);
}

static int handle_init_status(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt,
        uint8_t id_hi, uint8_t id_lo) {
    const char *str = is_initalized();
    int len = strlen(str);

    memcpy(response, str, len);

    return coap_make_response(scratch, outpkt, (const uint8_t *)response, len, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT,
                              COAP_CONTENTTYPE_TEXT_PLAIN);
}

static int handle_request_name(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt,
        uint8_t id_hi, uint8_t id_lo) {
    const char *str = get_name_of_player();
    int len = strlen(str);

    memcpy(response, str, len);

    return coap_make_response(scratch, outpkt, (const uint8_t *)response, len, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT,
                              COAP_CONTENTTYPE_TEXT_PLAIN);
}

static int handle_start_game(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt,
        uint8_t id_hi, uint8_t id_lo) {

    const char *str = start_game();
    int len = strlen(str);

    memcpy(response, str, len);

    return coap_make_response(scratch, outpkt, (const uint8_t *)response, len, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT,
                              COAP_CONTENTTYPE_TEXT_PLAIN);
}

static int handle_request_result(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt,
        uint8_t id_hi, uint8_t id_lo) {

    const char* str = get_result();
    int len = strlen(str);

    memcpy(response, str, len);

    return coap_make_response(scratch, outpkt, (const uint8_t *)response, len, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT,
                              COAP_CONTENTTYPE_TEXT_PLAIN);
}

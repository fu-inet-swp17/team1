#include <stdio.h>
#include <coap.h>
#include <string.h>
#include <led.h>

#define MAX_RESPONSE_LEN 500
static uint8_t response[MAX_RESPONSE_LEN] = { 0 };

extern void get_name_of_player(char *value);
extern char * start_game(void);
extern void get_result(char *value);
extern void set_winner(void);
extern void set_looser(void);

static int handle_request_name(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt,
        uint8_t id_hi, uint8_t id_lo);
static int handle_start_game(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt,
        uint8_t id_hi, uint8_t id_lo);
static int handle_request_result(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt,
        uint8_t id_hi, uint8_t id_lo);
static int handle_set_winner(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt,
        uint8_t id_hi, uint8_t id_lo);
static int handle_set_looser(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt,
        uint8_t id_hi, uint8_t id_lo);

static const coap_endpoint_path_t path_request_name =
        { 2, { "request", "name" } };
static const coap_endpoint_path_t path_start_game =
        { 2, { "start", "game" } };
static const coap_endpoint_path_t path_request_result =
        { 2, { "request", "result" } };
static const coap_endpoint_path_t path_set_winner =
        { 2, { "set", "winner" } };
static const coap_endpoint_path_t path_set_looser =
        { 2, { "set", "looser" } };

const coap_endpoint_t endpoints[] =
{
    { COAP_METHOD_GET,  handle_request_name,
        &path_request_name,    "ct=0"  },
    { COAP_METHOD_GET,  handle_start_game,
        &path_start_game,    "ct=0"  },
    { COAP_METHOD_GET,  handle_request_result,
        &path_request_result,    "ct=0"  },
    { COAP_METHOD_GET,  handle_set_winner,
        &path_set_winner,    "ct=0"  },
    { COAP_METHOD_GET,  handle_set_looser,
        &path_set_looser,    "ct=0"  },
    /* marks the end of the endpoints array: */
    { (coap_method_t)0, NULL, NULL, NULL }
};

static int handle_request_name(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt,
        uint8_t id_hi, uint8_t id_lo) {
    char str[40] = "unknown";
    get_name_of_player(str);
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

    char str[40] = "result";
    get_result(str);
    int len = strlen(str);

    memcpy(response, str, len);

    return coap_make_response(scratch, outpkt, (const uint8_t *)response, len, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT,
                              COAP_CONTENTTYPE_TEXT_PLAIN);
}

static int handle_set_winner(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt,
        uint8_t id_hi, uint8_t id_lo) {
    const char *str = "winner was displayed";
    int len = strlen(str);

    set_winner();

    memcpy(response, str, len);

    return coap_make_response(scratch, outpkt, (const uint8_t *)response, len, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT,
                              COAP_CONTENTTYPE_TEXT_PLAIN);
}

static int handle_set_looser(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt,
        uint8_t id_hi, uint8_t id_lo) {
    const char *str = "looser was displayed";
    int len = strlen(str);

    set_looser();

    memcpy(response, str, len);

    return coap_make_response(scratch, outpkt, (const uint8_t *)response, len, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT,
                              COAP_CONTENTTYPE_TEXT_PLAIN);
}

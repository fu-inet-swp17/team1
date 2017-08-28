char ping_stack[THREAD_STACKSIZE_DEFAULT];

void* ping_handler(void* args);

extern gcoap_listener_t coap_listener;
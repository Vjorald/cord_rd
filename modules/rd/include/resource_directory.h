static ssize_t _registration_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx);

static ssize_t _update_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx);

static ssize_t _endpoint_lookup_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx);

static ssize_t _resource_lookup_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx);

static const coap_resource_t _resources[] = {

    { "/resourcedirectory/", COAP_GET | COAP_PUT | COAP_POST , _registration_handler, NULL },
    { "/reg/",  COAP_PUT | COAP_POST | COAP_DELETE | COAP_MATCH_SUBTREE , _update_handler, NULL },
    { "/endpoint-lookup/",  COAP_GET | COAP_MATCH_SUBTREE , _endpoint_lookup_handler, NULL },
    { "/resource-lookup/",  COAP_GET | COAP_MATCH_SUBTREE , _resource_lookup_handler, NULL}
};

static gcoap_listener_t _listener = {
    _resources,
    ARRAY_SIZE(_resources),
    GCOAP_SOCKET_TYPE_UDP,
    NULL,
    NULL,
    NULL
};
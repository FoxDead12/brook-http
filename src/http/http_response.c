#include "http/http_response.h"

/**
 * Default response body
 * Error:
 * { message: '', title: '', detail: '' }
 */

int
brook_http_response_static ( brook_connection_t* con, uint16_t code, brook_str_t message, brook_str_t detail ) {
  {
    char http[100] = {0};
    brook_str_t _h;
    _h.data = http;
    _h.len = snprintf(http, sizeof(http), "HTTP/1.1 %d %s", code, brook_http_status_code_str(code));
    brook_http_response_add_header(con, _h);
  }

  brook_http_response_add_header(con, (brook_str_t) brook_str("Content-Type: application/json"));
  brook_http_response_add_header(con, (brook_str_t) brook_str("Server: brook-http"));

  {
    // ... build body json ...
    cJSON *body = cJSON_CreateObject();
    cJSON_AddStringToObject(body, "message", message.data);
    cJSON_AddStringToObject(body, "detail", detail.data);
    cJSON_AddNumberToObject(body, "code", code);

    brook_str_t _s_body;
    _s_body.data = cJSON_Print(body);
    _s_body.len = strlen(_s_body.data);

    {
      char len[100] = {0};
      brook_str_t _l;
      _l.data = len;
      _l.len = snprintf(len, sizeof(len), "Content-Length: %d", strlen(_s_body.data));
      brook_http_response_add_header(con, _l);
    }
    brook_http_response_add_body(con, _s_body);

    cJSON_Delete(body);
    free(_s_body.data);
  }

  return BROOK_OK;
}


int
brook_http_response_add_header (brook_connection_t* con, brook_str_t data) {
  brook_http_response_buffer_join(con, data);
  brook_http_response_buffer_join(con, (brook_str_t) brook_str("\r\n"));
  return BROOK_OK;
}

int
brook_http_response_add_body (brook_connection_t* con, brook_str_t data) {
  brook_http_response_buffer_join(con, (brook_str_t) brook_str("\r\n"));
  brook_http_response_buffer_join(con, data);
  return BROOK_OK;
}

int
brook_http_response_buffer_join (brook_connection_t* con, brook_str_t data) {

  size_t bwrite = 0;
  size_t len = 0;
  int work = 0;

  while ( work == 0 ) {
    // add to buffer response the data receive in buffer
    brook_buffer_chain_t* buffer = con->_reponse._data;
    brook_buffer_chain_t* last = NULL;

    while ( buffer != NULL && buffer->free == 0 ) {
      last = buffer;
      buffer = buffer->next;
    }

    if ( buffer == NULL ) {
      printf("Nao existe nenhum buffer de escrita\n");
      buffer = malloc(sizeof(brook_buffer_chain_t));
      buffer->data = malloc(4096);
      buffer->next = NULL;
      buffer->size = 4096;
      buffer->len = 0;
      buffer->nread = 0;
      buffer->free = 4096;

      if ( last == NULL ) {
        con->_reponse._data = buffer;
      } else {
        last->next = buffer;
      }
    }

    unsigned char* buf = buffer->data + buffer->len;
    unsigned char* src = data.data + bwrite;
    len = data.len - bwrite;

    if ( len > buffer->free ) {
      // will need repeate process
      len = buffer->free;
      work = 0;
    } else {
      work = 1;
    }

    memcpy(buf, src, len);
    bwrite += len;
    buffer->len += len;
    buffer->free -= len;

  }

  return BROOK_OK;
}

static const char* brook_http_status_code_str (uint16_t code) {
  switch (code) {
    case 100: return "Continue";
    case 101: return "Switching Protocols";
    case 102: return "Processing";
    case 200: return "OK";
    case 201: return "Created";
    case 202: return "Accepted";
    case 203: return "Non-authoritative Information";
    case 204: return "No Content";
    case 205: return "Reset Content";
    case 206: return "Partial Content";
    case 207: return "Multi-Status";
    case 208: return "Already Reported";
    case 226: return "IM Used";
    case 300: return "Multiple Choices";
    case 301: return "Moved Permanently";
    case 302: return "Found";
    case 303: return "See Other";
    case 304: return "Not Modified";
    case 305: return "Use Proxy";
    case 307: return "Temporary Redirect";
    case 308: return "Permanent Redirect";
    case 400: return "Bad Request";
    case 401: return "Unauthorized";
    case 402: return "Payment Required";
    case 403: return "Forbidden";
    case 404: return "Not Found";
    case 405: return "Method Not Allowed";
    case 406: return "Not Acceptable";
    case 407: return "Proxy Authentication Required";
    case 408: return "Request Timeout";
    case 409: return "Conflict";
    case 410: return "Gone";
    case 411: return "Length Required";
    case 412: return "Precondition Failed";
    case 413: return "Payload Too Large";
    case 414: return "Request-URI Too Long";
    case 415: return "Unsupported Media Type";
    case 416: return "Requested Range Not Satisfiable";
    case 417: return "Expectation Failed";
    case 418: return "I'm a teapot";
    case 421: return "Misdirected Request";
    case 422: return "Unprocessable Entity";
    case 423: return "Locked";
    case 424: return "Failed Dependency";
    case 426: return "Upgrade Required";
    case 428: return "Precondition Required";
    case 429: return "Too Many Requests";
    case 431: return "Request Header Fields Too Large";
    case 444: return "Connection Closed Without Response";
    case 451: return "Unavailable For Legal Reasons";
    case 499: return "Client Closed Request";
    case 500: return "Internal Server Error";
    case 501: return "Not Implemented";
    case 502: return "Bad Gateway";
    case 503: return "Service Unavailable";
    case 504: return "Gateway Timeout";
    case 505: return "HTTP Version Not Supported";
    case 506: return "Variant Also Negotiates";
    case 507: return "Insufficient Storage";
    case 508: return "Loop Detected";
    case 510: return "Not Extended";
    case 511: return "Network Authentication Required";
    case 599: return "Network Connect Timeout Error";
    default: return "";
  }
}

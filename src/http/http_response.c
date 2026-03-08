#include "http/http_response.h"

/**
 * Default response body
 * Error:
 * { message: '', title: '', detail: '' }
 */

int
brook_http_response_static ( brook_connection_t* con, uint16_t code, brook_str_t message, brook_str_t detail ) {
  // ... build body json ...
  cJSON *body = cJSON_CreateObject();
  cJSON_AddStringToObject(body, "message", message.data);
  cJSON_AddStringToObject(body, "detail", detail.data);
  cJSON_AddNumberToObject(body, "code", code);

  int length = strlen(cJSON_Print(body));
  char* b = cJSON_Print(body);
  const char* extra_headers = "Content-Type: application/json\r\nServer: brook-http\r\n";
  const char *template = "HTTP/1.1 %d %s\r\n%sContent-Length: %d\r\n\r\n%s";

  con->_reponse.data.len = asprintf(&con->_reponse.data.data, template,
    code, brook_http_status_code_str(code), extra_headers, length, b);

  cJSON_Delete(body);
  free(b);
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

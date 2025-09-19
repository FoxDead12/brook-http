//
//  brook_http_parse.c
//  http-c-broker
//
//  Created by David Xavier on 02/08/2025.
//

#include "brook_http_parse.h"
#include "brook_http.h"
#include "../core/brook_core.h"

int
brook_http_parse (brook_connection_t* connection) {

	int r = BROOK_OK;

	if (connection->http->state == READ_HEADER) {
		r = brook_http_header_parse(connection);

		if (r == BROOK_DONE) {
			connection->http->state = READ_BODY;
		}

	} else if (connection->http->state == READ_BODY) {
		r = brook_http_body_parse(connection);
	}

	if (r == BROOK_OK && (connection->state == WAITING_POOL_DB || connection->state == WAITING_POOL_REDIS)) {
		printf("tenho de esperar por uma conexão\n");
	}

	return r;
}

int
brook_http_header_parse (brook_connection_t* connection) {

	brook_buffer_t* b = &connection->pos->buf;
	brook_http_t* request = connection->http;

	// ... validate message with regex ...
	regmatch_t matches[5];
	if (regexec(&connection->conf->regex.http_line, (char*) b->start, 5, matches, 0) == REG_NOMATCH) {
		return BROOK_ERROR;
	}

	// ... get method of request ...
	if (brook_http_set_method(connection, (brook_str_t) { matches[1].rm_eo - matches[1].rm_so, (u_char*)b->start + matches[0].rm_so}) == BROOK_ERROR) {
		return BROOK_ERROR;
	}

	// ... get url of request ...
	request->url = (brook_str_t) { matches[2].rm_eo - matches[2].rm_so, b->start + matches[2].rm_so };

	// ... parse params of url ...
	if (matches[4].rm_so >= 0) { // TODO: need fix regex
		request->params = (brook_str_t) { matches[4].rm_eo - (matches[4].rm_so + 1), (u_char*) b->start + matches[4].rm_so + 1};
	} else {
		request->params = (brook_str_t) {0, NULL};
	}

	// ... check end of request exist ...
	u_char* header_end = (u_char*) strstr((char*) b->start, "\r\n\r\n");
	if (header_end == NULL) {
		return BROOK_ERROR;
	} else {
		header_end += 4;
	}

	// ... parse headers of request ...
	request->header.connection 	   = brook_http_request_header_value((char*)b->start, "connection");
	request->header.host           = brook_http_request_header_value((char*) b->start, "host");
	request->header.content_type   = brook_http_request_header_value((char*) b->start, "content-type");
	request->header.content_length = brook_str_to_int(brook_http_request_header_value((char*) b->start, "content-length"));

	// ... validate content type of request ...
	if (brook_strncmp(request->header.content_type.data,  "application/vnd.api+json", request->header.content_type.len) == 0) {
		request->type = JSON_API;
		connection->state = WAITING_POOL_DB;
	} else {
		return BROOK_ERROR;
	}

	// ... validate route in gatekeeper
	if (brook_gatekeeper_validate(connection) == BROOK_ERROR) {
		return BROOK_ERROR;
	}

	// ... create buffer to only point to header of request ...
	size_t header_lenght = header_end - b->start;
	request->_h = malloc(sizeof(brook_buffer_t));
	request->_h->start = b->start;
	request->_h->end = header_end;
	request->_h->length = header_lenght;
	request->_h->size = header_lenght;
	request->_b = NULL;

	// ... if is request dont contain body ...
	if (request->method != POST && request->method != PATCH) {
		return BROOK_OK;
	}

	// ... validate content lenght ...
	if (request->header.content_length <= 0) return BROOK_ERROR;
	if (request->header.content_length > connection->conf->http.max_body_size) return BROOK_ERROR;

	// ... now will parse the reast of message, is possible contain body ...
	size_t body_length_readed = b->length - header_lenght;

	request->_b = malloc(sizeof(brook_chain_t));
	request->_b->buf.start = header_end;
	request->_b->buf.end = b->start + b->length;
	request->_b->buf.length = body_length_readed;
	request->_b->buf.size = body_length_readed;
	request->_bp = request->_b;

	if (body_length_readed >= request->header.content_length) {
		return BROOK_OK;
	} else {
		return BROOK_DONE;
	}

	return BROOK_ERROR;
}

int
brook_http_body_parse (brook_connection_t* connection) {

	brook_buffer_t* b = &connection->pos->buf;
	brook_http_t* request = connection->http;

	if (request->_bp->buf.length >= request->_bp->buf.size) {
		request->_bp->next = malloc(sizeof(brook_chain_t));
		request->_bp = request->_bp->next;
		request->_bp->buf.start = b->start;
		request->_bp->buf.end = b->start + b->length;
		request->_bp->buf.length = b->length;
		request->_bp->buf.size = b->size;
	} else {
		request->_bp->buf.end = b->start + b->length;
		request->_bp->buf.length = b->length;
	}

	// ... calculate current content lenght ...
	int current_content_lenght = 0;
	brook_chain_t* header = request->_b;
	while (header != NULL) {
		current_content_lenght += header->buf.length;
		header = header->next;
	}

	// ... check if i need indicate socket to keep reading or no ...
	if (current_content_lenght >= request->header.content_length) {
		return BROOK_OK;
	} else {
		return BROOK_DONE;
	}

}

int
brook_http_set_method (brook_connection_t* connection, brook_str_t method) {

	brook_http_t* request = connection->http;

	if (brook_strncmp(method.data, "GET", method.len) == 0) {
		request->method = GET;
	}
	else if (brook_strncmp(method.data, "POST", method.len) == 0) {
		request->method = POST;
	}
	else if (brook_strncmp(method.data, "PATCH", method.len) == 0) {
		request->method = PATCH;
	}
	else if (brook_strncmp(method.data, "DELETE", method.len) == 0) {
		request->method = DELETE;
	} else {
		return BROOK_ERROR;
	}

	return BROOK_OK;
}

brook_str_t
brook_http_request_header_value (char* buf, const char* key) {

	brook_str_t s = {0, NULL};

	size_t key_len = strlen(key);
	char* pos = (char*)buf;

	while ((pos = strcasestr(pos, key)) != NULL) {
		if ((pos == buf || *(pos - 1) == '\n') && strncasecmp(pos, key, key_len) == 0) {
			char* start = strchr(pos, ':');
			if (start == NULL) {
				return s;
			}

			start += 1; // jump ':'
			while (*start == ' ' || *start == '\t') start++;
			char* end = strstr(start, "\r\n");
			if (end == NULL) {
				return s;
			}

			s.data = (u_char*) start;
			s.len  = end - start;
			return s;
		}
		pos++;
	}

	return s;

}

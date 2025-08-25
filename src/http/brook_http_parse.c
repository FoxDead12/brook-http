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
	
	if (connection->state == READ_HEADER) {
		r = brook_http_header_parse(connection);
		
		if (r == BROOK_DONE) {
			connection->http->state = READ_BODY;
		}
		
	} else if (connection->state == READ_BODY) {
		r = brook_http_body_parse(connection);
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
	} else if (brook_strncmp(request->header.content_type.data,  "application/json", request->header.content_type.len) == 0) {
		request->type = JOB;
	} else {
		return BROOK_ERROR;
	}
	
	// ... validate route in gatekeeper
	if (brook_gatekeeper_validate(connection) == BROOK_ERROR) {
		return BROOK_ERROR;
	}
	
	// ... if is request dont contain body ...
	if (request->method != POST && request->method != PATCH) {
		return BROOK_OK;
	}
	
	// ... validate content lenght ...
	if (request->header.content_length <= 0) return BROOK_ERROR;
	if (request->header.content_length > connection->conf->http.max_body_size) return BROOK_ERROR;
	
	/*
	
    // ... check content length necessary
    int content_length = connection->http->header.content_length;
    if (content_length <= 0) return BROOK_OK;
    if (content_length > connection->conf->http.max_body_size) return BROOK_ERROR;
    
    // ... run logic to each apllication type
    // TODO: for now we only will catch json api routes and job
	if (connection->http->type == JSON_API) {
		
        // ... calculate current lenght loaded
        size_t header_size = end - buf->start;
        size_t body_already_loaded = buf->len - header_size;
        
        // ... check if already load all body is done
        if (body_already_loaded >= content_length) {
			brook_chain_t* b = malloc(sizeof(brook_chain_t));
			b->buf.start = end;
			b->buf.size  = body_already_loaded;
			b->buf.len   = body_already_loaded;
			b->buf.free  = 1;
			
			connection->pos->next = b;
			connection->pos = b;
			connection->http->buff_body = &b->buf;
			
            return BROOK_OK;
        }
        
        // ... need continue reading socket, but is json object so create buffer with body lenght
        brook_chain_t* b = malloc(sizeof(brook_chain_t));
        b->buf.start = calloc(1, content_length + 1);
        b->buf.size  = content_length;
        b->buf.len   = body_already_loaded;

        memmove(b->buf.start, end, body_already_loaded);
        
		connection->pos->next = b;
		connection->pos = b;
        
        connection->http->state = READING_BODY;
		connection->http->buff_body = &b->buf;
		
		return BROOK_DONE;
        
    }
        
    // ... dont exist handler to content type, defined
	 */
    return BROOK_ERROR;
}

int
brook_http_body_parse (brook_connection_t* connection) {
	
	/*
	brook_buffer_t* buf = &connection->pos->buf;
	
	if (connection->http->type == JSON_API || connection->http->type == JOB) {
		// only ckeck if need keep reading or is all data stored
		if (connection->http->header.content_length == buf->len) {
			return BROOK_OK;
		} else {
			return BROOK_DONE;
		}
	}
	*/
	return BROOK_ERROR;
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

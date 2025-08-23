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
    /*
    int rs;

	switch (connection->http->state) {
        case READING_HEADER: rs = brook_http_header_handler(connection); break;
        case READING_BODY:   rs = brook_http_body_handler(connection);   break;
        default: return BROOK_ERROR; break;
    }
    
    if (rs == BROOK_OK) {
        
        // ... json api ...
        if (connection->http->type == JSON_API) {
            if (connection->http->method == POST || connection->http->method == PATCH) {
                if (brook_json_api_setup_body(connection->http) == BROOK_ERROR) return BROOK_ERROR;
            }
            connection->http->state = BUILDING_QUERY;
        }
        // ... json api ...
        
    }
	
    return rs;*/
	return BROOK_OK;
}

int
brook_http_header_handler (brook_connection_t* connection) {
		
	/*
    // .. current buffer in chain
	brook_buffer_t* buf = &connection->pos->buf;
    
    // ... Validate and parse first line of socket message
    regmatch_t matches[5];
    if (regexec(&connection->conf->regex.http_line, (char*) buf->start, 5, matches, 0) == REG_NOMATCH) {
        return BROOK_ERROR;
    }
    
    // ... Parse method of regex
	brook_str_t method;
	method.data = buf->start + matches[0].rm_so;
	method.len  = matches[1].rm_eo - matches[1].rm_so;
	if (brook_http_set_method(connection, method) == BROOK_ERROR) {
		return BROOK_ERROR;
	}
	
    // ... Parse url withou params of regex
    connection->http->url.data = buf->start + matches[2].rm_so;
    connection->http->url.len  = matches[2].rm_eo - matches[2].rm_so;
    
    // Parse params of url
    if (matches[4].rm_so >= 0) { // TODO: need fix regex
        connection->http->params.data = buf->start + matches[4].rm_so + 1;
        connection->http->params.len  = matches[4].rm_eo - (matches[4].rm_so + 1);
	} else {
		connection->http->params.data = NULL;
		connection->http->params.len  = 0;
	}
        
    u_char* end = (u_char*) strstr((char*) buf->start, "\r\n\r\n");
    if (end == NULL) {
        return BROOK_ERROR;
    }
    end += 4; // jump '\r\n\r\n'
    
    // ... Parse the necessary headers in http request
    connection->http->header.connection     = brook_http_request_header_value((char*) buf->start, "connection");
    connection->http->header.host           = brook_http_request_header_value((char*) buf->start, "host");
    connection->http->header.content_type   = brook_http_request_header_value((char*) buf->start, "content-type");
    connection->http->header.content_length = brook_str_to_int(brook_http_request_header_value((char*) buf->start, "content-length"));
    
    if (brook_array_find_value(connection->conf->http.allow_content_types, connection->http->header.content_type) == -1) {
        return BROOK_ERROR;
    }
    
    if (brook_gatekeeper_validate(connection) == BROOK_ERROR) {
        return BROOK_ERROR;
    }
    
	if (brook_http_request_check_type(connection) == BROOK_ERROR) {
		return BROOK_ERROR;
	}
    
	if (connection->http->method != POST && connection->http->method != PATCH) {
        return BROOK_OK;
    }
	
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
brook_http_body_handler(brook_connection_t* connection) {
	
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
brook_http_request_check_type (brook_connection_t* connection) {
	/*
	// detetar se o pedido é valido, verificando se existe no gatekeeper, o url desejado e o metodo indiciado
	// tendo validado é preciso verificar que tipo de pedido é JSON-API, JOB ou UPLOAD FILE
	
	brook_http_t* request = connection->http;
		
	if (brook_strncmp(request->header.content_type.data, "application/vnd.api+json", request->header.content_type.len) == 0) {
		brook_json_api_setup(request);
		request->type = JSON_API;
	}
	else if (brook_strncmp(request->header.content_type.data, "application/json", request->header.content_type.len) == 0) {
		request->type = JOB;
	} else {
		return BROOK_ERROR;
	}
	*/
	return BROOK_OK;
}

int
brook_http_set_method (brook_connection_t* connection, brook_str_t method) {
	/*
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
	*/
	return BROOK_OK;
}

brook_str_t
brook_http_request_header_value (char* buf, const char* key) {
    /*
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
	 */
	return (brook_str_t) {0, (u_char*) ""};
}

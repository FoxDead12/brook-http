//
//  brook_connection.c
//  http-c-broker
//
//  Created by David Xavier on 29/07/2025.
//

#include "brook_connection.h"
#include "brook_core.h"
#include "../http/brook_http.h"

brook_connection_t*
brook_create_connection (brook_config_t* conf) {

	struct sockaddr_in client_addr;

	int c_socket = brook_socket_accept(conf->socket, &client_addr);
    if (c_socket <= 0) {
        return NULL;
    }

	// ... init connection ...
    brook_connection_t* connection = malloc(sizeof(brook_connection_t));
    connection->conf = conf;
    connection->socket = c_socket;
	connection->state = READING_SOCKET_MESSAGE;
	connection->port = ntohs(client_addr.sin_port);
	inet_ntop(AF_INET, &(client_addr.sin_addr), connection->ip, INET_ADDRSTRLEN);
    {
        connection->ch_buf = malloc(sizeof(brook_chain_t));
        connection->ch_buf->next = NULL;
        connection->pos = connection->ch_buf;
    }
	{
		connection->http = malloc(sizeof(brook_http_t));
		brook_http_t* http = connection->http;
		http->connection = connection;
		http->json_api = NULL;
		http->state = READ_HEADER;
	}

    return connection;
}

int
brook_close_connection (brook_connection_t* connection) {

	{
        // ... free http request ...
		brook_http_t* request = connection->http;
		free(request->_h);
		
		brook_chain_t* header = request->_b;
		while (header != NULL) {
			brook_chain_t* tmp = header->next;
			free(header);
			header = NULL;
			header = tmp;
		}
		request->_b = NULL;
	}
	
	{
		// ... free chain buffer who contain socket data ...
		brook_chain_t* header = connection->ch_buf;
		while (header != NULL) {
			brook_chain_t* tmp = header->next;
			free(header->buf.start);
			free(header);
			header = NULL;
			header = tmp;
		}
		connection->ch_buf = NULL;
	}
	
	close(connection->socket);
	free(connection);

    return BROOK_OK;
}


int
brook_read_message_connection (brook_connection_t* connection) {

	brook_buffer_t* b = &connection->pos->buf;

	// ... alloc memory in buffer ...
	if (b->start == NULL) {
		b->start = calloc(1, connection->conf->http.buffers_size + 1); // i make this to force buffer end with '\n'
		b->size = connection->conf->http.buffers_size;
		b->length = 0;
	}

    // ... read content from socket ...
	size_t len_diff = b->size - b->length;
	size_t bytes = brook_socket_read(connection->socket, b->start + b->length, len_diff);
    
    // ... is possible dont read nothing in socket ...
    if (bytes == BROOK_ERROR) {
        return BROOK_ERROR;
    }
	b->length += bytes;
	
	int r = brook_http_parse(connection);
    
    // ... if my buffer is full, create new chain buffer ...
	if (b->length >= b->size && r == BROOK_DONE) {
        connection->pos->next = malloc(sizeof(brook_chain_t));
        connection->pos = connection->pos->next;
        connection->pos->next = NULL;
        return BROOK_DONE;
	}

	return r;
}

int
brook_connection_write_psql (brook_connection_t* connection, int pg_socket) {
	
	// ... TODO: FOR NOW ONLY JSON-API WILL COMUNICATE WITH DB ...
	
	PGconn* db = brook_postgres_get_connection_from_socket(connection->conf, pg_socket);
	
	if (connection->http->type == JSON_API) {
		return brook_json_api_write_query(connection->http, db);
	}
		
	return BROOK_ERROR;
}

int
brook_connection_read_psql (brook_connection_t* connection, int pg_socket) {
	
    PGconn* db = brook_postgres_get_connection_from_socket(connection->conf, pg_socket);

    if (connection->http->type == JSON_API) {
        return brook_json_api_read_query(connection->http, db);
    }
	
	return BROOK_ERROR;
}

int
brook_connection_write_redis (brook_connection_t* connection) {
	
	return BROOK_OK;
}

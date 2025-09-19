//
//  brook_json_api.c
//  http-c-broker
//
//  Created by David Xavier on 08/08/2025.
//

#include "brook_json_api.h"
#include "brook_http.h"
#include "../core/brook_core.h"

const char* QUERY_SELECT = "SELECT %s FROM %s %s ORDER BY %s LIMIT %s OFFSET %s";
const char* QUERY_DELETE = "DELETE FROM %s WHERE id = '%.*s'";
const char* QUERY_INSERT = "INSERT INTO %s VALUES %s";
const char* QUERY_UPDATE = "UPDATE %s SET %s WHERE %s";

int
brook_json_api_free (brook_http_t* request) {

    brook_json_api_query_chain_t* header = request->json_api->querys_list;

    while (header != NULL) {
        brook_json_api_query_chain_t* tmp = header->next;
        free(header->query_s.query);
        free(header);
        header = NULL;
        header = tmp;
    }

    request->json_api->querys_list = NULL;

    json_object_put(request->json_api->result);

    free(request->json_api);

    return BROOK_OK;
}

int
brook_json_api_setup (brook_http_t* request) {

    brook_config_t* s_conf = request->connection->conf;

    // ... check gatekeeper contain resource data ...
    if (request->gatekeeper_route->resource == NULL) {
        return BROOK_ERROR;
    }

    // ... get resource config from gatekeeper settings ...
    json_object* server_resource = json_object_object_get(s_conf->resources, request->gatekeeper_route->resource);
    if (server_resource == NULL) {
        return BROOK_ERROR;
    }

    // ... create struct in request ...
    request->json_api               = malloc(sizeof(brook_json_api_t));
    request->json_api->request    = request;
    request->json_api->s_resource = server_resource;
    request->json_api->result       = json_object_new_object();
    request->json_api->querys_list = malloc(sizeof(brook_json_api_query_chain_t));

    // ... if method don't contain body out here ...
    if ((request->method == POST || request->method == PATCH) && request->_b == NULL) {
        return BROOK_ERROR;
    }

    brook_json_api_query_t* query_s = &request->json_api->querys_list->query_s;
    query_s->table = json_get_str("table", server_resource, (brook_str_t) brook_string(""));

    // ... now the ideia is create all query templates ...
    if (request->method == GET) {

        // ... SELECT -> need attributes, table, filter, order by, and page (limit, offset) ...
        query_s->type = Q_SELECT;
        query_s->query_template = (char*) QUERY_SELECT;

        query_s->order = (brook_str_t) brook_string("id");
        query_s->limit = (brook_str_t) brook_string("100");
        query_s->offset = (brook_str_t) brook_string("0");
//   const char* QUERY_SELECT = "SELECT %s FROM %s %s ORDER BY %s LIMIT %s OFFSET %s";

        asprintf(&query_s->query, query_s->query_template,
                 "*",
                 query_s->table.data,
                 "",
                 query_s->order.data,
                 query_s->limit.data,
                 query_s->offset.data);


    } else if (request->method == DELETE) {

        query_s->type = Q_DELETE;
        query_s->query_template = (char*) QUERY_DELETE;

        if (brook_json_api_parse_id(request, query_s) == BROOK_ERROR) {
            return BROOK_ERROR;
        }

        asprintf(&query_s->query, query_s->query_template, query_s->table.data, query_s->id.len, query_s->id.data);

    } else if (request->method == POST) {

        // ... INSERT -> need table, attributes key ...
        query_s->type = Q_INSERT;
        query_s->query_template = (char*) QUERY_INSERT;

    } else if (request->method == PATCH) {

        // ... UPDATE -> need table, attributes key and id to update ...
        query_s->type = Q_UPDATE;
        query_s->query_template = (char*) QUERY_UPDATE;

        if (brook_json_api_parse_id(request, query_s) == BROOK_ERROR) {
            return BROOK_ERROR;
        }

    } else return BROOK_ERROR;

    return BROOK_OK;
}

int
brook_json_api_write_query (brook_http_t* request, PGconn* db) {

	if (request->json_api == NULL) {
        // ... need setup json api ...
		if (brook_json_api_setup(request) == BROOK_ERROR) {
			return BROOK_ERROR;
		}
	}

	brook_json_api_query_chain_t* q_chain = request->json_api->querys_list;
	PQsendQuery(db, q_chain->query_s.query);

	request->connection->state = READING_PSQL_MESSAGE;

	return BROOK_OK;
}

int
brook_json_api_read_query (brook_http_t* request, PGconn* db) {

    PGresult *res = NULL;

    if (PQconsumeInput(db) == 0) {
        fprintf(stderr, "postgres erro consume result: %s\n", PQerrorMessage(db));
        return BROOK_ERROR;
    }

    while ((res = PQgetResult(db)) != NULL) {

        ExecStatusType status = PQresultStatus(res);

        if (status == PGRES_TUPLES_OK) {
            brook_json_api_parse_postgres_result(request, res);
        }
        else if (status == PGRES_COMMAND_OK) {
            // Comando executado sem retorno de linhas
        }
        else if (status == PGRES_FATAL_ERROR) {
        }

        PQclear(res);
    }

    size_t body_len = 0;
    const char* body = json_object_to_json_string_length(request->json_api->result, 0, &body_len);

    brook_http_response_send(request->connection, 200, (u_char*) body, body_len);

	return BROOK_OK;
}

int
brook_json_api_parse_id (brook_http_t* request, brook_json_api_query_t* query_s) {

    u_char* pointer = request->url.data + request->url.len - 1;

    while (pointer >= request->url.data && *pointer != '/') {
        pointer--;
    }

    if (pointer != request->url.data) {
        // ... exist id ...
        query_s->id.data = pointer + 1;
        query_s->id.len  = (request->url.data + request->url.len) - query_s->id.data;
        return BROOK_OK;
    }

    return BROOK_ERROR;

}

int
brook_json_api_parse_postgres_result (brook_http_t* request, PGresult *res) {

    int nfields = PQnfields(res);
    int nrows = PQntuples(res);

    json_object* data = json_object_new_array();


    for (int i = 0; i < nrows; i++) {
        json_object *item = json_object_new_object();
        for (int j = 0; j < nfields; j++) {
            char *colname = PQfname(res, j);
            char *value = PQgetvalue(res, i, j);
            json_object_object_add(item, colname, json_object_new_string(value));
        }
        json_object_array_add(data, item);
    }

    json_object_object_add(request->json_api->result, "data", data);

    return BROOK_OK;
}







int
brook_json_api_setup_body (brook_http_t* request) {
    /*
    // ... transform body in json ...
    request->json_api->body = json_tokener_parse((char*) request->buff_body->start);

    // ... check if body contain god format of json api
    json_object* data = json_object_object_get(request->json_api->body, "data");
    if (data == NULL) return BROOK_ERROR;

    // ... type in body need bee equal to resource name in json ...
    json_object* type = json_object_object_get(data, "type");
    if (type == NULL) return BROOK_ERROR;
    if (brook_strncmp(request->gatekeeper_route->resource, json_object_get_string(type), json_object_get_string_len(type)) != 0) return BROOK_ERROR;

    // ... if method is PATCH and id is null is invalid, is necessary indicate id ...
    json_object* id = json_object_object_get(data, "id");
    if (request->method == PATCH && id == NULL) return BROOK_ERROR;

    // ... check if contain attributes ...
    json_object* attributes = json_object_object_get(data, "attributes");
    if (attributes == NULL) return BROOK_ERROR;

    // ... set data to store in object ...
    request->json_api->data = data;

    // ... set type ...
    request->json_api->type.data = (u_char*) json_object_get_string(type);
    request->json_api->type.len  = json_object_get_string_len(type);

    // ... set id ...
    if (id == NULL) {
        request->json_api->type.data = NULL;
        request->json_api->type.len  = 0;
    } else {
        request->json_api->type.data = (u_char*) json_object_get_string(id);
        request->json_api->type.len  = json_object_get_string_len(id);
    }

    request->json_api->attributes = attributes;
    */
	return BROOK_OK;
}

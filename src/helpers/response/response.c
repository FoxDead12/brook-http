//
//  response.c
//  http-c-broker
//
//  Created by David Xavier on 17/07/2025.
//

#include "response.h"

int send_json_api_response_error (http_connection_struct* con, int status, const char* code, const char* detail) {

    con->response.json_api.b = json_object_new_object();
    json_object* body = con->response.json_api.b;

    json_object* errors = build_json_api_error_obj (status, code, detail);
    json_object_object_add(body, "errors", errors);

    send_json_api_response(con);
    
    return HTTP_OK;
}

int send_json_api_response (http_connection_struct* con) {
    
    int http_status;
    json_object* json = con->response.json_api.b;
    
    // If existe error get http status code to response
    json_object* errors = NULL;
    if (json_object_object_get_ex(json, "errors", &errors)) {
        json_object* error = json_object_array_get_idx(errors, 0);
        json_object* status = NULL;
        if (json_object_object_get_ex(error, "status", &status)) {
            http_status = json_object_get_int(status);
        } else {
            http_status = 400;
        }
    } else {
        
        if (comp_str_to_str(con->request.method, http_str("POST")) == 0) {
            http_status = 201;
        } else {
            http_status = 200;
        }
        
    }

    const char* json_string = json_object_to_json_string(con->response.json_api.b);
    size_t json_lenght      = strlen(json_string);
    
    char* response;
    size_t response_len = response_header_format(&response, http_status, json_lenght);
    write(con->socket, response, response_len);
    
    write(con->socket, json_string, json_lenght);
    
    free(response);
    json_object_put(con->response.json_api.b);
    
    return HTTP_OK;
}

json_object* build_json_api_error_obj_from_db_result (PGresult* res) {
    
    const char *s = PQresultErrorField(res, PG_DIAG_SQLSTATE);
//    const char* e = PQresultErrorMessage(res); //TODO: Will be used in custom codes of db
    
    char* detail;
    char* code;
    int status;
    
    if (s == NULL ){
        
        status = 400;
        detail = "Resource execution error";
        code   = "HTTP_BROKER_ERROR";
        
    } else if (strcmp(s, "42P01") == 0) {
        
        status = 404;
        detail = "Resource table not found";
        code   = "HTTP_BROKER_ERROR_FOUND";
        
    } else if (strcmp(s, "23505")) {
        
        status = 400;
        detail = "Resource already exist";
        code   = "HTTP_BROKER_ERROR_EXIST";
        
    } else if (strcmp(s, "23503") == 0) {
        
        status = 400;
        detail = "Resource already exist in relationship resource";
        code   = "HTTP_BROKER_ERROR_EXIST";
        
    } else if (strcmp(s, "42703") == 0) {
        
        status = 400;
        detail = "Resource attribute does not exist";
        code   = "HTTP_BROKER_ERROR_ATTRIBUTE";
        
    }
    else {
        status = 400;
        detail = "Resource execution error";
        code   = "HTTP_BROKER_ERROR";
    }
    
    return build_json_api_error_obj(status, code, detail);
}

json_object* build_json_api_error_obj (int status, const char* code, const char* detail) {
    
    json_object* erros = json_object_new_array();
    
    json_object* error = json_object_new_object();
    json_object_object_add(error, "status", json_object_new_int(status));
    json_object_object_add(error, "code",   json_object_new_string(code));
    json_object_object_add(error, "detail", json_object_new_string(detail));
    
    json_object_array_add(erros, error);
    
    return erros;
}

size_t response_header_format (char** r, int status, size_t content_lenght) {
    
    char* status_description;
    
    if (status == 200) {
        status_description = "OK";
    } else if (status == 201) {
        status_description = "Created";
    } else if (status == 400) {
        status_description = "Bad Request";
    } else if (status == 404) {
        status_description = "Not Found";
    } else {
        status_description = "";
    }
    
    const char *response =
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: Content-Type: application/vnd.api+json;\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n";

    size_t len = asprintf(r, response, status, status_description, content_lenght);
    
    return len;
}

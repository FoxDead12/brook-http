//
//  brook_kqueue.c
//  http-c-broker
//
//  Created by David Xavier on 28/07/2025.
//

#include "brook_kqueue.h"

int
brook_start_kernel_event (brook_config_t* conf) {

    // ... start event queue ...
    int kq = kqueue();
    struct kevent kq_list[MAX_EVENTS];

    // ... add event of server socket (accepts connections) and parent process to see if exit ...
    brook_kqueue_set_descriptor(kq, conf->socket, EVFILT_READ, EV_ADD, 0, 0, NULL);
    brook_kqueue_set_descriptor(kq, conf->brook_parent_process, EVFILT_PROC, EV_ADD, NOTE_EXIT, 0, NULL);

    // ... iterate event queue ...
    while (1) {
        int n = kevent(kq, NULL, 0, kq_list, MAX_EVENTS, NULL);

        for (int i = 0; i < n; i++) {
            brook_kevent_handle(kq, kq_list[i], conf);
        }
    }

    return BROOK_OK;
}

int
brook_kqueue_set_descriptor (int kq, int fd, int filter, int flags, int fflags, size_t data, void* udata) {
    struct kevent set;
    EV_SET(&set, fd, filter, flags, fflags, data, udata);
    kevent(kq, &set, 1, NULL, 0, NULL);
    return BROOK_OK;
}

void
brook_kevent_handle (int kq, struct kevent event, brook_config_t* conf) {

	brook_connection_t* c = NULL;

    // ... check if is new connection to server ...
    if (event.ident == conf->socket) {
        c = brook_create_connection(conf);
        if (c != NULL) {
            brook_kqueue_set_descriptor(kq, c->socket, EVFILT_READ, EV_ADD, 0, 0, c);
            brook_kqueue_set_descriptor(kq, c->socket, EVFILT_TIMER, EV_ADD | EV_ONESHOT, 0, conf->http.timeout, c);
        }
		return;
    }

	// ... handle rest of events ...
	c = event.udata;
	int r = BROOK_ERROR;
	// ... TODO: CHECK IF CONNECTION WAS ALREADY CLOSE BEFORE FIRE EVENT ...
	
    switch (event.filter) {

		// ... events where need read content from socket ...
		case EVFILT_READ:
			r = brook_kevent_read(c, event);
		break;

        // ... events used to make a stack of events, to next enable write (its middle intermediate, before write, dont contain connection whet) will be used to redis and postgres ...
        case EVFILT_USER:
            r = brook_kevent_user(kq, c);
        break;

        // ... moment where we contain socket connection and will write ...
        case EVFILT_WRITE:
			r = brook_kevent_write(c, event);
        break;

        // ... will execute timout of request ...
        case EVFILT_TIMER:
            brook_close_connection(c);
            brook_kqueue_set_descriptor(kq, (int) event.ident, event.filter, EV_DELETE, 0, 0, NULL);
            return;
        break;
    }
    
    // ... if event after run retur error or ok is to remove old event ...
    if (r != BROOK_DONE) {
        brook_kqueue_set_descriptor(kq, (int) event.ident, event.filter, EV_DELETE, 0, 0, NULL);
    }
	
	if (r == BROOK_ERROR) {
		// TODO: IF RETURN SOME ERROR, NEED CREATE EVENT OF WRITE
	}
    
    if (r == BROOK_OK) {
        if (c->state == WAITING_POOL_DB || c->state == WAITING_POOL_REDIS) {
            brook_kqueue_set_descriptor(kq, c->socket, EVFILT_USER, EV_ADD, 0, 0, NULL);
            brook_kqueue_set_descriptor(kq, c->socket, EVFILT_USER, 0, NOTE_TRIGGER, 0, c);
        } else if (c->state == READING_PSQL_MESSAGE) {
			brook_kqueue_set_descriptor(kq, (int) event.ident, EVFILT_READ, EV_ADD, 0, 0, c);
		}
    }
    
    // ... if event return is BROOK_DONE need repeat the event ...
    
}

int
brook_kevent_read (brook_connection_t* connection, struct kevent event) {
	switch (connection->state) {
        case READING_SOCKET_MESSAGE:
            return brook_read_message_connection(connection);
        break;
		case READING_REDIS_MESSAGE: break;
		case READING_PSQL_MESSAGE:
			return brook_connection_read_psql(connection, (int) event.ident);
		default: break;
	}
    return BROOK_ERROR;
}

int
brook_kevent_write (brook_connection_t* connection, struct kevent event) {
	switch (connection->state) {
		case WRITING_SOCKET_MESSAGE: break;
		case WRITING_REDIS_MESSAGE: break;
		case WRITING_BEANSTALK_MESSAGE: break;
		case WRITING_PSQL_MESSAGE:
			return brook_connection_write_psql(connection, (int) event.ident);
		default: break;
	}
    return BROOK_ERROR;
}

int
brook_kevent_user (int kq, brook_connection_t* connection) {
            
    if (connection->state == WAITING_POOL_DB) {
        
		PGconn* db = brook_postgres_get_connection(connection->conf);
		if (db == NULL) return BROOK_DONE;
		connection->state = WRITING_PSQL_MESSAGE;
		brook_kqueue_set_descriptor(kq, PQsocket(db), EVFILT_WRITE, EV_ADD, 0, 0, connection);
		
    } else if (connection->state == WAITING_POOL_REDIS) {
		return BROOK_ERROR;// TODO: REMOVE THIS LINE, IS GUST TEMPORARY
    }
    
	return BROOK_OK;
}

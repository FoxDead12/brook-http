//
//  brook_kqueue.c
//  http-c-broker
//
//  Created by David Xavier on 28/07/2025.
//

#include "brook_kqueue.h"

brook_wait_list_t* brook_event_await = NULL;
brook_wait_list_t* brook_event_await_last = NULL;

int
brook_start_kernel_event (brook_config_t* conf) {

    // ... start event queue ...
    int kq = kqueue();
    struct kevent kq_list[MAX_EVENTS];

    // ... add event of server socket (accepts connections) and parent process to see if exit ...
    brook_kqueue_set_descriptor(kq, conf->socket, EVFILT_READ, EV_ADD, 0, 0, NULL);
    brook_kqueue_set_descriptor(kq, conf->brook_parent_process, EVFILT_PROC, EV_ADD, NOTE_EXIT, 0, NULL);
    brook_kqueue_set_descriptor(kq, 1, EVFILT_USER, EV_ADD | EV_ENABLE | EV_CLEAR, 0, 0, NULL);

    // ... set events to postgress connections
    for (int i = 0; i < conf->postgres_con_worker; i++) {
        int socket = PQsocket(conf->postgres_conns->conns[i]);
        brook_kqueue_set_descriptor(kq, socket, EVFILT_READ, EV_ADD | EV_DISABLE | EV_CLEAR, 0, 0, NULL);
        brook_kqueue_set_descriptor(kq, socket, EVFILT_WRITE, EV_ADD | EV_DISABLE | EV_CLEAR, 0, 0, NULL);
    }

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


    /*

        POSSIBLE EVENTS:
            -> clients socket: READ, WRITE;
            -> a fucking event to connections wait for services connectios
            -> postgres: READ, WRITE;
            -> redis: READ, WRITE;

        --- for now we dont has timeout

    */

    brook_connection_t* c = NULL;

    // ... accept new connetions to server ... //
    if (event.ident == conf->socket) {
        c = brook_create_connection(conf);
        if (c != NULL) {
			brook_kqueue_set_descriptor(kq, c->socket, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, c);
        }
		return;
    } else {
        c = event.udata;
    }

    int r = BROOK_ERROR;

    switch (event.filter) {

        case EVFILT_READ:
            r = brook_kevent_read(c, event);
            break;

        case EVFILT_WRITE:
            r = brook_kevent_write(c, event);
            break;

        case EVFILT_USER:
			c = brook_kevent_user();
			if (c) {
				r = BROOK_OK;
			}

			if (brook_event_await != NULL) {
				brook_kqueue_set_descriptor(kq, 1, EVFILT_USER, 0, NOTE_TRIGGER, 0, NULL);
			}

            break;

    }

    if (event.flags & EV_EOF) {
        if (c->socket_ext_type == PSQL) {
            brook_postgres_free_connection(c->conf, c->socket_ext);
        }
        brook_kqueue_set_descriptor(kq, c->socket, EVFILT_READ, EV_DELETE, 0, 0, NULL);
        brook_kqueue_set_descriptor(kq, c->socket, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
        brook_close_connection(c);
        return;
    }

    if (r == BROOK_OK) {
        if (c->state == WAITING_POOL_DB || c->state == WAITING_POOL_REDIS) {
            if (event.filter != EVFILT_USER) {
                brook_wait_list_t* item = malloc(sizeof(brook_wait_list_t));
                item->c = c;
                item->next = NULL;
                if (brook_event_await == NULL) {
                    brook_event_await = item;
                    brook_event_await_last = item;
                } else {
                    brook_event_await_last->next = item;
                    brook_event_await_last = item;
                }
            }

            brook_kqueue_set_descriptor(kq, c->socket, EVFILT_READ, EV_DISABLE, 0, 0, NULL);
			brook_kqueue_set_descriptor(kq, 1, EVFILT_USER, 0, NOTE_TRIGGER, 0, NULL);
		} else if (c->state == WRITING_PSQL_MESSAGE) {

            brook_kqueue_set_descriptor(kq, c->socket_ext, EVFILT_READ, EV_DISABLE, 0, 0, NULL);
			brook_kqueue_set_descriptor(kq, c->socket_ext, EVFILT_WRITE, EV_ENABLE, 0, 0, c);
		} else if (c->state == READING_PSQL_MESSAGE) {
			brook_kqueue_set_descriptor(kq, c->socket_ext, EVFILT_READ, EV_ENABLE, 0, 0, c);
            brook_kqueue_set_descriptor(kq, c->socket_ext, EVFILT_WRITE, EV_DISABLE, 0, 0, NULL);
		} else if (c->state == WRITING_SOCKET_MESSAGE) {
            brook_kqueue_set_descriptor(kq, c->socket_ext, EVFILT_READ, EV_DISABLE, 0, 0, NULL);
            brook_kqueue_set_descriptor(kq, c->socket_ext, EVFILT_WRITE, EV_DISABLE, 0, 0, NULL);
			brook_kqueue_set_descriptor(kq, c->socket, EVFILT_WRITE, EV_ADD | EV_CLEAR, 0, 0, c);

            if (c->socket_ext_type == PSQL) {
                brook_postgres_free_connection(c->conf, c->socket_ext);
            }

		} else if (c->state == CLOSED) {
            brook_kqueue_set_descriptor(kq, c->socket, EVFILT_READ, EV_DELETE, 0, 0, NULL);
            brook_kqueue_set_descriptor(kq, c->socket, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
			brook_close_connection(c);
		}
    }




// 	brook_connection_t* c = NULL;
//
//     if (event.ident == conf->socket) {
        // c = brook_create_connection(conf);
        // if (c != NULL) {
        //     brook_kqueue_set_descriptor(kq, c->socket, EVFILT_READ, EV_ADD, 0, 0, c);
        // }
		// return;
//     }
//
// 	c = event.udata;
//
// 	int r = BROOK_ERROR;
// 	// ... TODO: CHECK IF CONNECTION WAS ALREADY CLOSE BEFORE FIRE EVENT ...
//
//     switch (event.filter) {
//
// 		// ... events where need read content from socket ...
// 		case EVFILT_READ:
// 			r = brook_kevent_read(c, event);
// 		break;
//
//         // ... events used to make a stack of events, to next enable write (its middle intermediate, before write, dont contain connection whet) will be used to redis and postgres ...
//         case EVFILT_USER:
//             r = brook_kevent_user(kq, c);
//         break;
//
//         // ... moment where we contain socket connection and will write ...
//         case EVFILT_WRITE:
// 			r = brook_kevent_write(c, event);
//         break;
//
//         // ... will execute timout of request ...
//         case EVFILT_TIMER:
// 			/*
// 			c->state = CLOSED;
//             if (c->socket_ext_type == PSQL) {
//                 brook_postgres_free_connection(c->conf, c->socket_ext);
//             }
// 			 */
//
//         break;
//     }
//
//     // ... if event after run return error or ok is to remove old event ...
//     if (r != BROOK_DONE) {
//         brook_kqueue_set_descriptor(kq, (int) event.ident, event.filter, EV_DELETE, 0, 0, NULL);
//         if (c->state == CLOSED) {
//             brook_close_connection(c);
// 			return;
//         }
//     }
//
// 	if (r == BROOK_ERROR) {
// 		// TODO: IF RETURN SOME ERROR, NEED CREATE EVENT OF WRITE
//         // brook_kqueue_set_descriptor(kq, c->socket, EVFILT_WRITE, EV_ADD, 0, 0, c);
// 	}
//
//     if (r == BROOK_OK) {
//         if (c->state == WAITING_POOL_DB) {
//             brook_kqueue_set_descriptor(kq, c->socket, EVFILT_USER, EV_ADD, NOTE_TRIGGER, 0, c);
//         } else if (c->state == READING_PSQL_MESSAGE) {
// 			brook_kqueue_set_descriptor(kq, (int) event.ident, EVFILT_READ, EV_ADD, 0, 0, c);
//         } else if (c->state == WRITING_SOCKET_MESSAGE) {
//             brook_kqueue_set_descriptor(kq, c->socket, EVFILT_WRITE, EV_ADD, 0, 0, c);
// 		} else if (c->state == WRITING_PSQL_MESSAGE) {
// 			brook_kqueue_set_descriptor(kq, c->socket_ext, EVFILT_WRITE, EV_ADD, 0, 0, c);
// 		}
//     }
//
//     // ... if event return is BROOK_DONE need repeat the event ...

}

int
brook_kevent_read (brook_connection_t* connection, struct kevent event) {
	switch (connection->state) {
        case READING_SOCKET_MESSAGE: return brook_read_message_connection(connection);
		case READING_REDIS_MESSAGE: break;
		case READING_PSQL_MESSAGE: return brook_connection_read_psql(connection, (int) event.ident);
		default: break;
	}
    return BROOK_ERROR;
}

int
brook_kevent_write (brook_connection_t* connection, struct kevent event) {
	switch (connection->state) {
		case WRITING_SOCKET_MESSAGE: return brook_write_message_connection(connection);
		case WRITING_REDIS_MESSAGE: break;
		case WRITING_PSQL_MESSAGE: return brook_connection_write_psql(connection, (int) event.ident);
		default: break;
	}
    return BROOK_ERROR;
}

brook_connection_t*
brook_kevent_user (void) {

	if (brook_event_await == NULL) {
		return NULL;
	}

    brook_connection_t* connection = brook_event_await->c;

    if (connection->state == WAITING_POOL_DB) {
        PGconn* db = brook_postgres_get_connection(connection->conf);
        if (db) {

            brook_wait_list_t* old = brook_event_await;

            if (brook_event_await->next == NULL) {
                brook_event_await = NULL;
				brook_event_await_last = NULL;
            } else {
                brook_event_await = brook_event_await->next;
            }

            free(old);

            connection->state = WRITING_PSQL_MESSAGE;
            connection->socket_ext_type = PSQL;
            connection->socket_ext = PQsocket(db);
			return connection;
        }
    }

    return NULL;

}

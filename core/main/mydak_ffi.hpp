//
// Created by akicatt on 19.08.2026.
//

#ifndef MYDAK_BACKEND_LOCAL_SERVER_H
#define MYDAK_BACKEND_LOCAL_SERVER_H

extern "C" {

typedef void* client_handle;

client_handle client_create();
void client_destroy(client_handle handle);



void send_message(const char* message_raw);

}


#endif //MYDAK_BACKEND_LOCAL_SERVER_H

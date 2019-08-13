//
// Created by alok on 8/13/19.
//

#ifndef TEEWORLDS_AISERVER_H
#define TEEWORLDS_AISERVER_H

#include <zmq.h>
#include <iostream>


class aiserver {
    static aiserver* instance;

    void* zmq_context;
    void* game_information_sender;

    explicit aiserver(const std::string& send_port)
        : armor_collected(0), health_collected(0)
    {
        // game information sender
        zmq_context = zmq_ctx_new();
        game_information_sender = zmq_socket(zmq_context, ZMQ_PUSH);
        std::string send_address = "tcp://*:" + send_port;
        zmq_bind(game_information_sender, send_address.c_str());
    }

public:
    int armor_collected;
    int health_collected;

    static void init(const std::string& send_port) {
        instance = new aiserver(send_port);
    }

    static aiserver* get_instance() {
        return instance;
    }

    void send_update() {
        const int buffer[] = {armor_collected, health_collected};
        zmq_send(game_information_sender, buffer, sizeof(int)*2, ZMQ_DONTWAIT);
        armor_collected = 0;
        health_collected = 0;
    }

};


#endif //TEEWORLDS_AISERVER_H

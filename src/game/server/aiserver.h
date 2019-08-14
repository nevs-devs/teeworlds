//
// Created by alok on 8/13/19.
//

#ifndef TEEWORLDS_AISERVER_H
#define TEEWORLDS_AISERVER_H

#include <zmq.h>
#include <iostream>
#include <engine/shared/protocol.h>


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

        std::cout << "send port: " << send_port << std::endl;
        std::cout << "send port: " << send_port.c_str() << std::endl;

        zmq_bind(game_information_sender, send_address.c_str());
    }

public:
    int armor_collected;
    int health_collected;

    static void init(const std::string& send_port) {
        std::cout << "AI_SERVER:\n\tsend port: " << send_port << "\n" << std::endl;
        instance = new aiserver(send_port);
    }

    static aiserver* get_instance() {
        return instance;
    }

    void send_update() {
        const int buffer[] = {armor_collected, health_collected};
        if (armor_collected != 0 || health_collected != 0) {
            zmq_send(game_information_sender, buffer, sizeof(int)*2, ZMQ_DONTWAIT);

            if (armor_collected > 0) {
                std::cout << "armor collected: " << armor_collected << std::endl;
            }
            if (health_collected > 0)
                std::cout << "health collected: " << health_collected << std::endl;

            armor_collected = 0;
            health_collected = 0;
        }
    }

};


#endif //TEEWORLDS_AISERVER_H

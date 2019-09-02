//
// Created by alok on 8/13/19.
//

#ifndef TEEWORLDS_AISERVER_H
#define TEEWORLDS_AISERVER_H

#include <zmq.h>
#include <iostream>
#include <engine/shared/protocol.h>
#include <cstring>


constexpr unsigned int BUFFER_SIZE = sizeof(int)*2 + sizeof(char);


class aiserver {
    static aiserver* instance;

    void* zmq_context;
    void* game_information_sender;

    explicit aiserver(const std::string& send_port)
        : armor_collected(0), health_collected(0), died(false)
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
    bool died;

    static void init(const std::string& send_port) {
        std::cout << "AI_SERVER:\n\tsend port: " << send_port << "\n" << std::endl;
        instance = new aiserver(send_port);
    }

    static aiserver* get_instance() {
        return instance;
    }

    void fill_buffer(char* buffer) {
        int position = 0;

        memcpy(buffer + position, &armor_collected, sizeof(int));
        position += sizeof(int);

        memcpy(buffer + position, &health_collected, sizeof(int));
        position += sizeof(int);

        memcpy(buffer + position, &died, sizeof(char));
    }

    void send_update() {
        if (armor_collected || health_collected || died) {
            char buffer[BUFFER_SIZE] = {};
            fill_buffer(buffer);
            zmq_send(game_information_sender, buffer, BUFFER_SIZE, ZMQ_DONTWAIT);

            if (armor_collected)
                std::cout << "armor collected: " << armor_collected << std::endl;
            if (health_collected)
                std::cout << "health collected: " << health_collected << std::endl;
            if (died)
                std::cout << "died" << std::endl;

            armor_collected = 0;
            health_collected = 0;
            died = false;
        }
    }

};


#endif //TEEWORLDS_AISERVER_H

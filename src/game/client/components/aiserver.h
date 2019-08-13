//
// Created by alok on 7/10/19.
//

#ifndef TEEWORLDS_AISERVER_H
#define TEEWORLDS_AISERVER_H

#include <zmq.hpp>
#include <iostream>
#include <engine/client.h>


class aiserver {
private:
    static constexpr int BUFFERSIZE = 5;
    static constexpr int MOUSE_X_POS_OFFSET = 0;
    static constexpr int MOUSE_Y_POS_OFFSET = sizeof(short);
    static constexpr int ACTION_BYTE_OFFSET = sizeof(short)*2;
    static constexpr uint8_t JUMP_BITMASK = 0b00000001;
    static constexpr uint8_t HOOK_BITMASK = 0b00000010;
    static constexpr uint8_t FIRE_BITMASK = 0b00000100;
    static constexpr uint8_t RESET_BITMASK = 0b00100000;
    static constexpr uint8_t DIRECTION_BITMASK = 0b00011000;
    static constexpr uint8_t DIRECTION_LEFT = 0b00000010;
    static constexpr uint8_t DIRECTION_RIGHT = 0b00000001;

    static aiserver* instance;

    // player actions
    int8_t direction;
    short mouse_x;
    short mouse_y;
    bool jump;
    bool fire;
    bool hook;
    bool reset;

    // zmq
    void* zmq_context;
    void* actions_receiver;
    void* game_information_sender;

    IClient* client;

    explicit aiserver(const std::string& receive_port, const std::string& send_port, IClient* client)
        : direction(0), mouse_x(0), mouse_y(0), jump(false), fire(false), hook(false), reset(false), client(client)
    {
        std::string address = "tcp://localhost:" + receive_port;
        zmq_context = zmq_ctx_new();

        // action actions_receiver
        actions_receiver = zmq_socket(zmq_context, ZMQ_PULL);
        zmq_connect(actions_receiver, address.c_str());

        // game information sender
        game_information_sender = zmq_socket(zmq_context, ZMQ_PUSH);
        std::string send_address = "tcp://*:" + send_port;
        zmq_bind(game_information_sender, send_address.c_str());
    }
public:
    int armor_collected;
    int health_collected;

    static void init(const std::string& receive_port, const std::string& send_port, IClient* client) {
        std::cout << "ai server started:\n\treceive port: " << receive_port  << "\n\tsend port   : " << send_port << std::endl;
        instance = new aiserver(receive_port, send_port, client);
    }

    static aiserver* get_instance() {
        return instance;
    }

    int8_t get_direction() const {
        return direction;
    }

    bool get_hook() const {
        return hook;
    }

    bool get_fire() const {
        return fire;
    }

    bool get_jump() const {
        return jump;
    }

    short get_mouse_x() const {
        return mouse_x;
    }

    short get_mouse_y() const {
        return mouse_y;
    }

    void reset_game() const {
        if (client->RconAuthed()) {
            client->Rcon("restart");
            std::cout << "restarted" << std::endl;
        }
    }

    void apply_buffer(const uint8_t* buffer) {
        direction = 1;
        return;


        mouse_x = *reinterpret_cast<const short*>(buffer + MOUSE_X_POS_OFFSET);
        mouse_y = *reinterpret_cast<const short*>(buffer + MOUSE_Y_POS_OFFSET);

        unsigned char action_byte = *(buffer + ACTION_BYTE_OFFSET);

        jump = action_byte & JUMP_BITMASK;
        hook = action_byte & HOOK_BITMASK;
        fire = action_byte & FIRE_BITMASK;
        reset = action_byte & RESET_BITMASK;

        uint8_t direction_bits = static_cast<uint8_t>(action_byte & DIRECTION_BITMASK) >> 3u;
        if (direction_bits == DIRECTION_RIGHT) {
            direction = 1;
        } else if (direction_bits == DIRECTION_LEFT) {
            direction = -1;
        } else {
            direction = 0;
        }

        if (reset) {
            reset_game();
        }
    }

    /*
     * This updated the internal state of this aiserver by fetching all data send by the sender.
     */
    void receive_update() {
        if (!client->RconAuthed()) {
            client->RconAuth("", "123");
        }

        uint8_t buffer[BUFFERSIZE];

        unsigned int updates_received = 0;
        while (true) {
            int size = zmq_recv(actions_receiver, buffer,  BUFFERSIZE, ZMQ_DONTWAIT);
            if (size == -1) break;

            updates_received++;
            apply_buffer(buffer);
        }

        if (updates_received > 1) {
            std::cout << "WARNING: skipped " << updates_received - 1 << " updates" << std::endl;
        }


    }

    void send_update(int x_position, int y_position) {
        const int buffer[] = {x_position, y_position, armor_collected, health_collected};
        zmq_send(game_information_sender, buffer, sizeof(int)*2, ZMQ_DONTWAIT);
        armor_collected = 0;
        health_collected = 0;
    }
};


#endif //TEEWORLDS_AISERVER_H

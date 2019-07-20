//
// Created by alok on 7/10/19.
//

#ifndef TEEWORLDS_AISERVER_H
#define TEEWORLDS_AISERVER_H

#include <zmq.hpp>


class aiserver {
private:
    static constexpr int BUFFERSIZE = 5;
    static constexpr int MOUSE_X_POS_OFFSET = 0;
    static constexpr int MOUSE_Y_POS_OFFSET = sizeof(short);
    static constexpr int ACTION_BYTE_OFFSET = sizeof(short)*2;
    static constexpr uint8_t JUMP_BITMASK = 0b00000001;
    static constexpr uint8_t HOOK_BITMASK = 0b00000010;
    static constexpr uint8_t FIRE_BITMASK = 0b00000100;
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

    // zmq
    void* zmq_context;
    void* receiver;

    explicit aiserver(int port) : direction(0), mouse_x(0), mouse_y(0), jump(false), fire(false), hook(false)
    {
        std::string address = "tcp://localhost:" + std::to_string(port); // TODO
        zmq_context = zmq_ctx_new();
        receiver = zmq_socket(zmq_context, ZMQ_PULL);
        zmq_connect(receiver, address.c_str());
    }
public:
    static void init(int port) {
        instance = new aiserver(port);
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

    /*
     * This updated the internal state of this aiserver by fetching all data send by the sender.
     */
    void update() {
        uint8_t buffer[BUFFERSIZE];
        int size = zmq_recv(receiver, buffer,  BUFFERSIZE, ZMQ_DONTWAIT);
        if (size == -1)
            return;

        mouse_x = *reinterpret_cast<short*>(buffer + MOUSE_X_POS_OFFSET);
        mouse_y = *reinterpret_cast<short*>(buffer + MOUSE_Y_POS_OFFSET);

        unsigned char action_byte = *(buffer + ACTION_BYTE_OFFSET);

        jump = action_byte & JUMP_BITMASK;
        hook = action_byte & HOOK_BITMASK;
        fire = action_byte & FIRE_BITMASK;
        uint8_t direction_bits = static_cast<uint8_t>(action_byte & DIRECTION_BITMASK) >> 3u;
        if (direction_bits == DIRECTION_RIGHT) {
            direction = 1;
        } else if (direction_bits == DIRECTION_LEFT) {
            direction = -1;
        } else {
            direction = 0;
        }
    }
};


#endif //TEEWORLDS_AISERVER_H

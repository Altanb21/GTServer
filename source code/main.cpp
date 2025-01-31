﻿#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>
#include <fstream>
#include <signal.h>
#include <filesystem>

//static libraries:
#include <enet/include/enet.h>
#include <fmt/include/core.h>

#include <SFML/Graphics.hpp>

//server struct
//players:
#include <NetAvatar/NetAvatar.hpp>

//events:
#include <events/events.hpp>
#include <events/text/text_events.hpp>

//server:
#include <server/server.hpp>
#include <server/pool.hpp>

//items:
#include <database/itemdb/itemdb.hpp>
//tribute:
#include <database/player_tribute/tribute_data.hpp>

using namespace svr;
pool* g_servers;
events* g_event_manager;
std::vector<std::thread*> g_threads;

void exit_handler(int a) 
{
    for (auto& pair : g_servers->get_servers())
        g_servers->stop_instance(pair.first);
    exit(EXIT_SUCCESS);
}

int main() {
    fmt::print("starting server...\n");
    signal(SIGINT, exit_handler);

    {
        if (enet_initialize() != 0) {
            fmt::print("failed to bind ENet.\n");
            return EXIT_FAILURE;
        }
        g_servers = new pool();
        fmt::print("server pool started!\n");
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));


    fmt::print("initializing database\n"); {
        fmt::print("items.dat serialization -> {}.\n", item_database::instance().init() ? fmt::format("successed, hash: {}", item_database::instance().get_hash()) : "failed");
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    fmt::print("initializing player tribute\n"); {
        player_tribute::init();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    fmt::print("loading events...\n"); 
    {
        g_event_manager = new events();
        g_event_manager->load_events();
        fmt::print("events pushes!\n");
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));


    fmt::print("binding server...\n"); 
    {
        ENetServer* server = new ENetServer(1, "127.0.0.1", 69, 240);
        if (!server->start()) {
            fmt::print("failed to bind enet server -> {}:{}.\n", server->get_host().first, server->get_host().second);
            return EXIT_FAILURE;
        }
        server->set_component(g_event_manager);

        for (const auto& thread : g_threads)
            thread->detach();
        server->start_service();
        fmt::print("server loaded successfuly!\n");
    }
    while (true);
}
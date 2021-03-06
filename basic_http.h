// File:        basic_http.h
// Description: ---
// Notes:       ---
// Author:      Haust <wyy123_2008@qq.com>
// Revision:    2015-11-19 by Haust

#pragma once

#include "mongoose.h"
#include <map>
#include <string>
#include <functional>

class BasicHttp {
public:
	using handler = std::function<void(std::string, std::string)>;

public:
    virtual ~BasicHttp(){};

    //typedef void (*handler)(std::string query, std::string body);


    void Init(uint32_t port);
    bool Start();
    bool Close();
    bool RegisterHandler(std::string uri, handler f);
    void UnRegisterHandler(std::string uri);
    void Loop(int milli);

    void SendReply(std::string uri, std::string reply);
    void SendError(std::string uri, int errcode, std::string reply);

protected:
    using handler_map = std::map<std::string, handler>;
    // this define means that if get reply it will reply the first of the requests in the multimap which has the same uri
    // it may has a lot of other problems. so this defined can instead by uri+query_string as the map key, like this:
    // using connnection_map = std::map<std::string, mg_connection*>;
    using connection_map = std::multimap<std::string, mg_connection*>;

private:
    static void EvHandler(struct mg_connection* nc, int ev, void* ev_data);
    static void HandleRequst(struct mg_connection* nc, int ev, void *ev_data);

public:
    static handler_map _handlers;
    static connection_map _connections;

    char _port[11];
    struct mg_mgr _mgr;

};

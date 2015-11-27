// File:        basic_http.cpp
// Description: This imply file for http service with mongoose
// Notes:       ---
// Author:      Haust <wyy123_2008@qq.com>
// Revision:    2015-11-19 by Haust

#include "basic_http.h"

BasicHttp::handler_map BasicHttp::_handlers;
BasicHttp::connection_map BasicHttp::_connections;

void BasicHttp::Init(uint32_t port){
    memset(_port, 0, sizeof(_port));
    snprintf(_port, sizeof(_port), "%u", port);
}

bool BasicHttp::Start(){
    mg_mgr_init(&_mgr, NULL);
    auto nc = mg_bind(&_mgr, _port, EvHandler);

    if(nullptr == nc)
        return false;

    mg_set_protocol_http_websocket(nc);
    return true;
}

bool BasicHttp::Close(){
    mg_mgr_free(&_mgr);
    return true;
}

bool BasicHttp::RegisterHandler(std::string uri, handler f){
    auto it = _handlers.find(uri);
    if(_handlers.end() != it)
        return false;

    return _handlers.emplace(uri, f).second;
}

void BasicHttp::UnRegisterHandler(std::string uri){
    auto it = _handlers.find(uri);
    if(_handlers.end() != it)
        _handlers.erase(it);
}

void BasicHttp::Loop(int milli){
    mg_mgr_poll(&_mgr, milli);
}

void BasicHttp::EvHandler(struct mg_connection* nc, int ev, void* ev_data){
   switch(ev){
        case MG_EV_HTTP_REQUEST:
            HandleRequst(nc, ev, ev_data);
            break;
       default:
           break;
   }
}

void BasicHttp::HandleRequst(struct mg_connection *nc, int ev, void* ev_data){
    http_message* hm = (http_message*)ev_data;
    std::string uri(hm->uri.p, hm->uri.len);

    auto it = _handlers.find(uri);
    if(_handlers.end() == it)
        return;

    _connections.emplace(uri, nc);
    it->second(std::string(hm->query_string.p, hm->query_string.len),
            std::string(hm->body.p, hm->body.len));
}

void BasicHttp::SendReply(std::string uri, std::string reply){
    auto range = _connections.equal_range(uri);
    if(range.first == range.second)
        return;

    auto it = range.first;
    mg_printf(it->second, "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: text/html\r\nContent-Length: %u\r\n\r\n%s\r\n", 
            (uint32_t)reply.length(), reply.c_str());

    it->second->flags |= MG_F_SEND_AND_CLOSE;
    _connections.erase(it);
}

void BasicHttp::SendError(std::string uri, int errcode, std::string reply){
    auto range = _connections.equal_range(uri);
    if(range.first == range.second)
        return;

    auto it = range.first;
    mg_printf(it->second, "HTTP/1.1 %d %s\r\n\r\n%s\r\n", errcode, reply.c_str(), reply.c_str());

    it->second->flags |= MG_F_SEND_AND_CLOSE;
    _connections.erase(it);
}


#include "svr/3rd/mongoose.c"

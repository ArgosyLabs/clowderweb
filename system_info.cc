#include "handler.h"

#include <civetweb.h>
#include <iostream>
#include <cstring>
#include <string>
#include <lace/objectstack.h>

class system_info : public handler {
public:
    system_info(const std::string &p) : handler(p) { }

    int
    operator()(mg_connection *conn, std::ostream &os, std::istream &is) {
        std::string buf(mg_get_system_info(NULL, 0) + 1, 0);
        buf.resize(mg_get_system_info(&buf.front(), buf.length()));
        os << buf;
        return 200;
    }
};

void install(mg_context * mg, const char * prefix, const char * config) {
    static lace::objectstack os;
    os.make<system_info>(prefix)->install(mg);
}

//

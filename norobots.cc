#include "handler.h"

#include <civetweb.h>
#include <iostream>
#include <cstring>
#include <string>
#include <lace/objectstack.h>

class norobots : public handler {
public:
    norobots(const std::string &p) : handler(p) { }

    int
    operator()(mg_connection *conn, std::ostream &os, std::istream &is) {
        os << "User-agent: *" << std::endl << "Disallow: /" << std::endl;
        return 200;
    }
};

void install(mg_context * mg, const char * prefix, const char * config) {
    static lace::objectstack os;
    if (0 == strcmp("/", prefix))
        prefix = "/robots.txt";
    os.make<norobots>(prefix)->install(mg);
}

//

#ifndef HIGH_COMMAND_HANDLER
#define HIGH_COMMAND_HANDLER

#include <string>
#include <iosfwd>

extern "C" {
    struct mg_context;
    struct mg_connection;

    void install(mg_context *, const char *, const char *);
}

class handler {
public:
    handler(const std::string &p = "/") : prefix(p) { }
    virtual ~handler() { }
    virtual int operator()(mg_connection *, std::ostream &, std::istream &) = 0;

    void install(mg_context *, const std::string &path = "");
    static int wrapper(mg_connection *, void *);

protected:
    const std::string prefix;
};

#endif//HIGH_COMMAND_HANDLER

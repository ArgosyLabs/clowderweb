#include "handler.h"

#include <civetweb.h>
#include <exception>
#include <lace/ropebuf.h>
#include <iostream>
#include <streambuf>

namespace {

class consumer : public __gnu_cxx::_Rope_char_consumer<char> {
public:
    consumer(mg_connection *c) : conn(c), bytes(0) { }

    bool
    operator()(const char* buf, size_t len) {
        int b = mg_write(conn, buf, len);
        if (b > 0)
            bytes += b;
        return b > 0;
    }

    int
    apply(lace::ropebuf &buf) {
        buf.rope().apply_to_pieces(0, buf.rope().length(), *this);
        return bytes;
    }
private:
    struct mg_connection *conn;
    int bytes;
};

class buffer : public std::basic_streambuf<char, std::char_traits<char> > {
public:
    typedef std::basic_streambuf<char, std::char_traits<char> > streambuf_type;
    typedef typename streambuf_type::char_type char_type;
    typedef typename streambuf_type::traits_type traits_type;
    typedef typename streambuf_type::int_type int_type;
    typedef typename streambuf_type::pos_type pos_type;
    typedef typename streambuf_type::off_type off_type;

    buffer(mg_connection * c) : _(c), x(traits_type::eof()), len(0) {
        if (const mg_request_info *req = mg_get_request_info(_))
            len = req->content_length;
    }

    std::streamsize showmanyc() {
        std::streamsize s = len + (traits_type::eof() != x);
        return s > 0 ? s : traits_type::eof();
    }

    std::streamsize
    xsgetn(char_type *s, std::streamsize n) {
        std::streamsize ret = 0;

        if (traits_type::eof() != x) {
            *s++ = x;
            --n;
            x = traits_type::eof();
            ret += 1;
            len -= 1;
        }

        if (n > 0) {
            int r = mg_read(_, s, n);
            if (r >= 0) {
                ret += r;
                len -= r;
            } else {
                len = 0;
            }
        }

        return ret;
    }

    int_type
    underflow() {
        if (traits_type::eof() != x)
            return x;

        char c;
        if (1 == mg_read(_, &c, 1)) {
            x = c;
            --len;
        } else {
            len = 0;
        }

        return x;
    }

    int_type
    uflow() {
        if (traits_type::eof() != x) {
            char c = x;
            x = traits_type::eof();
            return c;
        }

        char c;
        if (0 < mg_read(_, &c, 1)) {
            --len;
            return c;
        } else {
            len = 0;
        }

        return traits_type::eof();
    }

    int_type
    pbackfail(int_type c = traits_type::eof()) {
        if (traits_type::eof() == x)
            return x = c;

        return traits_type::eof();
    }

private:
    mg_connection *_;
    std::streamsize len;
    int_type x;
};

}

void
handler::install(mg_context * mg, const std::string & path) {
    mg_set_request_handler(mg, (prefix + path).c_str(), &handler::wrapper, this);
}

int
handler::wrapper(mg_connection *conn, void *p) try {
    lace::ropebuf obuf;
    auto & h = *reinterpret_cast<handler*>(p);
    int rv = 0;

    {{
        buffer ibuf(conn);
        std::ostream os(&obuf);
        std::istream is(&ibuf);
        rv = h(conn, os, is);
    }}

    switch (rv) {
    case 0:
        break;

    case 200:
        mg_send_http_ok(conn, "text/plain; charset=utf-8", obuf.rope().length());
        consumer(conn).apply(obuf);
        break;

    case 301:
    case 302:
    case 303:
    case 307:
    case 308:
        mg_send_http_redirect(conn, obuf.rope().replace_with_c_str(), rv);
        break;

    case 404:
        mg_send_http_error(conn, rv, "%s", "Not found");
        break;

    default:
        throw std::runtime_error("error");
    }

    return rv;
} catch(const std::exception &e) {
    mg_send_http_error(conn, 500, "%s\n", e.what());
    return 500;
}

//

#include <civetweb.h>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <iostream>
#include <lace/haystack.h>
#include <lace/objectstack.h>
#include <lace/scoped.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <map>

void mg_atexit() { mg_exit_library(); }

int
main(int argc, char ** argv) {
    lace::haystack options;
    for (int i = 1 ; i < argc ; ++i) {
        char * arg = argv[i];

        static const char prefix[] = "--";
        if (0 != strncmp(arg, prefix, strlen(prefix)))
            continue;

        arg += strlen(prefix);
        for (const mg_option * opt = mg_get_valid_options() ; opt->name ; ++opt) {
            size_t l = strlen(opt->name);
            if (0 == strncmp(arg, opt->name, l) && 0 == strcspn(arg+l, "=")) {
                options.pointer(opt->name).pointer(arg+l+!!arg[l]);
                arg = NULL;
                break;
            }
        }

        if (arg) std::cerr << "unknown argument: " << arg << std::endl;
    }


    if (!mg_init_library(MG_FEATURES_SSL))
        return EXIT_FAILURE;

    atexit(mg_atexit);
    lace::objectstack dls;

    lace::scoped<mg_context, void, mg_stop> mg(mg_start(NULL, 0, (const char **)options.pointer(NULL).finish()));
    if (!mg)
        return EXIT_FAILURE;

    for (int i = 1 ; i < argc ; ++i) try {
        if (argv[i][0] == '-')
            continue;

        std::string prefix = "/";
        std::string config = "";

        const char * c = argv[i];

        size_t s = strcspn(c, "=#");
        std::string object(c, s);
        c += s;

        if ('=' == *c) {
            s = strcspn(c, "#");
            prefix.assign(c+1, s-1);
            c += s;
        }

        if ('?' == *c) {
            s = strcspn(c, "");
            config.assign(c+1, s-1);
            c += s;
        }

        std::cerr << "loading " << object << "=" << prefix << "#" << config << std::endl;

        void * h = dlopen(object.c_str(), RTLD_LOCAL | RTLD_LAZY);
        if (!h) throw std::runtime_error(dlerror());
        dls.make<lace::scoped<void, int, dlclose> >(h);

        typedef void (*install_t)(mg_context *, const char *, const char *);
        auto f = reinterpret_cast<install_t>(dlsym(h, "install"));
        if (!f) throw std::runtime_error(dlerror());

        f(mg, prefix.c_str(), config.c_str());
    } catch(const std::exception &e) {
        std::cerr << e.what() << std::endl;
        continue;
    }

    while (true) { }

    return EXIT_SUCCESS;
}

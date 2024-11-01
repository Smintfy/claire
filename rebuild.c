#include "include/nobu.h"

#define LIBS "lib/dotenv.c"

#ifdef _WIN32
    /* Windows is not supported */
#elif __APPLE__
    #define OPENSSL "-I/opt/homebrew/opt/openssl@3/include", "-L/opt/homebrew/opt/openssl@3/lib", "-lssl", "-lcrypto"
    #define LIBWEBSOCKETS "-I/opt/homebrew/opt/libwebsockets/include", "-L/opt/homebrew/opt/libwebsockets/lib", "-lwebsockets"
#elif __linux
    #define OPENSSL "-ssl", "-lcrypto"
    #define LIBWEBSOCKETS "-lwebsockets"
#endif
void rebuild_project()
{
#ifdef _WIN32
    LOG(NOBU_WARN, "windows is not supported\n", NULL);
#else
    const char *target_path = "main.c";
    const char *binary_path = "./main";

    LOG(NOBU_INFO, "rebuilding %s\n", target_path);
    CMD_INIT(CC, "-o", binary_path, target_path, LIBS, LIBWEBSOCKETS, OPENSSL);
    CMD_INIT(binary_path);
    exit(0);
#endif
}

int main(int argc, char **argv)
{
    AUTO_REBUILD_SELF(argc, argv);
    rebuild_project();

    return 0;
}

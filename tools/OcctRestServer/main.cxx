#include "RestServer.hxx"
#include "Logger.hxx"

int main(int argc, char** argv)
{
    // Optional: Port aus Argumenten lesen
    int port = 8080;
    if (argc > 1) {
        port = std::atoi(argv[1]);
    }
    Logger::instance().setLevel(LogLevel::DEBUG);
    return StartOcctRestServer(port);
}

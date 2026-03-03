#include "RestServer.hxx"
#include "Endpoints.hxx"
#include "SessionManager.hpp"

#define HOST_ADDRESS "127.0.0.1"

Standard_Integer StartOcctRestServer(Standard_Integer thePort)
{
    httplib::Server svr;
    SessionManager  manager;

    // Endpoints registrieren
    RegisterBBoxEndpoint(svr, manager);
    RegisterQuitEndpoint(svr, manager);
    RegisterSessionManagerEndpoints(svr, manager);

    int usePort = thePort;
    if (thePort <= 0)
    {
      usePort = svr.bind_to_any_port(HOST_ADDRESS);
    }
    else if (!svr.bind_to_port(HOST_ADDRESS, thePort))
    {
      usePort = -100000;
    }

    if (usePort > 0) {
      std::cout << "START-OK:" << usePort << std::endl;
      svr.listen_after_bind();
    }
    else if (usePort == -100000)
    {
      std::cout << "START-ERROR: Der angeforderte Port " << thePort
                << " ist nicht frei!" << std::endl;
      exit(1);
    }
    else
    {
      std::cout << "START-ERROR: Konnte keinen freien Port finden (" << usePort << ")" << std::endl;
      exit(2);
    }
    return 0;
}

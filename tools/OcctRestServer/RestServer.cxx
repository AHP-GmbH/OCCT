#include "RestServer.hxx"
#include "Endpoints.hxx"
#include "SessionManager.hpp"

Standard_Integer StartOcctRestServer(Standard_Integer thePort)
{
    httplib::Server svr;
  SessionManager  manager;

    // Endpoints registrieren
    RegisterBBoxEndpoint(svr, manager);
    // Weitere Endpoints hier hinzufügen:
    // RegisterMeshEndpoint(aServer);
    // RegisterStructureEndpoint(aServer);
    RegisterQuitEndpoint(svr, manager);
    RegisterSessionManagerEndpoints(svr, manager);

    std::cout << "OCCT REST server listening on port " << thePort << std::endl;

    // Blockiert bis Server beendet wird
    svr.listen("0.0.0.0", thePort);

    return 0;
}

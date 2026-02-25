#include "RestServer.hxx"
#include "Endpoints.hxx"

#include <httplib.h>
#include <Standard_Stream.hxx>

Standard_Integer StartOcctRestServer(Standard_Integer thePort)
{
    httplib::Server aServer;

    // Endpoints registrieren
    RegisterBBoxEndpoint(aServer);
    // Weitere Endpoints hier hinzufügen:
    // RegisterMeshEndpoint(aServer);
    // RegisterStructureEndpoint(aServer);

    std::cout << "OCCT REST server listening on port " << thePort << std::endl;

    // Blockiert bis Server beendet wird
    aServer.listen("0.0.0.0", thePort);

    return 0;
}

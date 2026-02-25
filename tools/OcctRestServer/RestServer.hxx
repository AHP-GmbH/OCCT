#pragma once

#include <Standard_Type.hxx>

// Startet den REST-Server und blockiert bis zum Shutdown.
// Gibt 0 bei Erfolg zurück.
Standard_Integer StartOcctRestServer(Standard_Integer thePort);

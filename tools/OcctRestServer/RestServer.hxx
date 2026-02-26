#pragma once

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
#endif
#include "httplib.h"

#include <Standard_Type.hxx>
#include <Standard_IStream.hxx>

// Startet den REST-Server und blockiert bis zum Shutdown.
// Gibt 0 bei Erfolg zurück.
Standard_Integer StartOcctRestServer(Standard_Integer thePort);

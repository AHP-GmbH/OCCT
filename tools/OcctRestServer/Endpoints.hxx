#pragma once

#include <httplib.h>

// Registriert den /bbox Endpoint
void RegisterBBoxEndpoint(httplib::Server& theServer);

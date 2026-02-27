#pragma once

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
#endif

#include "httplib.h"
#include "json.hpp"
#include "SessionManager.hpp"

// Registriert den /bbox Endpoint
void RegisterBBoxEndpoint(httplib::Server& svr, SessionManager& manager);
void RegisterQuitEndpoint(httplib::Server& svr, SessionManager& manager);
void RegisterSessionManagerEndpoints(httplib::Server& svr, SessionManager& manager);

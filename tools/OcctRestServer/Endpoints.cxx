#include "Endpoints.hxx"
#include "Logger.hxx"
#include <STEPControl_Reader.hxx>
#include <BRepBndLib.hxx>
#include <Bnd_Box.hxx>

using json = nlohmann::json;

// Hilfsfunktion für einheitliche Antworten
void send_json(httplib::Response& res, const json& j, int status = 200) {
    res.status = status;
    res.set_content(j.dump(), "application/json");
}

json session_json(std::shared_ptr<ManagedSession> session)
{
    // Bitte nur mit lock-guard verwenden.
  if (!session)
    return json::object(); // Sicherheitshalber
  json responseData = {{"uuid", UUID::toString(session->id)},
                       {"filename", session->filename},
                       {"status", static_cast<int>(session->status)}};

  if (session->status == SessionStatus::Error)
  {
    responseData["error_details"] = session->errorMessage;
  }

  return responseData;
}

json error_json(std::string msg) {
  json j = {{"success", false}, {"error", msg}};
  return j;
}

void RegisterBBoxEndpoint(httplib::Server& svr, SessionManager& manager)
{
    svr.Get("/bbox", [&](const httplib::Request& req, httplib::Response& res) {
        std::string rqn = "Request /bbox ";
        if (!req.has_param("id")) {
            Logger::instance().warn(rqn, "Parameter id fehlt.");
            send_json(res, error_json("ID fehlt"), 400);
            return;
        }
        auto id      = UUID::fromString(req.get_param_value("id"));
        auto session = manager.getSession(id);
        if (session)
        {
            std::lock_guard<std::mutex> lock(session->sessionMtx);
            json responseData = session_json(session);
            if (session->status != SessionStatus::Ready) {
              Logger::instance().info(rqn, "Session ", id, " noch nicht Ready (", session->status, ")");
              send_json(res, error_json("Session nicht Ready."), 400);
              return;
            }
            TopoDS_Shape aShape = session->shape;
            Bnd_Box aBox;
            BRepBndLib::Add(aShape, aBox);
            Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
            aBox.Get(xmin, ymin, zmin, xmax, ymax, zmax);
            json j = {
                {"xmin", xmin}, {"ymin", ymin}, {"zmin", zmin},
                {"xmax", xmax}, {"ymax", ymax}, {"zmax", zmax}
            };
            responseData["bbox"] = j;
            Logger::instance().debug(rqn,"OK.");
            send_json(res, {{"success", true}, {"data", responseData}});
        }
        else
        {
          Logger::instance().warn(rqn, "session ", id, " nicht gefunden.");
          send_json(res, error_json("Session nicht gefunden."), 404);
        }
      });
}

void RegisterQuitEndpoint(httplib::Server& svr, SessionManager& manager) {
  svr.Get("/quit", [](const httplib::Request& req, httplib::Response& res) {
    std::cout << "Quit Endpunkt aufgerufen, beende Server.";
    Logger::instance().info();
    send_json(res, {{"success", true}, {"message", "Quit Endpunkt aufgerufen, beende Server..."}});
    std::thread([] {
      std::this_thread::sleep_for(std::chrono::seconds(2));
      std::exit(0);
    }).detach();
  });
}

void RegisterSessionManagerEndpoints(httplib::Server& svr, SessionManager& manager) {
  svr.Get("/start", [&](const httplib::Request& req, httplib::Response& res) {
    if (!req.has_param("file")) {
        send_json(res, {{"success", false}, {"error", "Parameter 'file' fehlt"}}, 400);
        return;
    }
    std::string filename = req.get_param_value("file");
    auto        session  = manager.startSession(filename);
    if (session)
    {
      json rc = session_json(session);
      send_json(res, {{"success", true}, rc});
      return;
    }
    else
    {
      send_json(res, {{"success", false}, {"error", "Konnte Session nicht starten."}}, 400);
    }
  });

  svr.Get("/info", [&](const httplib::Request& req, httplib::Response& res) {
    if (!req.has_param("id")) {
        send_json(res, {{"success", false}, {"error", "ID fehlt"}}, 400);
        return;
    }
    auto id      = UUID::fromString(req.get_param_value("id"));
    auto session = manager.getSession(id);

    if (session)
    {
      std::lock_guard<std::mutex> lock(session->sessionMtx);
      json responseData = {
            {"uuid", UUID::toString(session->id)},
            {"filename", session->filename},
            {"status", static_cast<int>(session->status)}
      };

        if (session->status == SessionStatus::Error) {
            responseData["error_details"] = session->errorMessage;
        }

        send_json(res, {{"success", true}, {"data", responseData}});
    }
    else
    {
      send_json(res, {{"success", false}, {"error", "Session nicht gefunden"}},404);
    }
  });
}
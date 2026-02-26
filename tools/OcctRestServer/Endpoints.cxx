#include "Endpoints.hxx"

#include <STEPControl_Reader.hxx>
#include <BRepBndLib.hxx>
#include <Bnd_Box.hxx>

using json = nlohmann::json;

void RegisterBBoxEndpoint(httplib::Server& theServer)
{
    theServer.Post("/bbox", [](const httplib::Request& req, httplib::Response& res)
    {
        if (req.body.empty()) {
            res.status = 400;
            res.set_content("{\"error\":\"No file path provided\"}", "application/json");
            return;
        }

        const std::string aPath = req.body;

        STEPControl_Reader aReader;
        IFSelect_ReturnStatus aStatus = aReader.ReadFile(aPath.c_str());

        if (aStatus != IFSelect_RetDone) {
            res.status = 500;
            res.set_content("{\"error\":\"STEP read failed\"}", "application/json");
            return;
        }

        aReader.TransferRoots();
        TopoDS_Shape aShape = aReader.OneShape();

        Bnd_Box aBox;
        BRepBndLib::Add(aShape, aBox);

        Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
        aBox.Get(xmin, ymin, zmin, xmax, ymax, zmax);

        json j = {
            {"file", aPath},
            {"xmin", xmin}, {"ymin", ymin}, {"zmin", zmin},
            {"xmax", xmax}, {"ymax", ymax}, {"zmax", zmax}
        };
        res.set_content(j.dump(), "application/json");
    });
}

void RegisterQuitEndpoint(httplib::Server& theServer) {
  theServer.Get("/quit", [](const httplib::Request& req, httplib::Response& res) {
    std::cout << "Quit Endpunkt aufgerufen, beende Server.";
    json j     = {
        {"status", "OK"},
        {"message", "OCCT Server wird beendet."}
    };
    res.status = 200;
    res.set_content(j.dump(), "application/json");

    std::thread([] {
      std::this_thread::sleep_for(std::chrono::seconds(2));
      std::exit(0);
    }).detach();
  });
}

void RegisterSessionManagerEndpoints(httplib::Server& svr, SessionManager& manager) {
  svr.Get("/start", [&](const httplib::Request& req, httplib::Response& res) {
    if (!req.has_param("file"))
    {
      res.status = 400;
      res.set_content("Parameter 'file' fehlt", "text/plain");
      return;
    }

    std::string filename = req.get_param_value("file");
    auto        session  = manager.startSession(filename);

    // Liefert sofort die UUID zurück, während im Hintergrund geladen wird
    res.set_content(UUID::toString(session->id), "text/plain");
  });

  svr.Get("/info", [&](const httplib::Request& req, httplib::Response& res) {
    auto id      = UUID::fromString(req.get_param_value("id"));
    auto session = manager.getSession(id);

    if (session)
    {
      std::lock_guard<std::mutex> lock(session->sessionMtx);
      std::string                 out = "Datei: " + session->filename + "\nStatus: ";

      if (session->status == SessionStatus::Loading)
        out += "Lädt...";
      else if (session->status == SessionStatus::Ready)
        out += "Bereit";
      else
        out += "Fehler: " + session->errorMessage;

      res.set_content(out, "text/plain");
    }
    else
    {
      res.status = 404;
    }
  });
}
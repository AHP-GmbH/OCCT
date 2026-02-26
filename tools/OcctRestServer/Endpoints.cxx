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
    theServer.Get("/quit", [](const httplib::Request& req, httplib::Response& res) {
        std::cout << "Quit Endpunkt aufgerufen, beende Server.";
        json j = {
            {"message", "QUIT"}
        };
        res.status = 200;
        res.set_content(j.dump(), "application/json");
        std::thread([]{
            std::this_thread::sleep_for(std::chrono::seconds(2));
            std::exit(0);
        }).detach();
    });
}

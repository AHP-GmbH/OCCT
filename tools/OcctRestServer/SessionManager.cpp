#include "SessionManager.hpp"
#include <random>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <algorithm>

// OCCT Includes
#include <STEPControl_Reader.hxx>
#include <IGESControl_Reader.hxx>
#include <Interface_Static.hxx>

// --- UUID IMPLEMENTIERUNG ---

UUID UUID::generate()
{
  UUID                            uuid;
  std::random_device              rd;
  std::mt19937                    gen(rd());
  std::uniform_int_distribution<> dis(0, 255);

  for (auto& byte : uuid.data)
  {
    byte = static_cast<uint8_t>(dis(gen));
  }

  // Version 4 UUID Varianten setzen (RFC 4122 konform)
  uuid.data[6] = (uuid.data[6] & 0x0f) | 0x40;
  uuid.data[8] = (uuid.data[8] & 0x3f) | 0x80;

  return uuid;
}

std::string UUID::toString(const UUID& uuid)
{
  std::stringstream ss;
  ss << std::hex << std::setfill('0');
  for (size_t i = 0; i < 16; ++i)
  {
    ss << std::setw(2) << static_cast<int>(uuid.data[i]);
    // Optionale Bindestriche für Standard-Formatierung:
    if (i == 3 || i == 5 || i == 7 || i == 9)
      ss << "-";
  }
  return ss.str();
}

UUID UUID::fromString(const std::string& str)
{
  UUID        uuid;
  std::string hex = str;
  // Entferne Bindestriche, falls vorhanden
  hex.erase(std::remove(hex.begin(), hex.end(), '-'), hex.end());

  for (size_t i = 0; i < 16 && (i * 2 + 1) < hex.length(); ++i)
  {
    std::string byteString = hex.substr(i * 2, 2);
    uuid.data[i]           = static_cast<uint8_t>(std::stoul(byteString, nullptr, 16));
  }
  return uuid;
}

// --- SESSION MANAGER IMPLEMENTIERUNG ---

std::shared_ptr<ManagedSession> SessionManager::startSession(const std::string& filename)
{
  // 1. Suche: Existiert bereits eine Session mit diesem Dateinamen?
  for (auto const& [id, session] : sessions)
  {
    if (session->filename == filename)
    {
      // Gefunden! Wir geben die existierende Session zurück.
      return session;
    }
  }

  auto id      = UUID::generate();
  auto session = std::make_shared<ManagedSession>(id, filename);

  {
    std::lock_guard<std::mutex> lock(mtx);
    sessions[id] = session;
  }

  // Asynchroner Start des OCCT Ladevorgangs
  session->loadingTask = std::async(std::launch::async, [session]() { loadTask(session); });
  std::cout << "Loading file " << filename << " for session " << session->id.toString;

  return session;
}

std::shared_ptr<ManagedSession> SessionManager::getSession(const UUID& id) const
{
  std::lock_guard<std::mutex> lock(mtx);
  auto                        it = sessions.find(id);
  return (it != sessions.end()) ? it->second : nullptr;
}

bool SessionManager::stopSession(const UUID& id)
{
  std::lock_guard<std::mutex> lock(mtx);
  return sessions.erase(id) > 0;
}

// --- DER ASYNCHRONOE OCCT TASK ---

void SessionManager::loadTask(std::shared_ptr<ManagedSession> session)
{
  try
  {
    std::filesystem::path p(session->filename);
    std::string           ext = p.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    TopoDS_Shape result;
    bool         success = false;

    if (ext == ".stp" || ext == ".step")
    {
      STEPControl_Reader reader;
      if (reader.ReadFile(session->filename.c_str()) == IFSelect_RetDone)
      {
        reader.TransferRoots();
        result  = reader.OneShape();
        success = true;
      }
    }
    else if (ext == ".igs" || ext == ".iges")
    {
      IGESControl_Reader reader;
      if (reader.ReadFile(session->filename.c_str()) == IFSelect_RetDone)
      {
        reader.TransferRoots();
        result  = reader.OneShape();
        success = true;
      }
    }

    if (success && !result.IsNull())
    {
      session->update(SessionStatus::Ready, result);
    }
    else
    {
      session->update(SessionStatus::Error, {}, "Datei konnte nicht gelesen werden.");
    }
  }
  catch (const std::exception& e)
  {
    session->update(SessionStatus::Error, {}, e.what());
  }
}
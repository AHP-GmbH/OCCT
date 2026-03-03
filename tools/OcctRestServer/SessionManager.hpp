#pragma once

#include <TopoDS_Shape.hxx>
#include <string>
#include <memory>
#include <map>
#include <mutex>
#include <array>
#include <future>

// --- UUID Klasse wieder an Bord ---
struct UUID
{
  std::array<uint8_t, 16> data;

  static UUID        generate();
  static std::string toString(const UUID& uuid);
  static UUID        fromString(const std::string& str);

  // Wichtig für std::map: Der Vergleichsoperator
  bool operator<(const UUID& other) const { return data < other.data; }

  // Der "Stream-Operator" für den Logger und std::cout
  friend std::ostream& operator<<(std::ostream& os, const UUID& uuid)
  {
    os << UUID::toString(uuid);
    return os;
  }
};

// --- Zustände der Session ---
enum class SessionStatus
{
  Loading,
  Ready,
  Error
};

inline std::ostream& operator<<(std::ostream& os, const SessionStatus& status)
{
  switch (status)
  {
    case SessionStatus::Loading:
      os << "Loading";
      break;
    case SessionStatus::Ready:
      os << "Ready";
      break;
    case SessionStatus::Error:
      os << "Error";
      break;
    default:
      os << "Unknown";
      break;
  }
  return os;
}

// --- ManagedSession: Der Container für Daten & Status ---
struct ManagedSession
{
  UUID               id;
  std::string        filename;
  TopoDS_Shape       shape;
  SessionStatus      status;
  int                usageCount;
  std::string        errorMessage;
  mutable std::mutex sessionMtx; // Schützt Daten innerhalb der Session
  std::future<void>  loadingTask; // Hier parken wir den Task

  ManagedSession(const UUID& uuid, const std::string& fname)
      : id(uuid),
        filename(fname),
        status(SessionStatus::Loading)
  {
  }

  // Methode zum sicheren Aktualisieren aus dem Hintergrund-Thread
  void update(SessionStatus s, TopoDS_Shape sh, std::string err = "")
  {
    std::lock_guard<std::mutex> lock(sessionMtx);
    status       = s;
    shape        = sh;
    errorMessage = err;
  }
};

// --- SessionManager: Die Verwaltungseinheit ---
class SessionManager
{
private:
  std::map<UUID, std::shared_ptr<ManagedSession>> sessions;
  mutable std::mutex                              mtx; // Schützt die Map selbst

public:
  std::shared_ptr<ManagedSession> startSession(const std::string& filename);
  std::shared_ptr<ManagedSession> getSession(const UUID& id) const;
  bool                            stopSession(const UUID& id, const bool force);

private:
  // Der statische Task für OCCT, um "this"-Abhängigkeiten im Thread zu minimieren
  static void loadTask(std::shared_ptr<ManagedSession> session);
};
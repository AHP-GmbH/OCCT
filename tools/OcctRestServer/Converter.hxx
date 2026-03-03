#ifndef CONVERTER_HXX
#define CONVERTER_HXX

#include <Standard_Handle.hxx>

// Vorwärtsdeklarationen sparen Kompilierzeit
class TCollection_AsciiString;

/**
 * Hilfsklasse oder Namespace für die STEP-Konvertierung
 */
class StepConverter {
public:
    /**
     * Konvertiert eine STEP-Datei in eine binäre GLTF (GLB) Datei.
     * * @param stepPath   Pfad zur Quelldatei (.stp / .step)
     * @param glbPath    Pfad zur Zieldatei (.glb)
     * @param linDef     Linearer Fehlerwert (Standard: 0.1 mm)
     * @param angDef     Winkelabweichung in Radiant (Standard: 0.5 rad ≈ 28°)
     * @return           true bei Erfolg, false bei Fehlern
     */
    static bool ConvertStepToGlb(const char* stepPath, 
                                 const char* glbPath, 
                                 double linDef = 0.1, 
                                 double angDef = 0.5);
};

#endif // CONVERTER_HXX
#include "Converter.hxx"

// --- OCCT Basis & Framework ---
#include <XCAFApp_Application.hxx>
#include <TDocStd_Document.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <TDF_Label.hxx>
#include <NCollection_Sequence.hxx>
#include <NCollection_IndexedDataMap.hxx>

// --- STEP Einlesen ---
#include <STEPCAFControl_Reader.hxx>

// --- Meshing (Tessellierung) ---
#include <BRepMesh_IncrementalMesh.hxx>
#include <TopoDS_Shape.hxx>

// --- GLTF/GLB Export ---
#include <RWGltf_CafWriter.hxx>
#include <TCollection_AsciiString.hxx>
#include <Message_ProgressRange.hxx>

bool StepConverter::ConvertStepToGlb(const char* stepPath, const char* glbPath, double linDef, double angDef) {

    // 1. Dokument erstellen
    Handle(XCAFApp_Application) anApp = XCAFApp_Application::GetApplication();
    Handle(TDocStd_Document) aDoc;
    anApp->NewDocument("BinXCAF", aDoc);

    if (aDoc.IsNull()) return false;

    // 2. STEP einlesen
    STEPCAFControl_Reader reader;
    if (reader.ReadFile(stepPath) != IFSelect_RetDone) return false;
    if (!reader.Transfer(aDoc)) return false;

    // 3. Meshing mit NCollection_Sequence
    Handle(XCAFDoc_ShapeTool) shapeTool = XCAFDoc_DocumentTool::ShapeTool(aDoc->Main());
    NCollection_Sequence<TDF_Label> rootLabels;
    shapeTool->GetFreeShapes(rootLabels);

    for (int i = 1; i <= rootLabels.Length(); ++i) {
        TopoDS_Shape aShape;
        shapeTool->GetShape(rootLabels.Value(i), aShape);

        // Native bools verwenden
        BRepMesh_IncrementalMesh(aShape, linDef, false, angDef);
    }

    // 4. Export als GLB
    TCollection_AsciiString outPath(glbPath);
    RWGltf_CafWriter writer(outPath, true);

    // AKTUALISIERT: Moderne Map statt TColStd_IndexedDataMapOfStringString
    // Dies mappt TCollection_AsciiString auf TCollection_AsciiString
    NCollection_IndexedDataMap<TCollection_AsciiString, TCollection_AsciiString> aMetadata;

    return writer.Perform(aDoc, aMetadata, Message_ProgressRange());
}
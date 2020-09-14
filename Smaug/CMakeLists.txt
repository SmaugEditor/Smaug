
include_directories( . )
add_executable( smaug main.cpp smaugapp.cpp baseview.cpp 3dview.cpp uiview.cpp editview.cpp worldeditor.cpp worldrenderer.cpp utils.cpp shadermanager.cpp selectedview.cpp actionmanager.cpp baseexporter.cpp objexporter.cpp filesystem.cpp vmfexporter.cpp editoractions.cpp)
target_link_libraries( smaug bigg keyvalues )

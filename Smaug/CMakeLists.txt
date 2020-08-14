add_executable( smaug main.cpp smaugapp.cpp baseview.cpp 3dview.cpp uiview.cpp editview.cpp worldeditor.cpp worldrenderer.cpp utils.cpp shadermanager.cpp)
target_link_libraries( smaug bigg )

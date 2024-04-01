cmake_minimum_required(VERSION 3.12)

project(slicer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(VTK COMPONENTS
  CommonColor
  CommonCore
  CommonDataModel
  CommonExecutionModel
  FiltersGeneral
  FiltersSources
  IOImage
  IOGeometry
  ImagingCore
  CommonCore
  CommonDataModel
  FiltersSources
  IOImage
  ImagingStencil
  InteractionStyle
  RenderingContextOpenGL2
  RenderingCore
  RenderingFreeType
  RenderingGL2PSOpenGL2
  RenderingOpenGL2
  FiltersCore
  FiltersExtraction
  FiltersGeometry
  FiltersModeling
  FiltersSources
  CommonColor
  CommonCore
  CommonDataModel
  FiltersSources
  InteractionStyle
  InteractionWidgets
  RenderingAnnotation
  RenderingContextOpenGL2
  RenderingCore
  RenderingFreeType
  RenderingGL2PSOpenGL2
  RenderingOpenGL2
  RenderingVolumeOpenGL2
  CommonColor
   CommonCore
   CommonDataModel
   CommonTransforms
   IOImage
   ImagingCore
   InteractionImage
   InteractionStyle
   RenderingContextOpenGL2
   RenderingCore
   RenderingFreeType
   RenderingGL2PSOpenGL2
   RenderingOpenGL2
)

if (NOT VTK_FOUND)
  message(FATAL_ERROR "Unable to find the VTK build folder.")
endif()

add_executable(slicer main.cpp)
target_link_libraries(slicer PRIVATE ${VTK_LIBRARIES})

vtk_module_autoinit(
  TARGETS slicer
  MODULES ${VTK_LIBRARIES}
)

include(GNUInstallDirs)
install(TARGETS slicer
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)


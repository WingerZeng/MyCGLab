find_package(Qt5 COMPONENTS Widgets DBus REQUIRED)

option(USE_OPTIX "Use OptiX to denoise" ON)

# add configure file
configure_file(MyCGLabConfig.h.in MyCGLabConfig.h)

# add file my target
file(GLOB source_files "./*/*.cpp")
file(GLOB header_files "./*/*.h")
add_executable(MyCGLab main.cpp ${source_files} ${header_files})

# set include paths
list(APPEND INCLUDE_DIRS
	"${CMAKE_CURRENT_SOURCE_DIR}		"
	"accelerators				"
	"core					"
	"geometries				"
	"materials					"
	"primitives				"
	"raytrace					"
	"realtime					"
	"softrender				"
	"ui					"
	"${PROJECT_SOURCE_DIR}/include		"
	"${PROJECT_BINARY_DIR}				"
	"${CMAKE_CURRENT_BINARY_DIR}		")
target_include_directories(MyCGLab PRIVATE ${INCLUDE_DIRS})

# 自动执行qt moc
set_target_properties(MyCGLab PROPERTIES AUTOMOC ON)
# 自动执行qt uic
list(APPEND AUTOUIC_SEARCH_PATHS "./ui")
set_target_properties(MyCGLab PROPERTIES AUTOUIC ON)

list(APPEND EXT_LIBS Qt5::Core Qt5::Widgets Qt5::DBus)
list(APPEND EXT_LIBS glog)
list(APPEND EXT_LIBS opencv_core opencv_imgcodecs opencv_imgproc)
# 引入OptiX库
if(USE_OPTIX)
	find_package(CUDA REQUIRED)
	find_package(OptiX REQUIRED)
	if (CUDA_TOOLKIT_ROOT_DIR)
		include_directories(${CUDA_TOOLKIT_ROOT_DIR}/include)
	endif()
	include_directories(${OptiX_INCLUDE})
	target_compile_definitions(MyCGLab PRIVATE USE_OPTIX)
	list(APPEND EXT_LIBS 
		  ${OptiX_LIBRARY}
		  ${CUDA_LIBRARIES}
		  ${CUDA_CUDA_LIBRARY})
endif()

# 引入GLU库
find_package(GLU REQUIRED)
list(APPEND EXT_LIBS ${GLU_LIBRARY})

# OpenMP库
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    list(APPEND EXT_LIBS OpenMP::OpenMP_CXX)
endif()

target_link_libraries(MyCGLab PRIVATE ${EXT_LIBS})

add_custom_command(TARGET MyCGLab POST_BUILD
                   COMMAND "${CMAKE_COMMAND}" -E copy_directory "${PROJECT_SOURCE_DIR}/resources" "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/"
)

install(TARGETS MyCGLab DESTINATION bin)
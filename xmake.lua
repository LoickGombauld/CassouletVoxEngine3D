add_rules("mode.debug", "mode.release")
add_requires("glfw","glm","glad","stb")
set_allowedarchs("windows|x64")
set_warnings("allextra")

set_rundir("bin") -- Le dossier courant lors de l'exécution des binaires (depuis VS) - c'est depuis ce dossier que les chemins commencent
set_targetdir("bin/$(plat)_$(arch)_$(mode)") -- Le dossier de sortie des binaires, les $(X) sont remplacés par les valeurs existantes (plat = windows, arch = x64 et mode = debug)

set_languages("c++20")

target("CassouletVoxEngine_3D")
    set_kind("binary")
    add_files("src/*.cpp","src/**.cpp")
	add_headerfiles("src/*.hpp","src/**.hpp")
    add_packages("glfw","glm","glad","stb")
	
add_defines("GLFW_INCLUDE_NONE") 
if is_plat("windows") then 
add_syslinks("opengl32") 
elseif is_plat("linux") then 
add_syslinks("GL") 
elseif is_plat("macosx") then 
add_frameworks("OpenGL") 
end
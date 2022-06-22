workspace "vk_craft"

	newoption {
		trigger     = "build_directory",
		description = "build directory",
		default     = "build"
	}

	build_directory = path.getabsolute(_OPTIONS["build_directory"]);
    location(path.getabsolute(_OPTIONS["build_directory"]))
    targetdir (build_directory .. '/%{cfg.longname}')
    objdir (build_directory .. '/obj/')

	include("git.lua")

	git.clone_repo("https://github.com/FluentEngine/fluent", "deps/fluent")
	git.clone_repo("https://github.com/g-truc/glm.git", "deps/glm")

    configurations { "debug", "release" }

    include("deps/fluent/fluent-engine.lua")

    project "vk_craft"
        kind "WindowedApp"

		filter "configurations:debug"
			symbols "On"
			defines { "FLUENT_DEBUG" }
		filter { }

        files
        {
            "src/main.cpp",
            "src/voxel.cpp",
            "src/chunk.cpp",
            "src/chunk.hpp",
            "src/chunk_manager.cpp",
            "src/chunk_manager.hpp",
			"src/constantrs.hpp",
			"src/coordinates.hpp",
			"src/main_pass.cpp",
            "src/main_pass.hpp",
			"src/mesh.hpp",
			"src/mesh_generator.cpp",
			"src/mesh_generator.hpp",
			"src/mesh_renderer.cpp",
			"src/mesh_renderer.hpp",
			"src/quad.hpp",
			"src/raycast.hpp",
			"src/vertex.hpp",
			"src/voxel.cpp",
			"src/voxel.hpp",
			"src/ui_renderer.cpp",
			"src/ui_renderer.hpp",
            "src/shader_main_vert.cpp",
            "src/shader_main_vert.hpp",
            "src/shader_main_frag.cpp",
            "src/shader_main_frag.hpp",
            "src/shader_ui_vert.cpp",
            "src/shader_ui_vert.hpp",
            "src/shader_ui_frag.cpp",
            "src/shader_ui_frag.hpp"
        }

		-- copy data
		files
		{
			"data/ui/crafting_table.png",
			"data/ui/crosshair.png",
			"data/ui/inventory.png",
			"data/ui/text_background.png",
			"data/ui/toolbar.png",
			"data/ui/toolbar_held_item.png",
			"data/atlas.png"
		}

		filter "files:**.png"
			buildmessage 'copy resources %{file.relpath}'
			buildcommands { "{COPY} %{file.relpath} " .. "%{cfg.targetdir}/%{file.name}" }
			buildoutputs { "%{cfg.targetdir}/%{file.name}" }
		filter {}

        sysincludedirs 
        {
            "deps/fluent/sources",
            "deps/fluent/sources/third_party/",
            "deps/glm",
        }
        
        links 
        { 
            "ft_renderer", 
            "ft_os", 
            "ft_log"
        }
    
        fluent_engine.link()

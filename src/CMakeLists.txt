include_directories(. ${SDL_INCLUDE_DIR})

add_subdirectory(cdogs)
add_subdirectory(cdogsed)
add_subdirectory(json)
add_subdirectory(tests)

set(CDOGS_SDL_SOURCES
	autosave.c
	cdogs.c
	credits.c
	game.c
	handle_game_events.c
	mainmenu.c
	menu.c
	menu_utils.c
	namegen.c
	password.c
	player_select_menus.c
	prep.c
	weapon_menu.c
	XGetopt.c)
set(CDOGS_SDL_HEADERS
	autosave.h
	credits.h
	game.h
	handle_game_events.h
	mainmenu.h
	menu.h
	menu_utils.h
	namegen.h
	password.h
	player_select_menus.h
	prep.h
	weapon_menu.h
	XGetopt.h
	missions/bem.h
	missions/ogre.h)
set(CDOGS_SDL_EXTRA)
if(WIN32)
	set(CDOGS_SDL_EXTRA ../build/windows/cdogs.rc)
elseif(APPLE)
	set(CDOGS_SDL_EXTRA
		../build/macosx/cdogs-icon.icns
		../build/macosx/SDLMain.m
		../build/macosx/SDLMain.h)
	set_source_files_properties(../build/macosx/cdogs-icon.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
	set(MACOSX_BUNDLE_ICON_FILE cdogs-icon.icns)
	set_source_files_properties(../build/macosx/SDLMain.m
		PROPERTIES LANGUAGE C)
	set(EXTRA_LIBRARIES "${EXTRA_LIBRARIES} -framework Carbon -framework IOKit")
endif()
add_executable(cdogs-sdl MACOSX_BUNDLE ${CDOGS_SDL_SOURCES} ${CDOGS_SDL_HEADERS} ${CDOGS_SDL_EXTRA})
if(APPLE)
	set_target_properties(cdogs-sdl PROPERTIES
		MACOSX_RPATH 1
		BUILD_WITH_INSTALL_RPATH 1
		INSTALL_RPATH "@loader_path/../Frameworks")
endif()
target_link_libraries(cdogs-sdl
	cdogs json physfs-static
	${SDL_LIBRARY}
	${SDLIMAGE_LIBRARY}
	${SDLMIXER_LIBRARY}
	${ENet_LIBRARIES}
	${EXTRA_LIBRARIES})

add_executable(cdogs-sdl-editor MACOSX_BUNDLE cdogsed.c ${CDOGS_SDL_EXTRA})
if(APPLE)
	set_target_properties(cdogs-sdl-editor PROPERTIES
		MACOSX_RPATH 1
		BUILD_WITH_INSTALL_RPATH 1
		INSTALL_RPATH "@loader_path/../Frameworks")
endif()
target_link_libraries(cdogs-sdl-editor
	cdogsedlib cdogs json physfs-static
	${SDL_LIBRARY}
	${SDLIMAGE_LIBRARY}
	${SDLMIXER_LIBRARY}
	${ENet_LIBRARIES}
	${EXTRA_LIBRARIES})

add_executable(net_input_client net_input_client.c ${CDOGS_SDL_EXTRA})
target_link_libraries(net_input_client
	cdogs json physfs-static
	${SDL_LIBRARY}
	${SDLIMAGE_LIBRARY}
	${SDLMIXER_LIBRARY}
	${ENet_LIBRARIES}
	${EXTRA_LIBRARIES})

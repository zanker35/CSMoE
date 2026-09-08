
add_library(GameUI STATIC
	"${CMAKE_SOURCE_DIR}/src/ui/gameui/BackgroundMenuButton.cpp"
	"${CMAKE_SOURCE_DIR}/src/ui/gameui/BasePanel.cpp"
	"${CMAKE_SOURCE_DIR}/src/ui/gameui/BitmapImagePanel.cpp"
	"${CMAKE_SOURCE_DIR}/src/ui/gameui/CreateMultiplayerGameBotPage.cpp"
	"${CMAKE_SOURCE_DIR}/src/ui/gameui/CreateMultiplayerGameDialog.cpp"
	"${CMAKE_SOURCE_DIR}/src/ui/gameui/CreateMultiplayerGameGameplayPage.cpp"
	"${CMAKE_SOURCE_DIR}/src/ui/gameui/CreateMultiplayerGameServerPage.cpp"
	"${CMAKE_SOURCE_DIR}/src/ui/gameui/CvarNegateCheckButton.cpp"
	"${CMAKE_SOURCE_DIR}/src/ui/gameui/CvarSlider.cpp"
	"${CMAKE_SOURCE_DIR}/src/ui/gameui/CvarTextEntry.cpp"
	"${CMAKE_SOURCE_DIR}/src/ui/gameui/CvarToggleCheckButton.cpp"
	"${CMAKE_SOURCE_DIR}/src/ui/gameui/GameConsole.cpp"
	"${CMAKE_SOURCE_DIR}/src/ui/gameui/GameConsoleDialog.cpp"
	"${CMAKE_SOURCE_DIR}/src/ui/gameui/GameLoading.cpp"
	"${CMAKE_SOURCE_DIR}/src/ui/gameui/GameUI_Interface.cpp"
	"${CMAKE_SOURCE_DIR}/src/ui/gameui/KeyToggleCheckButton.cpp"
	"${CMAKE_SOURCE_DIR}/src/ui/gameui/LabeledCommandComboBox.cpp"
	"${CMAKE_SOURCE_DIR}/src/ui/gameui/LoadingDialog.cpp"
	"${CMAKE_SOURCE_DIR}/src/ui/gameui/ModInfo.cpp"
	"${CMAKE_SOURCE_DIR}/src/ui/gameui/MultiplayerAdvancedDialog.cpp"
	"${CMAKE_SOURCE_DIR}/src/ui/gameui/OptionsDialog.cpp"
	"${CMAKE_SOURCE_DIR}/src/ui/gameui/OptionsSubAdvanced.cpp"
	"${CMAKE_SOURCE_DIR}/src/ui/gameui/OptionsSubAudio.cpp"
	"${CMAKE_SOURCE_DIR}/src/ui/gameui/OptionsSubKeyboard.cpp"
	"${CMAKE_SOURCE_DIR}/src/ui/gameui/OptionsSubMouse.cpp"
	"${CMAKE_SOURCE_DIR}/src/ui/gameui/OptionsSubMultiplayer.cpp"
	"${CMAKE_SOURCE_DIR}/src/ui/gameui/OptionsSubVideo.cpp"
	"${CMAKE_SOURCE_DIR}/src/ui/gameui/PanelListPanel.cpp"
	"${CMAKE_SOURCE_DIR}/src/ui/gameui/ScriptObject.cpp"
	"${CMAKE_SOURCE_DIR}/src/ui/gameui/ToolBar.cpp"
	"${CMAKE_SOURCE_DIR}/src/ui/gameui/URLButton.cpp"
	"${CMAKE_SOURCE_DIR}/src/ui/gameui/VControlsListPanel.cpp"
	"${CMAKE_SOURCE_DIR}/src/ui/gameui/MoeSettings.cpp"
	"${CMAKE_SOURCE_DIR}/src/ui/gameui/OptionsSubMoeSettings.cpp"
 )

if(SE_WIN32)
	target_sources(GameUI PRIVATE
			LogoFile.cpp
			)
endif()

target_link_libraries(GameUI PUBLIC vgui_controls)

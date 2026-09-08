
add_library(cso_controls STATIC
	"${CMAKE_SOURCE_DIR}/src/ui/vgui2/cso_controls/TexturedButton.cpp"
	"${CMAKE_SOURCE_DIR}/src/ui/vgui2/cso_controls/RoundPanel.cpp"
	"${CMAKE_SOURCE_DIR}/src/ui/vgui2/cso_controls/SignalImagePanel.cpp"
)


target_link_libraries(cso_controls PUBLIC vgui_controls)

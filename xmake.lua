set_xmakever("3.0.0")

includes("lib/commonlibsf")

set_project("CinematicConversationCameraSF")
set_version("0.4.8")
set_license("GPL-3.0-or-later")
set_arch("x64")
set_languages("c++23")
set_warnings("allextra")
set_encodings("utf-8")

add_rules("mode.debug", "mode.releasedbg")

target("CinematicConversationCameraSF", function()
    add_rules("commonlibsf.plugin", {
        name = "CinematicConversationCameraSF",
        author = "Cinematic Conversation Camera SF contributors",
        description = "Cinematic shot/reverse-shot dialogue camera for Starfield"
    })

    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src")
    set_pcxxheader("src/pch.h")
end)

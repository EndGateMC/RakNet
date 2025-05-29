add_rules("mode.debug", "mode.release")

if not has_config("vs_runtime") then
    set_runtimes("MD")
end

target("RakNet")
    set_kind("static")
    set_languages("c++20")
    set_exceptions("none")
    add_includedirs("include/RakNet")
    add_files("src/**.cpp")
    add_defines(
        "NOMINMAX", 
        "UNICODE"
    )
    add_cxflags(
        "/EHa",
        "/utf-8",
        "/W4"
    )
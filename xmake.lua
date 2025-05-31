add_rules("mode.debug", "mode.release")

if is_os("windows") and not has_config("vs_runtime") then
    set_runtimes("MD")
end

target("RakNet")
    set_kind("static")
    set_languages("c++20")
    set_exceptions("none")
    add_includedirs("include/raknet")
    add_files("src/**.cpp")
    add_defines(
        "NOMINMAX",
        "UNICODE"
    )

    if is_os("windows") then
        add_cxflags(
            "/EHa",
            "/utf-8",
            "/W4",
            { force = true }
        )
    elseif is_os("linux") then
        add_cxxflags("-include cstddef")
        add_cxflags(
            "-Wall",
            "-Wextra",
            "-Wconversion",
            "-fexceptions",
            "-stdlib=libc++",
            { force = true }
        )
        add_ldflags(
            "-stdlib=libc++",
            {force = true}
        )
    end
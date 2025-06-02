add_rules("mode.debug", "mode.release")

option("libtype")
    set_default("static")
    set_values("static", "shared")
    set_showmenu(true)
option_end()

if is_os("windows") and not has_config("vs_runtime") then
    set_runtimes("MD")
end

target("RakNet")
    set_kind("$(libtype)")
    set_languages("c++20")
    set_exceptions("none")
    add_includedirs("include/raknet")
    add_files("src/**.cpp")
    if is_mode("debug") then
        set_symbols("debug")
    else
        set_optimize("aggressive")
    end

    if is_os("windows") then
        add_defines(
            "NOMINMAX",
            "UNICODE",
            "_CRT_SECURE_NO_WARNINGS",
            "_WINSOCK_DEPRECATED_NO_WARNINGS"
        )
        add_cxflags(
            "/EHa",
            "/utf-8",
            "/W4",
            { force = true }
        )
        if is_mode("release") then
            add_cxflags(
                "/O2",
                "/Ob3"
            )
        end
        if is_config("libtype", "shared") then
            add_defines("_RAKNET_EXPORT")
            add_links("ws2_32")
        end
    else
        add_cxflags(
            "-Wall",
            "-fexceptions",
            "-pedantic",
            "-Wno-pointer-bool-conversion",
            "-Wno-unused-but-set-variable",
            "-Wno-unused-private-field",
            "-stdlib=libc++",
            { force = true }
        )
        add_ldflags(
            "-stdlib=libc++",
            { force = true }
        )
        if is_mode("release") then
            add_cxflags(
                "-O3"
            )
        end
        if is_config("libtype", "shared") then
            add_defines("_RAKNET_EXPORT")
            add_cxflags(
                "-fvisibility=hidden",
                "-fvisibility-inlines-hidden",
                { force = true }
            )
        end
    end

    if is_config("libtype", "shared") then
        after_build(function (target)
            local output_dir = path.join(os.projectdir(), "bin")
            os.mkdir(output_dir)
            os.cp(target:targetfile(), path.join(output_dir, path.filename(target:targetfile())))
            cprint("${bright green}[Shared Library]: ${reset}".. path.filename(target:targetfile()) .. " already generated to " .. output_dir)
        end)
    end
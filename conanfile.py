import os
from conans import ConanFile, tools


class SkiaConan(ConanFile):
    name = "skia"
    version = "1.0.0"
    description = "Skia Graphics Library"
    settings = "os", "compiler", "build_type", "arch"
    url = "https://conan.io/"
    license = "NONE"

    # 基于https://github.com/Tereius/conan-ffmpeg仓库的修改
    requires = "ffmpeg/4.1@user/stable"

    no_copy_source = True
    exports_sources = "include/android/*", \
                      "include/atlastext/*", \
                      "include/codec/*", \
                      "include/config/*", \
                      "include/core/*", \
                      "include/docs/*", \
                      "include/effects/*", \
                      "include/encode/*", \
                      "include/gpu/*", \
                      "include/pathops/*", \
                      "include/ports/*", \
                      "include/private/*", \
                      "include/svg/*", \
                      "include/utils/*", \
                      "include/third_party/*", \
                      "modules/skottie/*.h", \
                      "modules/skresources/*.h", \
                      "src/utils/*.h", \
                      "src/core/*.h", \
                      "tools/*.h", \
                      "tools/gpu/*.h"

    skia_source_dir = None
    skia_gn = None

    def configure(self):
        self.skia_source_dir = os.getcwd()
        self.skia_gn = self.skia_source_dir + '/bin/gn --root=%s' % self.skia_source_dir

    def requirements(self):
        if self.settings.os == "Linux":
            self.requires.add("freetype/2.9.0@bincrafters/stable")

    def source(self):
        installer = tools.SystemPackageTool()

        if tools.which("python2.7") is None:
            installer.install(["python2.7", "python2", "python@2"])

        if self.settings.os == "Linux":
            installer.install(["libfontconfig1-dev", "fontconfig"])
            installer.install(["mesa-common-dev", "mesa"])
            installer.install(["libgl1-mesa-dev", "mesa"])

        # 需要正确安装depot_tools
        # 参考链接 https://skia.org/user/download
        # 没有进行python2 tools/git-sync-deps操作

    def build_configure_ninja(self):
        if self.settings.os not in ["iOS", "Macos", "Linux"]:
            raise Exception("Binary does not exist for these settings")

        # Get the absolute paths to FFmpeg include directories (list)
        ffmpeg_include_paths = self.deps_cpp_info["ffmpeg"].include_paths
        ffmpeg_include_flag = ''
        for include_path in ffmpeg_include_paths:
            ffmpeg_include_flag += "-I%s" % include_path
        print(ffmpeg_include_flag)

        # Get the directory where ffmpeg libs is
        ffmpeg_lib_paths = self.deps_cpp_info["ffmpeg"].lib_paths
        ffmpeg_lib_flag = ''
        for lib_path in ffmpeg_lib_paths:
            ffmpeg_lib_flag += "-L%s " % lib_path

        args = ''

        if self.settings.os == "iOS":
            args += 'target_os="ios" '

        if self.settings.arch == "armv7":
            args += 'target_cpu="arm" '
        elif self.settings.arch == "armv8":
            args += 'target_cpu="arm64" '
        elif self.settings.arch == "x86_64":
            args += 'target_cpu="x86_64" '

        if self.settings.build_type == "Release":
            args += 'is_debug=false '

        # The gcc compiler will treat all warnings to error.
        # So here ignore all warnings.
        if self.settings.os == "Linux":
            args += 'extra_cflags=[\"-w\", \"%s\"] ' % ffmpeg_include_flag
        else:
            args += 'extra_cflags=[\"%s\"] ' % ffmpeg_include_flag

        args += 'extra_ldflags=[\"%s\"] ' % ffmpeg_lib_flag

        if args != '':
            self.run("%s gen out --args='%s'" % (self.skia_gn, args))
        else:
            self.run("%s gen out" % self.skia_gn)

    def build(self):
        # Skia build tools requires python 2
        self.run("mkdir -p bin && ln -s %s bin/python" % tools.which("python2.7"))
        path_py2 = os.path.join(os.getcwd(), "bin") + os.pathsep + tools.get_env("PATH")
        with tools.environment_append({"PATH": path_py2}):
            self.build_configure_ninja()
            self.run("ninja -C out skia skottie modules/skottie:utils modules/skresources")

    def package(self):
        self.copy("*.h", dst="include", src="include")
        self.copy("*.h", dst="include/third_party", src="third_party")
        self.copy("*.h", dst="modules", src="modules")
        self.copy("*.h", dst="src", src="src")
        self.copy("*.h", dst="tools", src="tools")
        self.copy("*.h", dst="tools/gpu", src="tools/gpu")
        self.copy("*.dll", dst="bin", keep_path=False)
        self.copy("*.so", dst="lib", keep_path=False)
        self.copy("*.dylib", dst="lib", keep_path=False)
        self.copy("*.a", dst="lib", keep_path=False)

    def package_info(self):
        include_dirs = [
            "",
            "include/android",
            "include/atlastext",
            "include/codec",
            "include/config",
            "include/core",
            "include/docs",
            "include/effects",
            "include/encode",
            "include/gpu",
            "include/pathops",
            "include/ports",
            "include/private",
            "include/svg",
            "include/utils",
            "modules/skottie/include",
            "modules/skottie/utils",
            "modules/skresources/include",
            "tools/",
            "tools/gpu",
            "src/utils"]

        self.cpp_info.includedirs = include_dirs

        self.cpp_info.libs = tools.collect_libs(self)

        # Add system libs to the consumer project.
        if self.settings.os == "Linux":
            self.cpp_info.libs.extend(['pthread', 'dl', 'fontconfig', 'GL'])

        if self.settings.os == "iOS" or self.settings.os == "Macos":
            frameworks = ['CoreFoundation', 'CoreGraphics', 'CoreText']
            if self.settings.os == "Macos":
                frameworks.append('CoreServices')
            for framework in frameworks:
                self.cpp_info.exelinkflags.append("-framework %s" % framework)
            self.cpp_info.sharedlinkflags = self.cpp_info.exelinkflags

# vim: set expandtab ts=4 sw=4:
from conans import ConanFile, CMake, tools

class LibossiaConan(ConanFile):
    name = "libossia"
    license = "Available under both LGPLv3 and CeCILL-C"
    url = "https://opencollective.com/ossia"
    description = "A modern C++, cross-environment distributed object model for creative coding."
    topics = ("creative-coding", "osc", "open-sound-control", "ossia", "oscquery")
    settings = "os", "compiler", "build_type", "arch"
    options = {"fPIC": [True, False], "shared": [True, False]}
    default_options = {"fPIC": True, "shared": True}
    exports_sources = ["CMakeLists.txt"]
    generators = "cmake"
    requires = "boost/1.75.0"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.options.shared:
            del self.options.fPIC

    def set_version(self):
        git = tools.Git(folder=self.recipe_folder)
        tag = git.get_tag()
        if tag:
            self.version = tag.replace("/", "_")
        else:
            self.version = ("%s_%s" % (git.get_branch().replace("/", "_"), git.get_revision()))[:51]

    def export_sources(self):
        git = tools.Git(folder=self.recipe_folder)
        git.run("submodule update --init --recursive")
        self.copy("src/**")
        self.copy("cmake/**")
        self.copy("3rdparty/**")

    def configure_cmake(self):
        cmake = CMake(self)

        #submodules are updated while exporting source
        cmake.definitions["OSSIA_SUBMODULE_AUTOUPDATE"] = False

        #configurables
        cmake.definitions["OSSIA_STATIC"] = not self.options.shared

        #targets
        cmake.definitions["OSSIA_DATAFLOW"] = False
        cmake.definitions["OSSIA_EDITOR"] = False
        cmake.definitions["OSSIA_GFX"] = False
        cmake.definitions["OSSIA_JAVA"] = False
        cmake.definitions["OSSIA_PD"] = False
        cmake.definitions["OSSIA_MAX"] = False
        cmake.definitions["OSSIA_PYTHON"] = False
        cmake.definitions["OSSIA_QT"] = False
        cmake.definitions["OSSIA_C"] = False
        cmake.definitions["OSSIA_CPP"] = True
        cmake.definitions["OSSIA_UNITY3D"] = False
        cmake.definitions["OSSIA_QML"] = False
        cmake.definitions["OSSIA_QML_SCORE"] = False
        cmake.definitions["OSSIA_QML_DEVICE"] = False
        cmake.definitions["OSSIA_DISABLE_QT_PLUGIN"] = True
        cmake.definitions["OSSIA_DNSSD"] = True

        #protocols
        cmake.definitions["OSSIA_PROTOCOL_AUDIO"] = False
        cmake.definitions["OSSIA_PROTOCOL_MIDI"] = False
        cmake.definitions["OSSIA_PROTOCOL_OSC"] = True
        cmake.definitions["OSSIA_PROTOCOL_MINUIT"] = False
        cmake.definitions["OSSIA_PROTOCOL_OSCQUERY"] = True
        cmake.definitions["OSSIA_PROTOCOL_HTTP"] = False
        cmake.definitions["OSSIA_PROTOCOL_WEBSOCKETS"] = False
        cmake.definitions["OSSIA_PROTOCOL_SERIAL"] = False
        cmake.definitions["OSSIA_PROTOCOL_PHIDGETS"] = False
        cmake.definitions["OSSIA_PROTOCOL_LEAPMOTION"] = False
        cmake.definitions["OSSIA_PROTOCOL_JOYSTICK"] = False
        cmake.definitions["OSSIA_PROTOCOL_WIIMOTE"] = False
        cmake.definitions["OSSIA_PROTOCOL_ARTNET"] = False
        cmake.definitions["OSSIA_PROTOCOL_LIBMAPPER"] = False

        return cmake

    def build(self):
        cmake = self.configure_cmake()
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = self.configure_cmake()
        cmake.install()

    def package_info(self):
        self.cpp_info.libdirs = ["lib", "lib/static"]
        self.cpp_info.libs = tools.collect_libs(self)
        self.cpp_info.includedirs = ["include"]

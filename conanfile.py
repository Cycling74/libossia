# vim: set expandtab ts=4 sw=4:
from conan import ConanFile
from conan.tools.cmake import cmake_layout, CMakeToolchain, CMakeDeps, CMake
from conan.tools.scm import Git

class LibOssia(ConanFile):
    name = "libossia"
    settings = "os", "compiler", "build_type", "arch"
    license = "Available under both LGPLv3 and CeCILL-C"
    url = "https://opencollective.com/ossia"
    description = "A modern C++, cross-environment distributed object model for creative coding."
    topics = ("creative-coding", "osc", "open-sound-control", "ossia", "oscquery")

    exports_sources = "CMakeLists.txt", "src/**", "3rdparty/**", "cmake/**"

    options = {
        "shared": [True, False],

#option(OSSIA_USE_SYSTEM_LIBRARIES "Use system versions of the third-party libraries if possible")
#option(OSSIA_STATIC "Make a static build" OFF)
#option(OSSIA_INSTALL_STATIC_DEPENDENCIES "Generate install rules for wiiuse, etc" OFF)
#option(OSSIA_FAST_DEVELOPER_BUILD "Create shared libraries for some third-party libraries" OFF)
#
#option(OSSIA_COVERAGE "Run code coverage" OFF)
#option(OSSIA_EXAMPLES "Build examples" OFF)
#option(OSSIA_TESTING "Build tests" OFF)
#option(OSSIA_CI "Continuous integration run" OFF)

        "FRAMEWORK": [True, False], #"Build an OS X framework" OFF)
        "PCH": [True, False], #"Enable PCH" ON)
        "NO_SONAME": [True, False], #"Set NO_SONAME property" ON)
        "OSX_FAT_LIBRARIES": [True, False], #"Build 32 and 64 bit fat libraries on OS X" OFF)
        "OSX_RETROCOMPATIBILITY": [True, False], #"Build for older OS X versions" OFF)
        "DATAFLOW": [True, False], #"Dataflow features" ON)
        "EDITOR": [True, False], #"Editor features" ON)
        "SCENARIO_DATAFLOW": [True, False], #"Graph node support in scenario" ON)
        "GFX": [True, False], #"Graphics features" ON)
        "HIDE_ALL_SYMBOLS": [True, False], #"Hide all symbols from the ossia lib" OFF)
        "ENABLE_JACK": [True, False], #"Use JACK if available" ON)
        "ENABLE_PORTAUDIO": [True, False], #"Use PortAudio if available" ON)
        "ENABLE_PIPEWIRE": [True, False], #"Use PortAudio if available" ON)
        "ENABLE_RUBBERBAND": [True, False], #"Use RubberBand" ON)
        "ENABLE_LIBSAMPLERATE": [True, False], #"Use libsamplerate" ON)
        "ENABLE_SDL": [True, False], #"Use SDL if available" ON)
        "ENABLE_FFT": [True, False], #"Enable FFT support" OFF)
        "ENABLE_FFTW": [True, False], #"Enable FFT through FFTW" OFF)
        "ENABLE_KFR": [True, False], #"Enable KFR library" OFF)
        "DISABLE_QT_PLUGIN": [True, False], #"Disable building of a Qt plugin" OFF)
        "DNSSD": [True, False], #"Enable DNSSD support" ON)

        "JAVA": [True, False],
        "PD": [True, False],
        "MAX": [True, False],
        "PYTHON": [True, False],
        "C": [True, False],
        "CPP": [True, False],
        "UNITY3D": [True, False],
        "QT": [True, False],
        "QML": [True, False],
        "QML_SCORE": [True, False],
        "QML_DEVICE": [True, False],
        "NODEJS": [True, False],

        "PROTOCOL_AUDIO": [True, False],
        "PROTOCOL_MIDI": [True, False],
        "PROTOCOL_OSC": [True, False],
        "PROTOCOL_MINUIT": [True, False],
        "PROTOCOL_OSCQUERY": [True, False],
        "PROTOCOL_MQTT5": [True, False],
        "PROTOCOL_COAP": [True, False],
        "PROTOCOL_HTTP": [True, False],
        "PROTOCOL_WEBSOCKETS": [True, False],
        "PROTOCOL_SERIAL": [True, False],
        "PROTOCOL_PHIDGETS": [True, False],
        "PROTOCOL_JOYSTICK": [True, False],
        "PROTOCOL_WIIMOTE": [True, False],
        "PROTOCOL_ARTNET": [True, False],
        "PROTOCOL_LIBMAPPER": [True, False],
    }

    default_options = {
        "shared": True,

        "FRAMEWORK": False,
        "PCH": False,
        "NO_SONAME": False,
        "OSX_FAT_LIBRARIES": False,
        "OSX_RETROCOMPATIBILITY": False,
        "DATAFLOW": False,
        "EDITOR": False,
        "SCENARIO_DATAFLOW": False,
        "GFX": False,
        "HIDE_ALL_SYMBOLS": False,
        "ENABLE_JACK": False,
        "ENABLE_PORTAUDIO": False,
        "ENABLE_PIPEWIRE": False,
        "ENABLE_RUBBERBAND": False,
        "ENABLE_LIBSAMPLERATE": False,
        "ENABLE_SDL": False,
        "ENABLE_FFT": False,
        "ENABLE_FFTW": False,
        "ENABLE_KFR": False,
        "DISABLE_QT_PLUGIN": True,
        "DNSSD": True,

        "JAVA": False,
        "PD": False,
        "MAX": False,
        "PYTHON": False,
        "C": False,
        "CPP": True,
        "UNITY3D": False,
        "QT": False,
        "QML": False,
        "QML_SCORE": False,
        "QML_DEVICE": False,
        "NODEJS": False,

        "PROTOCOL_AUDIO": False,
        "PROTOCOL_MIDI": False,
        "PROTOCOL_OSC": True,
        "PROTOCOL_MINUIT": False,
        "PROTOCOL_OSCQUERY": True,
        "PROTOCOL_MQTT5": False,
        "PROTOCOL_COAP": False,
        "PROTOCOL_HTTP": False,
        "PROTOCOL_WEBSOCKETS": False,
        "PROTOCOL_SERIAL": False,
        "PROTOCOL_PHIDGETS": False,
        "PROTOCOL_JOYSTICK": False,
        "PROTOCOL_WIIMOTE": False,
        "PROTOCOL_ARTNET": False,
        "PROTOCOL_LIBMAPPER": False,
    }

    def set_version(self):
        git = Git(self, self.recipe_folder)
        self.version = git.run("describe --tags")

    def requirements(self):
        self.requires("boost/1.83.0") #websocketcpp requires
        self.requires("magic_enum/0.9.7")
        self.requires("re2/20251105")
        #version mismatch
        #self.requires("websocketpp/0.8.2")
        self.requires("concurrentqueue/1.0.4")
        self.requires("ctre/3.10.0")
        self.requires("dr_libs/cci.20230529")
        self.requires("fmt/12.0.0")
        self.requires("rapidfuzz/3.1.1")
        #bug: https://github.com/Tencent/rapidjson/issues/2277
        #self.requires("rapidjson/1.1.0")
        self.requires("rapidjson/cci.20230929")
        self.requires("mdspan/0.6.0")
        self.requires("tuplet/2.1.1")
        self.requires("readerwriterqueue/1.0.6")
        self.requires("spdlog/1.16.0")
        self.requires("unordered_dense/4.8.1")
        # missing nanosignal::nanosignal, smallfun, span, verdigris


        if self.options.QT:
            self.requires("qt/6.8.3")
        if self.options.PROTOCOL_COAP:
            self.requires("libcoap/4.3.3")

    def build_requirements(self):
        self.tool_requires("cmake/3.27.9")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)

        tc.cache_variables["OSSIA_USE_CONAN"] = True
        tc.cache_variables["OSSIA_STATIC"] = not bool(self.options.shared)
        tc.cache_variables["OSSIA_SUBMODULE_AUTOUPDATE"] = False

        tc.cache_variables["OSSIA_FRAMEWORK"] = bool(self.options.FRAMEWORK)
        tc.cache_variables["OSSIA_PCH"] = bool(self.options.PCH)
        tc.cache_variables["OSSIA_NO_SONAME"] = bool(self.options.NO_SONAME)
        tc.cache_variables["OSSIA_OSX_FAT_LIBRARIES"] = bool(self.options.OSX_FAT_LIBRARIES)
        tc.cache_variables["OSSIA_OSX_RETROCOMPATIBILITY"] = bool(self.options.OSX_RETROCOMPATIBILITY)
        tc.cache_variables["OSSIA_DATAFLOW"] = bool(self.options.DATAFLOW)
        tc.cache_variables["OSSIA_EDITOR"] = bool(self.options.EDITOR)
        tc.cache_variables["OSSIA_SCENARIO_DATAFLOW"] = bool(self.options.SCENARIO_DATAFLOW)
        tc.cache_variables["OSSIA_GFX"] = bool(self.options.GFX)
        tc.cache_variables["OSSIA_HIDE_ALL_SYMBOLS"] = bool(self.options.HIDE_ALL_SYMBOLS)
        tc.cache_variables["OSSIA_ENABLE_JACK"] = bool(self.options.ENABLE_JACK)
        tc.cache_variables["OSSIA_ENABLE_PORTAUDIO"] = bool(self.options.ENABLE_PORTAUDIO)
        tc.cache_variables["OSSIA_ENABLE_PIPEWIRE"] = bool(self.options.ENABLE_PIPEWIRE)
        tc.cache_variables["OSSIA_ENABLE_RUBBERBAND"] = bool(self.options.ENABLE_RUBBERBAND)
        tc.cache_variables["OSSIA_ENABLE_LIBSAMPLERATE"] = bool(self.options.ENABLE_LIBSAMPLERATE)
        tc.cache_variables["OSSIA_ENABLE_SDL"] = bool(self.options.ENABLE_SDL)
        tc.cache_variables["OSSIA_ENABLE_FFT"] = bool(self.options.ENABLE_FFT)
        tc.cache_variables["OSSIA_ENABLE_FFTW"] = bool(self.options.ENABLE_FFTW)
        tc.cache_variables["OSSIA_ENABLE_KFR"] = bool(self.options.ENABLE_KFR)
        tc.cache_variables["OSSIA_DISABLE_QT_PLUGIN"] = bool(self.options.DISABLE_QT_PLUGIN)
        tc.cache_variables["OSSIA_DNSSD"] = bool(self.options.DNSSD)

        tc.cache_variables["OSSIA_JAVA"] = bool(self.options.JAVA)
        tc.cache_variables["OSSIA_PD"] = bool(self.options.PD)
        tc.cache_variables["OSSIA_MAX"] = bool(self.options.MAX)
        tc.cache_variables["OSSIA_PYTHON"] = bool(self.options.PYTHON)
        tc.cache_variables["OSSIA_C"] = bool(self.options.C)
        tc.cache_variables["OSSIA_CPP"] = bool(self.options.CPP)
        tc.cache_variables["OSSIA_UNITY3D"] = bool(self.options.UNITY3D)
        tc.cache_variables["OSSIA_QT"] = bool(self.options.QT)
        tc.cache_variables["OSSIA_QML"] = bool(self.options.QML)
        tc.cache_variables["OSSIA_QML_SCORE"] = bool(self.options.QML_SCORE)
        tc.cache_variables["OSSIA_QML_DEVICE"] = bool(self.options.QML_DEVICE)
        tc.cache_variables["OSSIA_NODEJS"] = bool(self.options.NODEJS)

        tc.cache_variables["OSSIA_PROTOCOL_AUDIO"] = bool(self.options.PROTOCOL_AUDIO)
        tc.cache_variables["OSSIA_PROTOCOL_MIDI"] = bool(self.options.PROTOCOL_MIDI)
        tc.cache_variables["OSSIA_PROTOCOL_OSC"] = bool(self.options.PROTOCOL_OSC)
        tc.cache_variables["OSSIA_PROTOCOL_MINUIT"] = bool(self.options.PROTOCOL_MINUIT)
        tc.cache_variables["OSSIA_PROTOCOL_OSCQUERY"] = bool(self.options.PROTOCOL_OSCQUERY)
        tc.cache_variables["OSSIA_PROTOCOL_MQTT5"] = bool(self.options.PROTOCOL_MQTT5)
        tc.cache_variables["OSSIA_PROTOCOL_COAP"] = bool(self.options.PROTOCOL_COAP)
        tc.cache_variables["OSSIA_PROTOCOL_HTTP"] = bool(self.options.PROTOCOL_HTTP)
        tc.cache_variables["OSSIA_PROTOCOL_WEBSOCKETS"] = bool(self.options.PROTOCOL_WEBSOCKETS)
        tc.cache_variables["OSSIA_PROTOCOL_SERIAL"] = bool(self.options.PROTOCOL_SERIAL)
        tc.cache_variables["OSSIA_PROTOCOL_PHIDGETS"] = bool(self.options.PROTOCOL_PHIDGETS)
        tc.cache_variables["OSSIA_PROTOCOL_JOYSTICK"] = bool(self.options.PROTOCOL_JOYSTICK)
        tc.cache_variables["OSSIA_PROTOCOL_WIIMOTE"] = bool(self.options.PROTOCOL_WIIMOTE)
        tc.cache_variables["OSSIA_PROTOCOL_ARTNET"] = bool(self.options.PROTOCOL_ARTNET)
        tc.cache_variables["OSSIA_PROTOCOL_LIBMAPPER"] = bool(self.options.PROTOCOL_LIBMAPPER)

        tc.generate()

        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

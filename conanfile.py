from conan import ConanFile
from conan.tools.cmake import cmake_layout

class Recipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps", "VirtualRunEnv"

    def layout(self):
        cmake_layout(self)

    def requirements(self):
        self.requires("spdlog/[>=1.15]")
        # you probably will like these
        # self.requires("asio/1.32.0")
        # self.requires("nlohmann_json/3.11.3")
        # self.requires("raylib/5.5")

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.28]")
        self.tool_requires("ninja/[>=1.12]")
        self.test_requires("gtest/[>=1.14]")

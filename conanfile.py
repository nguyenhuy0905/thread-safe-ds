from conan import ConanFile

class Recipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps", "VirtualRunEnv"

    def layout(self):
        self.folders.generators = "conan"

    def requirements(self):
        self.requires("spdlog/1.15.0")
        # you probably will like these
        # self.requires("asio/1.32.0")
        # self.requires("nlohmann_json/3.11.3")
        # self.requires("raylib/5.5")

    def build_requirements(self):
        self.test_requires("catch2/3.8.0")

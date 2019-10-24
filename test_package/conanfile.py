import os
from conans import ConanFile, CMake, tools


class DbSQLiteAdapterTestConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake_find_package"
    options = {"boost": ["1.66.0", "1.67.0"], "gtest": ["1.7.0", "1.8.1"]}
    default_options = {"boost":"1.67.0", "gtest":"1.8.1"}

    def configure(self):
        self.options["DbSQLiteAdapter"].gtest = self.options.gtest
        self.options["DbSQLiteAdapter"].boost = self.options.boost

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def imports(self):
        self.copy("*.dll", dst=("bin/%s" % self.settings.build_type), src="bin")
        self.copy("*.dylib*", dst=("bin/%s" % self.settings.build_type), src="lib")
        self.copy('*.so*', dst=("bin/%s" % self.settings.build_type), src='lib')

    def test(self):
        cmake = CMake(self)
        cmake.test()

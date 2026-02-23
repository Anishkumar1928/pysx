import os
import subprocess
import sys
from setuptools import setup, find_packages, Extension
from setuptools.command.build_ext import build_ext

class PysxBuildExt(build_ext):
    """Custom command to build the C++ pysx compiler using g++."""
    def run(self):
        # We need g++ to compile the PYSX compiler
        try:
            subprocess.check_output(['g++', '--version'])
        except OSError:
            raise RuntimeError("g++ must be installed to build PYSX")

        source_dir = os.path.abspath("src")
        # Ensure we place the binary inside the Python package directory
        extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath("pysx.dummy")))
        
        is_windows = sys.platform == "win32"
        binary_name = "pysx_compiler.exe" if is_windows else "pysx_compiler"
        output_binary = os.path.join(extdir, binary_name)
        
        # Ensure the output directory exists
        os.makedirs(extdir, exist_ok=True)
        
        # Source files for the C++ compiler
        sources = [
            os.path.join(source_dir, "main.cpp"),
            os.path.join(source_dir, "parser.cpp"),
            os.path.join(source_dir, "lexer.cpp"),
            os.path.join(source_dir, "generator.cpp"),
            os.path.join(source_dir, "linker.cpp"),
        ]

        # Compile command
        compile_cmd = ["g++"] + sources + ["-o", output_binary]
        
        print(f"Building PYSX compiler: {' '.join(compile_cmd)}")
        subprocess.check_call(compile_cmd)

# Define a dummy extension to trigger our custom build step
pysx_ext = Extension("pysx.dummy", sources=[])

setup(
    name="pysx",
    version="0.1.0",
    description="Python-Syntax (PYSX) compiler for web applications.",
    author="Your Name",  # Update as needed
    packages=find_packages(),
    ext_modules=[pysx_ext],
    cmdclass={"build_ext": PysxBuildExt},
    entry_points={
        "console_scripts": [
            "pysx = pysx.cli:main",
            "create-pysx-app = pysx.create_app:main",
        ]
    },
    include_package_data=True,
    zip_safe=False,
)

import os
import subprocess
import sys
from setuptools import setup, find_packages, Extension
from setuptools.command.build_ext import build_ext

# Read the README.md for the PyPI project description
with open(os.path.join(os.path.dirname(__file__), "README.md"), encoding="utf-8") as f:
    long_description = f.read()

class ReactxpyBuildExt(build_ext):
    """Custom command to build the C++ reactxpy compiler using g++."""
    def run(self):
        # We need g++ to compile the PYSX compiler
        try:
            subprocess.check_output(['g++', '--version'])
        except OSError:
            raise RuntimeError("g++ must be installed to build ReactXPy")

        source_dir = os.path.abspath("src")
        # Ensure we place the binary inside the Python package directory
        extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath("reactxpy.dummy")))
        
        is_windows = sys.platform == "win32"
        binary_name = "reactxpy_compiler.exe" if is_windows else "reactxpy_compiler"
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
        
        print(f"Building ReactXPy compiler: {' '.join(compile_cmd)}")
        subprocess.check_call(compile_cmd)

# Define a dummy extension to trigger our custom build step
reactxpy_ext = Extension("reactxpy.dummy", sources=[])

setup(
    name="reactxpy",
    version="0.1.1",
    description="ReactXPy compiler for web applications.",
    long_description=long_description,
    long_description_content_type="text/markdown",
    author="Anish Kumar",
    packages=find_packages(),
    ext_modules=[reactxpy_ext],
    cmdclass={"build_ext": ReactxpyBuildExt},
    entry_points={
        "console_scripts": [
            "reactxpy = reactxpy.cli:main",
            "create-reactxpy-app = reactxpy.create_app:main",
        ]
    },
    include_package_data=True,
    zip_safe=False,
)

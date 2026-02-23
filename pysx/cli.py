import os
import sys
import subprocess

def main():
    # Handle version flags directly in the wrapper
    if len(sys.argv) > 1 and sys.argv[1] in ["-v", "--version"]:
        print("pysx version 0.1.0")
        sys.exit(0)

    # Find the compiled C++ compiler binary relative to this script
    current_dir = os.path.dirname(os.path.abspath(__file__))
    is_windows = sys.platform == "win32"
    binary_name = "pysx_compiler.exe" if is_windows else "pysx_compiler"
    compiler_path = os.path.join(current_dir, binary_name)
    
    if not os.path.exists(compiler_path):
        print(f"Error: PYSX compiler binary not found at {compiler_path}")
        print("Please ensure the package was built correctly.")
        sys.exit(1)

    # Pass all arguments directly to the C++ compiler
    args = [compiler_path] + sys.argv[1:]
    
    try:
        # Replaces the current process with the compiler
        if is_windows:
            sys.exit(subprocess.call(args))
        else:
            os.execv(compiler_path, args)
    except OSError as e:
        print(f"Failed to execute compiler: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()

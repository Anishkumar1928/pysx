import os
import subprocess
from pathlib import Path

def main():
    print("🚀 Building ReactXPy App...")
    
    os.makedirs("dist", exist_ok=True)
    compiled_files = []
    
    from shutil import which
    if which("reactxpy") is None:
        print("❌ Error: reactxpy compiler not found. Please install the reactxpy pip package globally.")
        return
        
    for file in Path("src").rglob("*.pysx"):
        basename = file.stem
        output_path = f"dist/{basename}.js"
        
        print(f"⚡ Compiling: {file} -> {output_path}")
        subprocess.run(["reactxpy", str(file), "-o", output_path], check=True)
        compiled_files.append(output_path)
        
    if compiled_files:
        print("\n✅ Success! Build complete.")
    else:
        print("❌ Error: No .pysx files found in src directory.")

if __name__ == "__main__":
    main()

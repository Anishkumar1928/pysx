#!/bin/bash

GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${BLUE}🚀 Starting PYSX Example App Build...${NC}"

# Ensure output directory exists
mkdir -p dist

# Compile all .pysx files found in the current folder
echo -e "\n📦 Compiling Source Files..."
compiled_files=()

# Using the globally installed pysx executable
COMPILER="pysx"

if ! command -v "$COMPILER" >/dev/null 2>&1; then
    echo -e "${RED}❌ Error: pysx compiler not found. Did you run 'pip install -e .' in the root directory?${NC}"
    exit 1
fi

# Find all pysx files in src directory and its subdirectories
while IFS= read -r file; do
    FILENAME=$(basename -- "$file")
    BASENAME="${FILENAME%.*}"
    OUTPUT_PATH="dist/${BASENAME}.js"
    
    echo "⚡ Compiling: $file -> $OUTPUT_PATH"
    "$COMPILER" "$file" -o "$OUTPUT_PATH"
    
    compiled_files+=("$OUTPUT_PATH")
done < <(find src -name "*.pysx" -type f)

# Link/Bundle them all together
if [ ${#compiled_files[@]} -gt 0 ]; then
    echo -e "\n🔗 Linking/Bundling into dist/bundle.js..."
    
    "$COMPILER" --bundle "${compiled_files[@]}" dist/bundle.js
    
    echo -e "\n${GREEN}✅ Success! Compiled and Linked into: dist/bundle.js${NC}"
else
    echo -e "${RED}❌ Error: No .pysx files found in src directory.${NC}"
fi

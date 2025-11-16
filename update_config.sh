#!/bin/bash
# Quick script to copy config files to build directory

echo "Copying config and asset files to build/src/..."
cp -r config/* build/src/config/
cp -r src/assets/* build/src/assets/
echo "✓ Config files updated!"
echo ""
echo "Restart the game to see changes."

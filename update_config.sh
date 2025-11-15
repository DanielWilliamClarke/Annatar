#!/bin/bash
# Quick script to copy config files to build directory

echo "Copying config files to build/src/config/..."
cp -r config/* build/src/config/
echo "✓ Config files updated!"
echo ""
echo "Restart the game to see changes."

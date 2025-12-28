#!/bin/bash
set -e

BENCHC_HOME="$(cd "$(dirname "$0")" && pwd)"
INSTALL_DIR="${HOME}/.local/bin"

mkdir -p "$INSTALL_DIR"

cat > "$INSTALL_DIR/benchc" << EOF
#!/bin/bash
exec "$BENCHC_HOME/scripts/bench.sh" "\$@"
EOF
chmod +x "$INSTALL_DIR/benchc"

cat > "$INSTALL_DIR/benchc-notebook" << EOF
#!/bin/bash
exec python3 "$BENCHC_HOME/scripts/generate_notebook.py" "\$@"
EOF
chmod +x "$INSTALL_DIR/benchc-notebook"

echo "Installed to $INSTALL_DIR"
echo "  benchc          - compile and run benchmarks"
echo "  benchc-notebook - generate notebook from csv"

if [[ ":$PATH:" != *":$INSTALL_DIR:"* ]]; then
    echo ""
    echo "Add to PATH (add to ~/.bashrc or ~/.zshrc):"
    echo "  export PATH=\"\$PATH:$INSTALL_DIR\""
fi


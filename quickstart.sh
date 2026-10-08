#!/bin/bash
# GraphOS - Quick Start Script
# Автоматическая проверка и установка необходимых инструментов

set -e

echo "╔════════════════════════════════════════════════════════════════╗"
echo "║            GraphOS Development Environment Setup               ║"
echo "╚════════════════════════════════════════════════════════════════╝"
echo ""

# Проверка операционной системы
echo "[1/5] Checking operating system..."
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    echo "  ✓ Linux detected"
    OS="linux"
elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
    echo "  ⚠ Windows detected (Git Bash/MSYS)"
    echo "  ⚠ Recommended: Use WSL2 for better compatibility"
    OS="windows"
else
    echo "  ⚠ Unknown OS: $OSTYPE"
    OS="unknown"
fi
echo ""

# Проверка необходимых инструментов
echo "[2/5] Checking required tools..."

check_tool() {
    if command -v $1 &> /dev/null; then
        VERSION=$($1 --version 2>&1 | head -n1)
        echo "  ✓ $1: $VERSION"
        return 0
    else
        echo "  ✗ $1: NOT FOUND"
        return 1
    fi
}

MISSING=0

check_tool nasm || MISSING=$((MISSING+1))
check_tool gcc || MISSING=$((MISSING+1))
check_tool ld || MISSING=$((MISSING+1))
check_tool make || MISSING=$((MISSING+1))
check_tool qemu-system-i386 || MISSING=$((MISSING+1))

echo ""

if [ $MISSING -eq 0 ]; then
    echo "  ✓ All tools are installed!"
    echo ""

    # Сборка проекта
    echo "[3/5] Building GraphOS..."
    make clean
    make all
    echo ""

    # Проверка результата
    echo "[4/5] Checking build artifacts..."
    if [ -f build/os.img ]; then
        SIZE=$(stat -f%z build/os.img 2>/dev/null || stat -c%s build/os.img 2>/dev/null)
        echo "  ✓ OS image created: build/os.img ($SIZE bytes)"

        # Проверка boot signature
        SIGNATURE=$(xxd -p -s 510 -l 2 build/os.img 2>/dev/null || echo "")
        if [ "$SIGNATURE" == "55aa" ]; then
            echo "  ✓ Boot signature valid (0x55AA)"
        else
            echo "  ✗ Boot signature invalid: 0x$SIGNATURE"
        fi
    else
        echo "  ✗ OS image not found!"
        exit 1
    fi
    echo ""

    # Запуск
    echo "[5/5] Ready to run!"
    echo ""
    echo "╔════════════════════════════════════════════════════════════════╗"
    echo "║                    BUILD SUCCESSFUL!                           ║"
    echo "╚════════════════════════════════════════════════════════════════╝"
    echo ""
    echo "To run GraphOS in QEMU:"
    echo "  make run"
    echo ""
    echo "Or manually:"
    echo "  qemu-system-i386 -drive format=raw,file=build/os.img"
    echo ""
    echo "To debug:"
    echo "  make debug"
    echo ""

else
    echo "  ✗ Missing $MISSING tool(s)"
    echo ""
    echo "[3/5] Installation required"
    echo ""

    if [ "$OS" == "linux" ]; then
        echo "To install on Ubuntu/Debian:"
        echo "  sudo apt update"
        echo "  sudo apt install -y build-essential nasm qemu-system-x86 make"
        echo ""
    elif [ "$OS" == "windows" ]; then
        echo "Recommended approach for Windows:"
        echo ""
        echo "1. Install WSL2:"
        echo "   - Open PowerShell as Administrator"
        echo "   - Run: wsl --install"
        echo "   - Restart computer"
        echo ""
        echo "2. Open Ubuntu (WSL):"
        echo "   - Run: wsl"
        echo ""
        echo "3. Install tools in WSL:"
        echo "   - sudo apt update"
        echo "   - sudo apt install -y build-essential nasm qemu-system-x86 make"
        echo ""
        echo "4. Navigate to project:"
        echo "   - cd /mnt/e/cloude"
        echo ""
        echo "5. Run this script again:"
        echo "   - ./quickstart.sh"
        echo ""
    fi

    echo "For more details, see: docs/SETUP.md"
    echo ""
    exit 1
fi

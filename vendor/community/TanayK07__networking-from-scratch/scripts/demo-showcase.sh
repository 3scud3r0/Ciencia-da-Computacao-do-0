#!/bin/bash
# Demo showcase for GIF recording
# Simulates typing + runs real demos

ROOT="/home/tanay/personal/networking-from-scratch"

type_slow() {
    local text="$1"
    for (( i=0; i<${#text}; i++ )); do
        printf '%s' "${text:$i:1}"
        sleep 0.04
    done
    echo
}

pause() { sleep "$1"; }

clear
printf '\033[1;36m'
echo "  ┌─────────────────────────────────────────────┐"
echo "  │   Networking from Scratch                    │"
echo "  │   289 lessons · 15 phases · C & Python       │"
echo "  │   Build the network stack from raw bytes     │"
echo "  └─────────────────────────────────────────────┘"
printf '\033[0m'
pause 2

echo
printf '\033[1;33m── Phase 2: Build an Ethernet Frame ──\033[0m\n'
pause 0.5
printf '\033[0;32m$ \033[0m'
type_slow "cd phases/02-link-layer/05-send-your-first-frame && make clean && make"
cd "$ROOT/phases/02-link-layer/05-send-your-first-frame"
make clean -s 2>/dev/null
make -s 2>&1
pause 1

printf '\033[0;32m$ \033[0m'
type_slow "./frame"
./frame
pause 2.5

echo
printf '\033[1;33m── Phase 1: Virtual Wire Simulator ──\033[0m\n'
pause 0.5
printf '\033[0;32m$ \033[0m'
type_slow "cd phases/01-bits-and-wires/12-capstone-a-virtual-wire && ./vwire"
cd "$ROOT/phases/01-bits-and-wires/12-capstone-a-virtual-wire"
./vwire
pause 2.5

echo
printf '\033[1;33m── Phase 6: DNS Parser (RFC 1035) ──\033[0m\n'
pause 0.5
printf '\033[0;32m$ \033[0m'
type_slow "cd phases/06-application-protocols/01-dns-message-format-rfc-1035 && ./dns_parse example.com"
cd "$ROOT/phases/06-application-protocols/01-dns-message-format-rfc-1035"
./dns_parse example.com
pause 2.5

echo
printf '\033[1;33m── Phase 6: HTTP/1.1 Parser ──\033[0m\n'
pause 0.5
printf '\033[0;32m$ \033[0m'
type_slow "cd phases/06-application-protocols/08-http10-and-http11-parser && ./http_parse"
cd "$ROOT/phases/06-application-protocols/08-http10-and-http11-parser"
./http_parse
pause 2.5

echo
printf '\033[1;36m'
echo "  ⭐ github.com/TanayK07/networking-from-scratch"
printf '\033[0m'
pause 3

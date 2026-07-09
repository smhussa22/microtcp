#!/usr/bin/env bash
set -euo pipefail

# create and configure the tun0 interface for phase 1 testing
# run this once per wsl session before running ./microtcp

DEV=${1:-tun0}

sudo ip tuntap add dev "$DEV" mode tun user "$USER"
sudo ip addr add 10.0.0.1/24 dev "$DEV"
sudo ip link set "$DEV" up

echo "TUN device '$DEV' configured:"
echo "  - kernel side: 10.0.0.1"
echo "  - your stack should claim 10.0.0.2"
echo "  - try: ping 10.0.0.2 (from another terminal while ./microtcp is running)"

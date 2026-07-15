#!/bin/bash
# Phase 5 TCP data transfer test: captures handshake + echo traffic in pcap for Wireshark inspection

set -e

PCAP_FILE="docs/captures/05-tcp-data.pcap"
mkdir -p docs/captures

echo "[*] Phase 5 test: TCP handshake + data echo"
echo "[*] Capture will be saved to: $PCAP_FILE"
echo ""

# start tcpdump in background
echo "[*] Starting packet capture..."
sudo tcpdump -i tun0 -w "$PCAP_FILE" >/dev/null 2>&1 &
TCPDUMP_PID=$!

# start the stack in background
echo "[*] Starting microtcp stack..."
sudo ./microtcp >/dev/null 2>&1 &
STACK_PID=$!

# wait for stack to initialize
sleep 1

# send test traffic via nc
echo "[*] Sending test data: 'hello from phase 5'"
echo "hello from phase 5" | nc -w 1 10.0.0.2 8080 2>/dev/null || true

# send a few more test cases
sleep 0.5
echo "test 2" | nc -w 1 10.0.0.2 8080 2>/dev/null || true

sleep 0.5
echo "final test" | nc -w 1 10.0.0.2 8080 2>/dev/null || true

# cleanup
echo "[*] Stopping capture and stack..."
sleep 1
kill $TCPDUMP_PID 2>/dev/null || true
kill $STACK_PID 2>/dev/null || true
wait $TCPDUMP_PID 2>/dev/null || true
wait $STACK_PID 2>/dev/null || true

echo ""
echo "[+] Test complete!"
echo "[+] Pcap saved to: $PCAP_FILE"
echo "[+] Open in Wireshark to inspect:"
echo "      wireshark $PCAP_FILE &"
echo ""
echo "Look for:"
echo "  - SYN → SYN-ACK → ACK (handshake)"
echo "  - PSH|ACK with payload (your data)"
echo "  - PSH|ACK with echoed payload (stack response)"
echo "  - ACK packets confirming receipt"
echo "  - Seq/ack numbers should increment by payload size"

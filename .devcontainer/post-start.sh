#!/usr/bin/env bash
set -euo pipefail

TAP_IFNAME="${SMING_HOST_TAP_IFNAME:-tap0}"
TAP_CIDR="${SMING_HOST_TAP_CIDR:-192.168.13.1/24}"

if ! command -v ip >/dev/null 2>&1; then
  echo "[post-start] 'ip' command not found; skipping TAP setup."
  exit 0
fi

if ip link show "${TAP_IFNAME}" >/dev/null 2>&1; then
  if ! ip -4 addr show dev "${TAP_IFNAME}" | grep -q "${TAP_CIDR%/*}"; then
    sudo ip address add "${TAP_CIDR}" dev "${TAP_IFNAME}" || true
  fi
  sudo ip link set "${TAP_IFNAME}" up || true
  exit 0
fi

if [ ! -e /dev/net/tun ]; then
  echo "[post-start] /dev/net/tun is unavailable. Rebuild container after updating runArgs."
  exit 0
fi

if ! sudo ip tuntap add dev "${TAP_IFNAME}" mode tap user "$(id -un)"; then
  echo "[post-start] Failed to create ${TAP_IFNAME}; verify container has NET_ADMIN and /dev/net/tun."
  exit 0
fi

sudo ip address add "${TAP_CIDR}" dev "${TAP_IFNAME}" || true
sudo ip link set "${TAP_IFNAME}" up || true
#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="/workspaces/fermentbox"
SMING_ROOT="/opt/Sming"
SMING_EXPORT_SCRIPT="${SMING_ROOT}/Tools/export.sh"
SMING_TOOLCHAINS_DIR="${SMING_TOOLCHAINS:-${HOME}/.sming/toolchains}"
MARKER_FILE="${HOME}/.sming-installed"

export SMING_TOOLCHAINS="${SMING_TOOLCHAINS_DIR}"
mkdir -p "${SMING_TOOLCHAINS}"

if [ ! -d "${SMING_ROOT}/.git" ]; then
  sudo git clone --depth 1 https://github.com/SmingHub/Sming.git "${SMING_ROOT}"
fi

sudo chown -R "$(id -u):$(id -g)" "${SMING_ROOT}"

if [ ! -f "${MARKER_FILE}" ]; then
  "${SMING_ROOT}/Tools/install.sh" esp8266 host
  touch "${MARKER_FILE}"
fi

if [ -f "${SMING_EXPORT_SCRIPT}" ] && ! grep -q "# Sming environment" "${HOME}/.bashrc"; then
  {
    echo ""
    echo "# Sming environment"
    echo "export SMING_TOOLCHAINS=${SMING_TOOLCHAINS}"
    echo "source ${SMING_EXPORT_SCRIPT}"
  } >> "${HOME}/.bashrc"
fi

if [ -f "${SMING_EXPORT_SCRIPT}" ]; then
  # shellcheck disable=SC1090
  set +u
  source "${SMING_EXPORT_SCRIPT}"
  set -u
fi

cd "${REPO_ROOT}"
npm install

cd "${REPO_ROOT}/fermentbox-frontend"
yarn install --frozen-lockfile || yarn install

#!/bin/sh

cd "${MESON_SOURCE_ROOT}"
cd ${MESON_BUILD_ROOT} && zip -r ${MESON_SOURCE_ROOT}/game.zip ./index*

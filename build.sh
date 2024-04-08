#!/bin/bash
set -e

versionMajor=0
versionMinor=1
versionPatch=0
versionTweak=0
version="${versionMajor}.${versionMinor}.${versionPatch}.${versionTweak}"

packageMD5=""
# shellcheck disable=SC2046
curDir=$(dirname $(realpath -- "$0"))
workDir=${curDir}/out
packageName="clibrary-${version}.tar.gz"

[[ -d "${workDir}" ]] && rm -rf "${workDir}"
[[ ! -d "${workDir}" ]] && mkdir -p "${workDir}"

sed -i -E "s/^set\(PROJECT_VERSION_MAJOR\ [0-9]+\)$/set\(PROJECT_VERSION_MAJOR\ ${versionMajor}\)/" \
    "${curDir}/CMakeLists.txt"
sed -i -E "s/^set\(PROJECT_VERSION_MINOR\ [0-9]+\)$/set\(PROJECT_VERSION_MINOR\ ${versionMinor}\)/" \
    "${curDir}/CMakeLists.txt"
sed -i -E "s/^set\(PROJECT_VERSION_PATCH\ [0-9]+\)$/set\(PROJECT_VERSION_PATCH\ ${versionPatch}\)/" \
    "${curDir}/CMakeLists.txt"
sed -i -E "s/^set\(PROJECT_VERSION_TWEAK\ [0-9]+\)$/set\(PROJECT_VERSION_TWEAK\ ${versionTweak}\)/" \
    "${curDir}/CMakeLists.txt"

tar cf "${packageName}" ./c ./data ./demo ./glib ./qt5 ./test ./Makefile ./CMakeLists.txt ./LICENSE ./README.md
[[ -f "./${packageName}" ]] && mv "./${packageName}" "${workDir}"
[[ -f "${workDir}/${packageName}" ]] && packageMD5=$(sha512sum "${workDir}/${packageName}" | awk '{print $1}')


cat << EOF > ${workDir}/PKGBUILD
# Maintainer: dingjing <dingjing@live.cn>

pkgname=clibrary
pkgver=${versionMajor}.${versionMinor}.${versionPatch}
pkgrel=${versionTweak}
pkgdesc='c library.'
url='https://github.com/dingjingmaster/clibrary'
arch=('x86_64' 'arm')
license=('MIT')
depends=(
        breeze-icons
    )
optdepends=(
    )
makedepends=(
        'cmake' 'gcc' 'qt5-base'
    )
source=("${workDir}/${packageName}")

sha512sums=("${packageMD5}")
noextract=()
validpgpkeys=('18B65970A361B77D6C7C67C29EE375D12E7A3EB1')

prepare() {
  cd \$srcdir
}

build() {
  make
}

package() {
  make DESTDIR="\$pkgdir" install
}

EOF

cd "${workDir}"
makepkg --printsrcinfo > .SRCINFO
makepkg
cd "${curDir}"

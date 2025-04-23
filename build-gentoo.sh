#!/bin/bash
set -e

versionMajor=0
versionMinor=2
versionPatch=0
version="${versionMajor}.${versionMinor}.${versionPatch}"

packageMD5=""
# shellcheck disable=SC2046
curDir=$(dirname $(realpath -- "$0"))
workDir=${curDir}/out
packageName="clibrary-${version}.tar.gz"

[[ -d "${workDir}" ]] && rm -rf "${workDir}"
[[ ! -d "${workDir}" ]] && mkdir -p "${workDir}"
[[ -d ${workDir}/dev-libs ]] && rm -rf "${workDir}/dev-libs"
[[ ! -d ${workDir}/dev-libs ]] && mkdir -p "${workDir}/dev-libs"

sed -i -E "s/^set\(PROJECT_VERSION_MAJOR\ [0-9]+\)$/set\(PROJECT_VERSION_MAJOR\ ${versionMajor}\)/" \
    "${curDir}/CMakeLists.txt"
sed -i -E "s/^set\(PROJECT_VERSION_MINOR\ [0-9]+\)$/set\(PROJECT_VERSION_MINOR\ ${versionMinor}\)/" \
    "${curDir}/CMakeLists.txt"
sed -i -E "s/^set\(PROJECT_VERSION_PATCH\ [0-9]+\)$/set\(PROJECT_VERSION_PATCH\ ${versionPatch}\)/" \
    "${curDir}/CMakeLists.txt"
sed -i -E "s/^set\(PROJECT_VERSION_TWEAK\ [0-9]+\)$/set\(PROJECT_VERSION_TWEAK\ ${versionTweak}\)/" \
    "${curDir}/CMakeLists.txt"

tar cf "${packageName}" ./c ./data ./demo ./test ./Makefile ./CMakeLists.txt ./LICENSE ./README.md
[[ -f "./${packageName}" ]] && mv "./${packageName}" "${workDir}"
[[ -f "${workDir}/${packageName}" ]] && packageMD5=$(sha512sum "${workDir}/${packageName}" | awk '{print $1}')

cat << EOF > ${workDir}/dev-libs/clibrary-${version}.ebuild
# Maintainer: dingjing <dingjing@live.cn>

EAPI=8

DESCRIPTION="clibrary"
HOMEPAGE="https://github.com/dingjingmaster/clibrary"

LICENSE="MIT"
SLOT="0"
KEYWORDS="~amd64"

DEPEND=""
RDEPEND="${DEPEND}"
BDEPEND=""

export BASEDIR="`pwd`/.."
export WORKDIR="${BASEDIR}/work"
export BUILDDIR="${WORKDIR}/build/"
export SRCDIR="${WORKDIR}/data-analysis-${PV}"

pkg_pretend() {
        elog "BASEDIR   :       ${BASEDIR}"
        elog "BUILDDIR  :       ${BUILDDIR}"
        elog "SRCDIR    :       ${SRCDIR}"
        elog "Pretent ..."
}

pkg_setup() {
        elog "Setup ..."
}

src_unpack() {
        elog "Unpacking ..."
        default
        mv "${WORKDIR}/clibrary-${PV}" "${WORKDIR}/clibrary-${PV}"
}

src_prepare() {
        elog "Prepare ..."
        eapply_user
}

src_configure() {
        elog "Configure ..."
        cd "${WORKDIR}/clibrary-${PV}" 
        mkdir -p ${BUILDDIR}
        cmake -B "${BUILDDIR}" "${SRCDIR}" 
}

src_compile() {
        elog "Start compile ..."
        make -C "${BUILDDIR}" -j$(nproc)
}

src_test() {
        elog "Start test ..."
}

src_install() {
        elog "Start Install ..."
#        install -Dm 0555 "${BUILDDIR}/app/data-analysis" "${D}${bindir}/bin/data-analysis"
}

pkg_preinst() {
        elog "Preinstall ..."
}

pkg_postinst() {
        elog "Postinstall ..."
}

EOF

cd "${workDir}"
cd "${curDir}"


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
[[ ! -d ${workDir}/metadata ]] && mkdir -p "${workDir}/metadata"
[[ ! -d ${workDir}/profiled ]] && mkdir -p "${workDir}/profiles"
[[ ! -d ${workDir}/dev-libs/clibrary ]] && mkdir -p "${workDir}/dev-libs/clibrary"

sed -i -E "s/^set\(PROJECT_VERSION_MAJOR\ [0-9]+\)$/set\(PROJECT_VERSION_MAJOR\ ${versionMajor}\)/" \
    "${curDir}/CMakeLists.txt"
sed -i -E "s/^set\(PROJECT_VERSION_MINOR\ [0-9]+\)$/set\(PROJECT_VERSION_MINOR\ ${versionMinor}\)/" \
    "${curDir}/CMakeLists.txt"
sed -i -E "s/^set\(PROJECT_VERSION_PATCH\ [0-9]+\)$/set\(PROJECT_VERSION_PATCH\ ${versionPatch}\)/" \
    "${curDir}/CMakeLists.txt"

tar zcf "${packageName}" ./c ./data ./demo ./test ./Makefile ./CMakeLists.txt ./LICENSE ./README.md
[[ -f "./${packageName}" ]] && mv "./${packageName}" "${workDir}"
[[ -f "${workDir}/${packageName}" ]] && packageMD5=$(sha512sum "${workDir}/${packageName}" | awk '{print $1}')
#/var/tmp/portage/dev-libs/clibrary-0.2.0/distdir/clibrary-0.2.0.tar.gz
#[[ ! -f "/var/tmp/portage" ]]
sudo cp ${workDir}/${packageName} /var/cache/distfiles/

cat << EOF > ${workDir}/metadata/layout.conf
masters = gentoo
EOF

cat << EOF > ${workDir}/profiles/eapi
8
EOF

cat << EOF > ${workDir}/profiles/repo_name
local-gentoo-clibrary
EOF

cat << EOF > ${workDir}/dev-libs/clibrary/clibrary-${version}.ebuild
# Maintainer: dingjing <dingjing@live.cn>

EAPI=8

IUSE="test"
RESTRICT="fetch"
DESCRIPTION="clibrary"
HOMEPAGE="https://github.com/dingjingmaster/clibrary"
SRC_URI="${packageName}"

LICENSE="MIT"
SLOT="0"
KEYWORDS="~amd64"

# 编译时需要的依赖
DEPEND=""
# 运行时需要的依赖
RDEPEND=""
# 构建工具依赖
BDEPEND="
	dev-build/cmake
	virtual/pkgconfig
"
# 安装后推荐依赖
PDEPEND=""

export BASEDIR="\`pwd\`/.."
export WORKDIR="\${BASEDIR}/work"
export BUILDDIR="\${WORKDIR}/build/"
export SRCDIR="\${WORKDIR}/clibrary-\${PV}"

#CMAKE_MAKEFILE_GENERATOR="makefiles"
#inherit cmake

pkg_pretend() {
        elog "BASEDIR   :       \${BASEDIR}"
        elog "BUILDDIR  :       \${BUILDDIR}"
        elog "SRCDIR    :       \${SRCDIR}"
        elog "Pretent ..."
}

pkg_setup() {
        elog "Setup ..."
}

src_unpack() {
        elog "Unpacking ..."
        default
		elog "Move from \${WORKDIR}/clibrary-\${PV} To \${WORKDIR}/clibrary-\${PV}"
		mv \${WORKDIR} \${WORKDIR}.bak
		[[ ! -d "\${WORKDIR}/" ]] && mkdir -p "\${WORKDIR}/"
		mv \${WORKDIR}.bak \${WORKDIR}/../clibrary-\${PV}
		mv \${WORKDIR}/../clibrary-\${PV} \${WORKDIR}/clibrary-\${PV}
}

src_prepare() {
        elog "Prepare ..."
        eapply_user
}

src_configure() {
        elog "Configure ..."
        cd "\${WORKDIR}/" 
        mkdir -p \${BUILDDIR}
        cmake -DCMAKE_INSTALL_PREFIX=/usr/ -B "\${BUILDDIR}" "\${SRCDIR}" 
}

src_compile() {
        elog "Start compile ..."
        make -C "\${BUILDDIR}" -j\$(nproc)
}

src_test() {
        elog "Start test ..."
		make -C "\${BUILDDIR}" test || die "test error!"
}

src_install() {
        elog "Start Install ..."
		cd "\${BUILDDIR}/"
		DESTDIR="\${D}" make -C \${BUILDDIR} install
}

pkg_preinst() {
        elog "Preinstall ..."
}

pkg_postinst() {
        elog "Postinstall ..."
}

EOF

cd "${workDir}"
ebuild dev-libs/clibrary/clibrary-${version}.ebuild digest
#sudo ebuild dev-libs/clibrary/clibrary-${version}.ebuild package
sudo ebuild dev-libs/clibrary/clibrary-${version}.ebuild merge
cd "${curDir}"


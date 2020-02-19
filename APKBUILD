pkgname=jshu
pkgver=1.0.0
pkgrel=20
pkgdesc="Simplified unit test framework for shell script which produces junit-style xml results file"
url="https://github.com/AdrianDC/jshu"
arch="noarch"
license="BSD"
depends=""
depends_dev=""
makedepends="$depends_dev"
install=""
maintainer="Adrian DC <radian.dc@gmail.com>"
options="!fhs"
srcdir="$startdir/build"
source="
  jshutest.inc
  wrapper.sh
  "
builddir="$startdir/build"
pkgdir="$startdir/build/pkg"

prepare() {
  default_prepare
}

build() {
  cd "$builddir"
}

check() {
  cd "$builddir"
}

package() {
  cd "$builddir"
  mkdir -p "$pkgdir/opt/jshu/"
  install ./jshutest.inc "$pkgdir/opt/jshu/"
  install ./wrapper.sh "$pkgdir/opt/jshu"
}

sha512sums="f3e3f9d0b50dece8d28bb97160f27004e112eab14e469ad0ff43b4ca61624b77badb71292353271bc209e87dc8e97e8cbbc9dd70d13115aa4c36a1a7fbf13777  jshutest.inc
8168d5a1b29eb0810278ce45f6f786b3211a654cbf27195f66a237f560e1b0fc8c80f752e8f326ebe7a97bab872c15090d153196296da5f44ce975c7ed2ed38e  wrapper.sh"

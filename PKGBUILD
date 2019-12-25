# Contributor: ivanp7
# Maintainer: ivanp7

_pkgname=surf
pkgname=$_pkgname-ivanp7-git
pkgver=2.0
pkgrel=1
pkgdesc="a WebKit based browser"
arch=('i686' 'x86_64')
url="http://surf.suckless.org/"
license=('custom:MIT/X')
depends=('webkit2gtk' 'gcr' 'xorg-xprop')
makedepends=('git')
optdepends=('dmenu: url bar and search'
            'tabbed: tab browsing'
            'ca-certificates: SSL verification'
            'st: default terminal for the download handler'
            'curl: default download handler'
            'mpv: default video player')
provides=("$_pkgname")
conflicts=("$_pkgname")
source=('arg.h' 'config.h' 'config.mk' 'LICENSE' 'Makefile' 'surf.1' 'surf.c')
md5sums=('4a06cd509ed479450e36833c6ce2015b' 'SKIP' 
         '4099620f9c40bf1cf10d1f14c45f528c' 'b57e7f7720307a02d5a6598b00fe3afa' 
         '163d841abdc5d65e05a506ea68087e13' '3cfd590a7f85f3ed8d55c43451ea58ba' 
         'SKIP')

build() {
  make PREFIX=/usr DESTDIR="$pkgdir"
}

package() {
  make PREFIX=/usr DESTDIR="$pkgdir" install
  install -Dm644 LICENSE "$pkgdir/usr/share/licenses/$pkgname/LICENSE"
}

# vim:set ts=2 sw=2 et:

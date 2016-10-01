#!/bin/sh

VERSION='20161001003'

if [ -f "/etc/squidanalyzer.version" ]; then
	if [ "$(cat /etc/squidanalyzer.version)" = "$VERSION" ]; then
		echo "ERROR: Changes have been applied!"
		exit 2
	fi
fi

PF_VERSION="$(cat /etc/version | cut -f 1,2 -d '.')"

if [ ! "$PF_VERSION" == "2.3" ]; then
	echo "Not working with this version"
	exit 2
fi


URL='https://pkg.mundounix.com.br/pfsense/squidanalyzer/'

if [ ! "$(/usr/sbin/pkg info | grep squidanalyzer-6.5)" ]; then
	ASSUME_ALWAYS_YES=YES
	export ASSUME_ALWAYS_YES
	/usr/sbin/pkg add https://pkg.mundounix.com.br/pfsense/packages/amd64/All/squidanalyzer-6.5.txz
fi

fetch -q -o /usr/local/pkg/ $URL/squidanalyzer.inc
fetch -q -o /usr/local/pkg/ $URL/squidanalyzer.xml

/usr/local/sbin/pfSsh.php <<EOF
\$sa = false;
foreach (\$config['installedpackages']['menu'] as \$item) {
  if ('SquidAnalyzer' == \$item['name']) {
    \$sa = true;
    break;
  }
}
if (\$sa == false) {
  \$config['installedpackages']['menu'][] = array(
    'name' => 'SquidAnalyzer',
    'section' => 'Services',
    'url' => '/pkg_edit.php?xml=squidanalyzer.xml'
  );
}
write_config();
exec;
exit
EOF


if [ "$PF_VERSION" == "2.3" ]; then
	if [ ! "$(/usr/sbin/pkg info | grep pfSense-pkg-Squid)" ]; then
		ASSUME_ALWAYS_YES=YES
		export ASSUME_ALWAYS_YES
		/usr/sbin/pkg install -r pfSense pfSense-pkg-Squid
	fi
fi

echo "$VERSION" > /etc/squidanalyzer.version

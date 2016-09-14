#!/bin/sh

VERSION='20160913009'

if [ -f "/etc/squidanalyzer.version" ]; then
	if [ "$(cat /etc/squidanalyzer.version)" = "$VERSION" ]; then
		echo "ERROR: Changes have been applied!"
		exit 2
	fi
fi

PF_VERSION="$(cat /etc/version | cut -f 1,2 -d '.')"


URL='https://pkg.mundounix.com.br/pfsense/squidanalyzer/'

fetch -q -o /usr/local/pkg $URL/squidanalyzer.inc
fetch -q -o /usr/local/pkg $URL/squidanalyzer.xml

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

if [ "$PF_VERSION" == "2.2" ]; then
	if [ ! "$(/usr/local/sbin/pfSsh.php playback listpkg | grep 'Squid3')" ]; then
	        /usr/local/sbin/pfSsh.php playback installpkg "Squid3"
	fi
fi

if [ "$PF_VERSION" == "2.3" ]; then
	if [ ! "$(/usr/sbin/pkg info | grep pfSense-pkg-Squid)" ]; then
		ASSUME_ALWAYS_YES=YES
		export ASSUME_ALWAYS_YES
		/usr/sbin/pkg install -r pfSense pfSense-pkg-Squid
	fi
fi

echo "$VERSION" > /etc/squidanalyzer.version

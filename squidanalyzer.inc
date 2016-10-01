<?php
# ------------------------------------------------------------------------------
/*  squidanalyzer.inc
    Copyright (C) 2016 Luiz Gustavo <me@luizgustavo.pro.br>
    All rights reserved.
*/
# ------------------------------------------------------------------------------

require_once('globals.inc');
require_once('config.inc');
require_once('util.inc');
require_once('pfsense-utils.inc');
require_once('pkg-utils.inc');
require_once('filter.inc');
require_once('service-utils.inc');


define('SACONFIG', '/usr/local/etc/squidreport/squidanalyzer.conf');
define('CRONTAB_FILE', '/var/cron/tabs/root');

function validate_form_sa($post, $input_errors) {
  if (isset($post['saenable'])) {
    if ( ! is_numeric($post['topnumber']) ) {
        $input_errors[] = 'The field "Top Number" must be a numeric value';
    }

    if ( ! is_numeric($post['topurluser']) ) {
        $input_errors[] = 'The field "Top URL User" must be a numeric value';
    }
  }
}

function sync_squidanalyzer() {

  global $g, $config;

  if ($_POST['Submit'] == 'Force update now') {
      if ( file_exists(SACONFIG) ) {
        mwexec("/usr/local/bin/squid-analyzer -c " . SACONFIG);
      }
  }

  if (($_POST['Submit'] == 'Save') || !isset($_POST['Submit'])) {
    $settings = $config['installedpackages']['squidanalyzer']['config'][0];
    $SquidLog = $settings['squidlog'];
    $sa_lang = $settings['lang'];
    $sa_dateformat = $settings['dateformat'];
    $sa_topurluser = $settings['topurluser'];
    $sa_locale = $settings['locale'];
    $sa_useclientdns = ($settings['useclientdnsname'] == 'on') ? '1':'0';
    $sa_urlreport = ($settings['urlreport'] == 'on') ? '1':'0';
    $sa_userreport = ($settings['userreport'] == 'on') ? '1':'0';
    $sa_config = <<<EOD
Output	/usr/local/www/squidreport
WebUrl	/squidreport
LogFile	$SquidLog
UseClientDNSName	$sa_useclientdns
DNSLookupTimeout	0.0001
OrderNetwork	bytes
OrderUser	bytes
OrderUrl	bytes
OrderMime	bytes
UrlReport	$sa_urlreport
UrlHitsOnly	0
UserReport $sa_userreport
QuietMode	1
Lang	/usr/local/etc/squidreport/lang/$sa_lang
DateFormat	$sa_dateformat
TopUrlUser	$sa_topurluser
Locale $sa_locale
EOD;
    if (!file_exists('/usr/local/www/squidreport'))
      mwexec('mkdir /usr/local/www/squidreport');
    file_put_contents(SACONFIG, $sa_config);

    // add CRONTAB
    $sa = false;
    foreach ($config['cron']['item'] as $item) {
      if ("/usr/local/bin/squid-analyzer -c " . SACONFIG == $item['command']) {
        $sa = true;
        break;
      }
    }
    if ($sa == false) {
      if (preg_match('/(\d+)m/', $settings['frequency'], $matches)) {
      $config['cron']['item'][] = array(
        "minute" => "*/" . $matches[1],
        'hour'    => '*',
        'mday'    => '*',
        'month'   => '*',
        'wday'    => '*',
        'who'     => 'root',
        'command' => "/usr/local/bin/squid-analyzer -c " . SACONFIG
      );
      }
      if (preg_match('/(\d+)h/', $settings['frequency'], $matches)) {
      $config['cron']['item'][] = array(
        "minute"  => "*",
        'hour'    => "*/" . $matches[1],
        'mday'    => '*',
        'month'   => '*',
        'wday'    => '*',
        'who'     => 'root',
        'command' => "/usr/local/bin/squid-analyzer -c " . SACONFIG
      );
      }
      if (preg_match('/(\d+)d/', $settings['frequency'], $matches)) {
      $config['cron']['item'][] = array(
        "minute"  => "*",
        'hour'    => '*',
        'mday'    => "*/" . $matches[1],
        'month'   => '*',
        'wday'    => '*',
        'who'     => 'root',
        'command' => "/usr/local/bin/squid-analyzer -c " . SACONFIG
      );
      }
      write_config();
    }
  }
}

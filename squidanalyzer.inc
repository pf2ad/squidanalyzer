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

    $settings = $config['installedpackages']['squidanalyzer']['config'][0];
    $SquidLog = $settings['squidlog'];
    $sa_lang = $settings['lang'];
    $sa_dateformat = $settings['dateformat'];
    $sa_topurluser = $settings['topurluser'];
    $sa_locale = $settings['locale'];
    $sa_config = <<<EOD
Output	/usr/local/www/squidanalyzer
WebUrl	/squidanalyzer
LogFile	$SquidLog
UseClientDNSName	0
DNSLookupTimeout	0.0001
OrderNetwork	bytes
OrderUser	bytes
OrderUrl	bytes
OrderMime	bytes
UrlReport	1
UrlHitsOnly	0
UserReport	1
QuietMode	1
Lang	/usr/local/etc/squidreport/lang/$sa_lang
DateFormat	$sa_dateformat
TopUrlUser	$sa_topurluser
Locale $sa_locale
EOD;
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

<?php
// $Id$

/**
 * @file
 * Drush integration for Libraries API.
 */

/**
 * Implements hook_drush_command().
 */
function libraries_drush_command() {
  return;
  $items['libraries-list'] = array(
    'callback' => 'libraries_drush_list',
    'description' => dt('Lists registered library information.'),
  );
  $items['libraries-download'] = array(
    'callback' => 'libraries_drush_download',
    'description' => dt('Downloads a registered library into the libraries directory for the active site.'),
    'arguments' => array(
      'name' => dt('The internal name of the registered library.'),
    ),
  );
  return $items;
}

/**
 * Implements hook_drush_help().
 */
function libraries_drush_help($section) {
  switch ($section) {
    case 'drush:libraries-list':
      return dt('Lists registered library information.');

    case 'drush:libraries-download':
      return dt('Downloads a registered library into the libraries directory for the active site.

See libraries-list for a list of registered libraries.');
  }
}

/**
 * Downloads a library.
 *
 * @param $name
 *   The internal name of the library to download.
 */
function libraries_drush_download($name) {
  return;

  // @todo Looks wonky?
  if (!drush_shell_exec('type unzip')) {
    return drush_set_error(dt('Missing dependency: unzip. Install it before using this command.'));
  }

  // @todo Simply use current drush site.
  $args = func_get_args();
  if ($args[0]) {
    $path = $args[0];
  }
  else {
    $path = 'sites/all/libraries';
  }

  // Create the path if it does not exist.
  if (!is_dir($path)) {
    drush_op('mkdir', $path);
    drush_log(dt('Directory @path was created', array('@path' => $path)), 'notice');
  }

  // Set the directory to the download location.
  $olddir = getcwd();
  chdir($path);

  $filename = basename(COLORBOX_DOWNLOAD_URI);
  $dirname = basename(COLORBOX_DOWNLOAD_URI, '.zip');

  // Remove any existing Colorbox plugin directory
  if (is_dir($dirname)) {
    drush_log(dt('A existing Colorbox plugin was overwritten at @path', array('@path' => $path)), 'notice');
  }
  // Remove any existing Colorbox plugin zip archive
  if (is_file($filename)) {
    drush_op('unlink', $filename);
  }

  // Download the zip archive
  if (!drush_shell_exec('wget '. COLORBOX_DOWNLOAD_URI)) {
    drush_shell_exec('curl -O '. COLORBOX_DOWNLOAD_URI);
  }

  if (is_file($filename)) {
    // Decompress the zip archive
    drush_shell_exec('unzip -qq -o '. $filename);
    // Remove the zip archive
    drush_op('unlink', $filename);
  }

  // Set working directory back to the previous working directory.
  chdir($olddir);

  if (is_dir($path .'/'. $dirname)) {
    drush_log(dt('Colorbox plugin has been downloaded to @path', array('@path' => $path)), 'success');
  }
  else {
    drush_log(dt('Drush was unable to download the Colorbox plugin to @path', array('@path' => $path)), 'error');
  }
}
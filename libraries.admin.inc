<?php

/**
 * @file
 * Provides administrative page and form callbacks for Libraries module.
 */

/**
 * Form generation callback for the libraries overview table.
 *
 * This is a form instead of a page to allow easier extending in contributed
 * modules.
 *
 * @param array $form
 *   An associative array containing the structure of the form.
 * @param array $form_state
 *   A keyed array containing the current state of the form.
 *
 * @return array
 *   The form array for the overview form.
 */
function libraries_admin_overview(array $form, array &$form_state) {
  $header = array(t('Name'), t('Status'), t('Installed version'), t('Links'));
  $libraries = array();
  $rows[] = array();

  foreach (libraries_info() as $name => $info) {
    $libraries[$name] = libraries_detect($name);
  }
  ksort($libraries);

  foreach ($libraries as $machine_name => $library) {
    $actions = array();

    if ($library['vendor url']) {
      $actions[] = l('Homepage', $library['vendor url']);
    }
    if ($library['download url']) {
      $actions[] = l('Download', $library['download url']);
    }

    $rows[] = array(
      'data' => array(
        l($library['name'], 'admin/reports/libraries/' . $machine_name),
        ($library['installed'] ? t('OK') : drupal_ucfirst($library['error'])),
        (isset($library['version']) ? $library['version'] : ''),
        implode(' | ', $actions),
      ),
      'class' => ($library['installed'] ? array('ok') : array('error')),
    );
  }

  $form['libraries']['list'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('There are currently no libraries installed'),
  );

  return $form;
}

/**
 * Form generation callback for the status overview for a single library.
 *
 * This is a form instead of a page to allow easier extending in contributed
 * modules.
 *
 * @param array $form
 *   An associative array containing the structure of the form.
 * @param array $form_state
 *   A keyed array containing the current state of the form.
 * @param string $name
 *   The name of the library to return the status report for.
 *
 * @return array|null
 *   The form array for the status form or NULL if the library was not found.
 *
 * @todo Add some var_export($library)-style output
 */
function libraries_admin_library_status_form(array $form, array &$form_state, $name) {
  $library = libraries_detect($name);

  if ($library === FALSE) {
    // @todo MENU_NOT_FOUND does not work here.
    return;
  }

  drupal_set_title(t('Status report for library %library', array('%library' => $library['name'])), PASS_THROUGH);

  if ($library['installed']) {
    $rows = array();
    drupal_set_message(t('The %name library is installed correctly.', array('%name' => $library['name'])));
    $header = array(array('data' => '<strong>' . t('General information') . '</strong>', 'colspan' => 2, 'class' => 'table-heading', 'no_striping' => TRUE));

    $rows[] = array('<strong>' . t('Name') . '</strong>', check_plain($library['name']));
    $rows[] = array('<strong>' . t('Machine name') . '</strong>', check_plain($library['machine name']));

    if ($library['vendor url']) {
      $rows[] = array('<strong>' . t('Vendor URL') . '</strong>', l($library['vendor url'], $library['vendor url']));
    }

    if ($library['download url']) {
      $rows[] = array('<strong>' . t('Download URL') . '</strong>', l($library['download url'], $library['download url']));
    }

    $rows[] = array('<strong>' . t('Location') . '</strong>', $library['library path']);
    $rows[] = array('<strong>' . t('Version') . '</strong>', $library['version']);

    if (!empty($library['variants'])) {
      $rows[] = array('<strong>' . t('Variants') . '</strong>', implode(', ', array_keys($library['variants'])));
    }

    $form['status']['overview'] = array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $rows,
    );
  }
  else {
    drupal_set_message($library['error message'], 'error');
    switch ($library['error']) {
      case 'not found':
        $form['status']['not_found']['instruction']['#markup'] = t('Follow these steps to install the library:');
        $items = array();
        // 1. Download the library.
        // If no supported versions are specified, the latest version is
        // recommended.
        if (empty($library['versions'])) {
          $items[] = t('Download the latest version of the library <a href="@download-url">here</a>.', array(
            '@download-url' => $library['download url'],
          ));
        }
        // Otherwise, the latest supported version is recommended.
        else {
          $versions = array_keys($library['versions']);
          usort($versions, 'version_compare');
          $versions = array_reverse($versions);
          $version = $versions[0];
          $items[] = t('Download version %version of the library <a href="@download-url">here</a>.', array(
            '%version' => $version,
            '@download-url' => $library['download url'],
          ));
        }
        // 2. Unpack it.
        $items[] = t('If the library is an archive, i.e. if the file ending is for example <em>.tar.gz</em> or <em>.zip</em>, unpack it.');
        // 3. Create the libraries folder.
        $items[] = t('In the %library-directory directory of your Drupal installation create a %library directory.', array(
          '%library-directory' => 'sites/all/libraries',
          '%library' => $library['machine name'],
        ));
        // 4. Upload it.
        // If the library has variant-independent files, give the user the
        // location of an example file to check his filesystem against.
        list($filepath, $file) = libraries_get_example_filepath($library);
        if ($filepath !== FALSE) {
          $items[] = t('Upload the whole library (which can consist of multiple directories) into the newly created %library-path directory. For example, the %file file, should be located at following filepath: %filepath', array(
            '%library-path' => 'sites/all/libraries/' . $library['machine name'],
            '%file' => $file,
            '%filepath' => $filepath,
          ));
        }
        else {
          $items[] = t('Upload the whole library (which can consist of multiple directories) into the newly created %library-path directory.', array(
            '%library-path' => 'sites/all/libraries/' . $library['machine name'],
          ));
        }
        // 5. Reload.
        $items[] = t('<a href="">Reload</a> the page. If successful, you should see status information about this library.');

        $form['status']['not_found']['steps'] = array(
          '#theme' => 'item_list',
          '#items' => $items,
          '#type' => 'ol'
        );
        break;

      case 'not detected':
        // Re-check location.
        // If the library has variant-independent files, give the user the
        // location of an example file to check his filesystem against.
        list($filepath, $file) = libraries_get_example_filepath($library);
        if ($filepath !== FALSE) {
          $form['status']['not_detected']['instruction']['#markup'] = t('Check that the whole library is located at %library-path. For example, the %file file, should be located at following filepath: %filepath', array(
              '%library-path' => $library['library path'],
              '%file' => $file,
              '%filepath' => $filepath,
            )) . '<br>';
        }
        else {
          $form['status']['not_detected']['instruction']['#markup'] = t('Check that the whole library is located at %library-path.', array(
              '%library-path' => $library['library path'],
            )) . '<br>';
        }
        // If the library information was provided by a module, contact the maintainer.
        // @todo Add support for themes by accessing $library['info type']
        if (!empty($library['module'])) {
          $form['status']['not_detected']['help']['#markup'] = t('If yes, the library information is corrupted. Contact the maintainer of the %module module with the following information:', array(
              // @todo Use the human-readable module name instead.
              '%module' => $library['module'],
            )) . '<br>';
        }
        // Otherwise contact the author of the info file.
        elseif (!empty($library['info file'])) {
          $form['status']['not_detected']['help']['#markup'] = t("If yes, the library's info file (%info-file) is corrupted. Contact the author of this file with the following information:", array(
              '%info-file' => $library['info file'],
            )) . '<br>';
        }
        break;

      case 'not supported':
        // Either download a different version of the library...
        $versions = array_keys($library['versions']);
        usort($versions, 'version_compare');
        $versions = array_reverse($versions);
        $version = $versions[0];
        $form['status']['not_supported']['instruction']['#markup'] = t('Please install version %version of the library by following the following steps:', array(
          '%version' => $version,
        ));
        // 1. Delete the old library.
        $items[] = t('Delete the entire contents of the %library-path directory.', array(
          '%library-path' => $library['library path'],
        ));
        // 2. Download the new library.
        $items[] = t('Download version %version of the library <a href="@download-url">here</a>.', array(
          '%version' => $version,
          '@download-url' => $library['download url'],
        ));
        // 3. Unpack it.
        $items[] = t('If the library is an archive, i.e. if the file ending is for example <em>.tar.gz</em> or <em>.zip</em>, unpack it.');
        // 4. Upload the new library.
        // If the library has variant-independent files, give the user the
        // location of an example file to check his filesystem against.
        list($filepath, $file) = libraries_get_example_filepath($library);
        if ($filepath !== FALSE) {
          $items[] = t('Upload the new files into the %library-path directory. For example, the %file file, should be located at following filepath: %filepath', array(
            '%library-path' => $library['library path'],
            '%file' => $file,
            '%filepath' => $filepath,
          ));
        }
        else {
          $items[] = t('Upload the new files into the %library-path directory.', array(
            '%library-path' => $library['library path'],
          ));
        }
        // 5. Reload.
        $items[] = t('<a href="">Reload</a> the page. If successful, you should see status information about this library.');
        $form['status']['not_supported']['steps'] = array(
          '#theme' => 'item_list',
          '#items' => $items,
          '#type' => 'ol',
        );
        // ...or contact the maintainer of the library information.
        // If the library information was provided by a module, contact the maintainer.
        // @todo Add support for themes by accessing $library['info type']
        if (!empty($library['module'])) {
          $form['status']['not_supported']['help']['#markup'] = t('If you are bound to the current version of the library, ask the maintainer of the %module module to provide support for version %version.', array(
              // @todo Use the human-readable module name instead.
              '%module' => $library['module'],
              '%version' => $library['version'],
            )) . '<br>';
        }
        // Otherwise contact the author of the info file.
        elseif (!empty($library['info file'])) {
          $form['status']['not_supported']['help']['#markup'] = t("If you are bound to the current version of the library, ask the author of this library's info file (%info-file) to provide support for version %version", array(
              '%info-file' => $library['info file'],
              '%version' => $library['version'],
            )) . '<br>';
        }
        break;

        // @todo Add a 'missing dependency' case.
        // @todo Add a 'incompatible dependency' case.
    }
  }

  return $form;
}

/**
 * Returns an example file path and file name from a library, if possible.
 *
 * The result of this function can help users to verify that they have uploaded
 * the library to the correct location.
 *
 * @param array $library
 *   A library information array.
 *
 * @return array
 *   A list containing the example filepath and the example filename. If no
 *   example file was found, both entries will be FALSE.
 */
function libraries_get_example_filepath(array $library) {
  $filepath = FALSE;
  $file = FALSE;
  foreach (array('js', 'css', 'php') as $type) {
    if (!empty($library['files'][$type])) {
      $files = array_keys($library['files'][$type]);
      $filepath = $files[0];
      $parts = array_reverse(explode('/', $filepath));
      $file = $parts[0];
      $filepath = (!empty($library['path']) ? $library['path'] . '/' . $filepath : $filepath);
      $filepath = 'sites/all/libraries/' . $library['machine name'] . '/' . $filepath;
    }
  }
  return array($filepath, $file);
}

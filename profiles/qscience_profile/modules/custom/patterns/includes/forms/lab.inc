<?php
/**
 * @file
 * Functions related to exporting patterns.
 */

function patterns_lab($form, &$form_state) {

  $form = array();

  if (!patterns_parser_ready()) {
    $messages = t('No available patterns parser was found.</br>');
    $messages .= t(' Go to the !modules page to enable more Patterns parsers.', array('!modules' => l(t('modules'), 'admin/modules')));
    drupal_set_message($messages,'warning');
    return $form;
  }

  $header = array(
    'category' => array('data' => t('category'), 'field' => 'n.category', 'sort' => 'asc'),
    'name' => array('data' => 'name', 'field' => 'n.name'),
    'title' => array('data' => 'title', 'field' => 'n.title'),
    'status' => array('data' => 'status', 'field' => 'n.status'),
    'author' => array('data' => 'author', 'field' => 'n.author'),
    'version' => array('data' => 'version', 'field' => 'n.version'),
    'description' => array('data' => 'description', 'field' => 'n.description')
  );
  $rows = array();

  // Load the patterns from database.
  // no check with file system
  $patterns = patterns_db_get_patterns();

  foreach ($patterns as $pid => $pattern) {
    $pattern_code = unserialize($pattern->pattern);
    $category = @$pattern_code['info']['category'] ? @$pattern_code['info']['category'] : t('Other');
    $title = @$pattern_code['info']['title'];
    $name = $pattern->name;
    $status = (@$pattern->status) ? t('Enabled') : t('Disabled');
    $author = @$pattern_code['info']['author'];
    $version = @$pattern_code['info']['version'];
    $description = @$pattern->description;
    $row = array(
      'category'    => $category,
      'name'        => $name,
      'title'       => empty($title) ? '-' : $title,
      'status'      => $status,
      'author'      => empty($author) ? '-' : $author,
      'version'     => empty($version) ? '-' : $version,
      'description' => empty($description) ? '-' : 'No description',
    );
    $rows[$pid+1] = $row; // somehow we get (pid -1) in the submit handler
  }
  
  $text = 'In this area you can experiment by mixing several patterns together into a new one. The resulting pattern file can be imported into the database or downloaded as a file.';
  $title = 'Patterns Lab';
  patterns_forms_add_page_header($form, $title, $text);

  $form['patterns'] = array(
    '#type' => 'fieldset',
    '#title' => t('Available Patterns'),
  );

  $form['patterns']['patterns_table'] = array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $rows,
    '#empty' => t('No Patterns'),
  );

  $form['lab'] = array(
    '#type' => 'fieldset',
    '#title' => t('Lab options'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $form['lab']['mode'] = array(
    '#type' => 'radios',
    '#options' => _patterns_lab_get_exp_mode_options(),
    '#title' => t('Include mode'),
    '#default_value' => PATTERNS_EXPORT_MODE_FULL,
  );

  $form['lab']['to'] = array(
    '#type' => 'radios',
    '#options' => _patterns_lab_get_exp_to_options(),
    '#title' => t('Action'),
    '#default_value' => PATTERNS_EXPORT_TO_FILE,
  );

  patterns_forms_get_info_section($form, array(), array('collapsed' => TRUE));

  $form['export_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name of the new pattern file'),
    '#description' => t('Extension will be automatically added'),
    '#default_value' => t('pattern') . '_' . strval(time()),
  );

  $form = patterns_forms_get_formats_selector($form,
                                              patterns_forms_get_default_format(),
                                              'Include all selected patterns in this format.',
                                              FALSE
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Go')
  );

  $form['#submit'][] = 'patterns_lab_submit';

  return $form;
}

/**
 * Checks if the patterns directory is ready and if the file name
 * for exported patterns is valid.
 *
 * @param $form
 * @param $form_state
 */
function patterns_lab_validate($form, &$form_state) {
  if (!isset($form_state['values']['patterns_table'])) {
    form_set_error('patterns_table', t('An error has occurred'));
  }

  if (array_sum($form_state['values']['patterns_table']) === 0) {
    form_set_error('patterns_table', t('No pattern selected.'));
  }

  if (!isset($form_state['values']['mode']) || !patterns_lab_is_valid_export_mode($form_state['values']['mode'])) {
    form_set_error('mode', t('Please select a valid include mode option'));
  }

  if (!isset($form_state['values']['to'])) {
    form_set_error('to', t('Please select a valid action for the export'));
  }

  if (!patterns_parser_exists($form_state['values']['format'])) {
    form_set_error('format', t('The pattern format seems to be invalid.'));
  }

  if ($form_state['values']['to'] === PATTERNS_EXPORT_TO_DB) {
    _patterns_import_check_name($form_state['values']['export_name']);
  }
  elseif (empty($form_state['values']['export_name'])) {
    $form_state['values']['export_name'] = 'pattern';
  }

  return TRUE;
}

/**
 * Exports selected patterns either in a file or as a zip-archive
 * @param $form
 * @param $form_state
 */
function patterns_lab_submit($form, &$form_state) {
  global $base_url;
  
  // Load the patterns from database.
  @$patterns = patterns_db_get_patterns();
  $path = patterns_path_get_files_dir();

  $format   = $form_state['values']['format'];
  $filename = $form_state['values']['export_name'];
  $zip_flag = ($form_state['values']['to'] === PATTERNS_EXPORT_TO_ZIP) ? TRUE : FALSE;
  
  // Merged all the selected patterns
  $export = patterns_api_add_info_section($form_state['values']['info']);
  $count = 0;
  $errors = array();
  $ancestors = array();
  foreach ($form_state['values']['patterns_table'] as $pid) {
    if (!is_int($pid)) {
      $pattern = patterns_get_pattern($pid);

      if (!$pattern) {
        drupal_set_message(t('Error reading pattern with id :pid', array(':pid' => $pid)), 'error');
        continue;
      }

      if (empty($pattern->pattern)) {
        $errors[] = patterns_utils_toString($pattern);
        continue;
      }

        $included = _patterns_lab_include_pattern($pattern, $form_state['values']['mode']);
        $included = array('pattern' => $included);
        $export['lab'][] = array('include' => $included);
        $ancestors[] = $pattern->uuuid;
      $count++;
    }
    //Add history header including all the merged patterns as ancestors. D2D ID will be null in case the module is not enabled
    $export = patterns_history_add_header($export, 'merge', $base_url . '/xmlrpc.php', variable_get('d2d_my_id', NULL), $ancestors);
  }

  // Prepare a zip file or a regular one
  if ($zip_flag) {
    // Cast to the expected format
    $p[$filename] = $export;
    $zip_path = patterns_create_zip($p, $format, $path, $filename);
    
    if (!$zip_path) {
      return FALSE;
    }else{
      return patterns_download_zip($zip_path);
    }
  }else{
    // Parse into a file and add info to the exported pattern
    $file = patterns_parser_dump($export, $format);
    $file = patterns_parser_dump_comment("Patterns exported: " . $count, $format, $file);
    $file = patterns_parser_dump_comment("Date and time: " . date('r'), $format, $file);
    // Store only in the database or return as regular file
    if ($form_state['values']['to'] === PATTERNS_EXPORT_TO_DB) {
      patterns_io_save_pattern($file, $filename, $format, $path);
      drupal_goto('admin/patterns/lab/');
    }else{
      drupal_add_http_header("Content-type", " text/plain; charset=utf-8");
      drupal_add_http_header("Content-Disposition", "attachment;filename=" . $filename . '.' . $format);
      print $file;
      exit;
    }
  }
}

/**
 * Builds an associative array of export options.
 *
 * Checks if zip compression is available.
 *
 */
function _patterns_lab_get_exp_to_options() {
  $options = array(
    PATTERNS_EXPORT_TO_DB   => t('Import into the database'),
    PATTERNS_EXPORT_TO_FILE => t('Download as a pattern file'),
  );

  if (class_exists('ZipArchive')) {
    $options[PATTERNS_EXPORT_TO_ZIP] = t('Download as a zip-archive');
  }

  return $options;
}

/**
 * Builds an associative array of export options.
 *
 */
function _patterns_lab_get_exp_mode_options() {
  $options = array(
    PATTERNS_EXPORT_MODE_FULL     => t('All the sections (info and modules included) of the pattern'),
    PATTERNS_EXPORT_MODE_ACTIONS  => t('Actions only (info and modules sections excluded) of the pattern'),
    PATTERNS_EXPORT_MODE_NAME     => t('The unique name of the pattern'),
    PATTERNS_EXPORT_MODE_ID       => t('The numerical id of the pattern'),
  );

  return $options;
}

/**
 * Builds an associative array of export type options.
 *
 */
function _patterns_lab_get_exp_type_options() {
  $options = array(
      PATTERNS_MODIFY    => t('Export as a set of MODIFY actions'),
      PATTERNS_CREATE   => t('Export as a set of CREATE actions'),
  );

  return $options;
}


/**
 * Returns TRUE if the passed parameter is a valid flag
 * to configure Patterns behaviors with the 'include' tag.
 *
 */
function patterns_lab_is_valid_export_mode($mode = NULL) {
  if (is_null($mode)) return FALSE;

  $modes = array(
    PATTERNS_EXPORT_MODE_FULL,
    PATTERNS_EXPORT_MODE_ACTIONS,
    PATTERNS_EXPORT_MODE_NAME,
    PATTERNS_EXPORT_MODE_ID,
  );

  return (in_array($mode, $modes)) ? TRUE : FALSE;
}

/**
 * Returns the portion of a pattern object to be included as a string,
 * depending on the passed mode of exporting.
 *
 * @param stdclass $pattern an object representing the pattern as loaded from
 *  the database
 * @param mixed $mode one of the four inclusion modes
 *
 */
function _patterns_lab_include_pattern($pattern, $mode = PATTERNS_EXPORT_MODE_FULL) {
  if (empty($pattern)) return FALSE;
  if ($mode === PATTERNS_EXPORT_MODE_FULL) {
    return $pattern->pattern;
  }
  if ($mode === PATTERNS_EXPORT_MODE_ACTIONS) {
    return patterns_parser_extract_all_actions($pattern->pattern);
  }
  if ($mode === PATTERNS_EXPORT_MODE_NAME) {
    return $pattern->name;
  }
  if ($mode === PATTERNS_EXPORT_MODE_ID) {
    return $pattern->pid;
  }
  return FALSE;

}


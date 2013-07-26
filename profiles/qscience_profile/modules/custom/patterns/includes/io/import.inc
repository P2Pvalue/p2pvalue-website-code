<?php
/**
 * @file
 * Importing Patterns from a file or using an URL.
 */


function _patterns_io_import_pre_checkings() {
	if (!patterns_parser_ready()) {
   	$messages = t('No available patterns parser found. ');
    $messages.= t('Go to the !modules page to enable more Patterns parsers.', array('!modules' => l(t('modules'), 'admin/modules')));
   	drupal_set_message($messages, 'error');
   	return FALSE;
  }

  $destination = patterns_path_get_files_dir();

  // If not automatically returns
  $diready = _patterns_import_check_dirs($destination);
  if (!$diready) {
  	return FALSE;
  }
	
  return FALSE;
}

function _patterns_io_import_validate_content($pattern) {
  $patternscan = patterns_scan_pattern($pattern);
  if (!_patterns_scan_validate_patternscan($patternscan)) {
    $errors_on_scan = _patterns_scan_analyze_patternscan($patternscan);
    drupal_set_message(t('Error parsing the YAML file: ' . implode(', ', $errors_on_scan)), 'error');
    return FALSE;
  }
  return TRUE;
}
/**
 * Helper function to be called in one of the import validation handlers.
 */
function _patterns_io_import_check_dirs($dir = NULL) {
  // Check Patterns Directory.
  if (!_patterns_io_is_patterns_dir_ready($dir, FILE_CREATE_DIRECTORY)) {
    $message = t("Destination folder does not exist or is not writable: %path<br />", array('%path' => $dir)); // TODO: move this out for performance
    $link = l(t('status report'), 'admin/reports/status');
    $message .= t('You might want to check the !link to ensure your files directory exists and is writable.', array('!link' => $link));
    form_set_error('pattern_file', $message);
    return FALSE;
  }
}

/**
 * Helper function to be called in one of the import validation handlers.
 */
function _patterns_io_import_check_name($name = '') {
  // Name Validation.
  $errors_on_name = _patterns_parser_analyze_pattern_name($name);
  if (!empty($errors_on_name)) {
    form_set_error('name', t('Errors in the name of the pattern: ' . implode(', ', $errors_on_name))); // TODO: use proper t()
    return FALSE;
  }

  if ($obj = patterns_db_load_pattern_from_name($name)) {
    form_set_error('name', t('A pattern file with the same name already exists: !name', array('!name' => $name)));
    return FALSE;
  }
}

/**
 * Extracts and validates the pattern format from a file string
 * 
 * The pattern format can be specified as parameter, and it this case
 * the first parameter will be ignored. The formata will be anyway
 * validated.
 * 
 * @param string $file A path to a file
 * @param string $format Optional. Enforces the format type
 * @return string|bool $format The pattern format, or FALSE, if the format
 *   is invalid 
 */
function patterns_io_get_format_from_file($file, $format = PATTERNS_FORMAT_UNKNOWN, $verbose = TRUE) {
	if ($format == PATTERNS_FORMAT_UNKNOWN) {
		$format = pathinfo($file, PATHINFO_EXTENSION);
	}
	if (!patterns_parser_exists($format)) {
		if ($verbose) drupal_set_message(t('Unknown pattern format: %format. Please check available parsers.', array('%format' => $format)), 'error');
		return FALSE;
	}
	return $format;
}

function patterns_io_get_name_from_file($file) {
	return pathinfo($file, PATHINFO_BASENAME);
}

function patterns_io_import_file($file, $format = PATTERNS_FORMAT_UNKNOWN, $name = NULL) {
  $ready = _patterns_io_import_pre_checkings();
	if ($ready) {
		return FALSE;
	}
	
	$format = patterns_io_get_format_from_file($file, $format);
	if (!$format) {
		return FALSE;
	}
	
	if (is_null($name)) {
		$name = patterns_io_get_name_from_file($file);
		if (!$name) {
			return FALSE;
		}
	}
	
	$content = file_get_contents($file);
  if (!$content) {
  	drupal_set_message(t('An error occurred while opening remote file.'), 'error');
  	return FALSE;
  }
  $pattern = patterns_parser_parse($content, $format);
	if (!$pattern) {
		drupal_set_message(t('Patterns syntax seems invalid.'), 'error');
		return FALSE;
	}
	
	$valid = _patterns_io_import_validate_content($pattern);
  if (!$valid) {
  	return FALSE;
  }
  return patterns_io_save_pattern($pattern, $name, $format);
    
}


function patterns_io_import_file_remote($uri, $format = PATTERNS_FORMAT_UNKNOWN) {
	if (!ini_get('allow_url_fopen')) {
  	drupal_set_message(t('allow_url_fopen must be enabled in your php configuration in order to open remote file.'), 'error');
  	return FALSE;
	}
	
	return patterns_io_import_file($uri, $format);
}
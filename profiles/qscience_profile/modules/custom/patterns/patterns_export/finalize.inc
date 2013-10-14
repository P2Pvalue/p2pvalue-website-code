<?php

/**
 * Initializes the appropriate action to export a collection
 * of patterns
 * 
 * @param string $action The type of export action
 * @param array $sections The sections of the pattern
 * @param array $info The info section of the pattern
 * @param array $modules The modules section of the pattern
 * @param string $format The pattern format
 * @param string $filename (optional) The final name of file 
 * @param string $path (optional) The path of the temporary directory in which the zip file will be saved
 */
function patterns_export_finalize_export($action = PATTERNS_EXPORT_TO_FILE, $sections = array(), $info = array(), $modules = array(), $format, $filename = NULL) {
  // If some sections did not return any data
  // we remove them. We don't remove the modules!
  foreach ($sections as $s => $value) {
    if (empty($value)) {
      unset($sections[$s]);
    }
  }

  if (empty($sections)) {
    drupal_set_message(t('Export did not return any data.'), 'error');
    return FALSE;
  }
																						
	if ($action == PATTERNS_EXPORT_TO_FILE) {
		 return patterns_export_to_file($sections, $info, $modules, $format, $filename);
	}
	else if ($action == PATTERNS_EXPORT_TO_DB) {
		 return patterns_export_to_db($sections, $info, $modules, $format, $filename);
	}
	else if ($action == PATTERNS_EXPORT_TO_ZIP) {
		$path = pathinfo($filename, PATHINFO_DIRNAME);
		if (!empty($path)) {
			// no full path, probably it is from the web interface
			if ($path == '.') {
				$path = NULL;
			}
			// User specified the full path, it want the Zip file to be created there
			else {
				$path.= '/';
				$filename = pathinfo($filename, PATHINFO_BASENAME);	
			}
		}
		
		return patterns_export_to_zip($sections, $info, $modules, $format, $filename, $path);
	}
	else {
		drupal_set_message(t('Unknown export option: %option', array('%options' => $action)));
	}
}

/**
 * Exports patterns as a donwlodable Zip archive
 * 
 * @param array $sections The sections of the pattern
 * @param array $info The info section of the pattern
 * @param array $modules The modules section of the pattern
 * @param string $format The pattern format
 * @param string $filename (optional) The final name of file 
 * @param string $path (optional) 
 *   The path of the temporary directory in which the zip file will be saved
 */
function patterns_export_to_zip($sections = array(), $info = array(), $modules = array(), $format, $filename = NULL, $path = NULL) {
	
  $zip_path = patterns_create_zip($sections, $format, $path, $filename);
	
  if (!$zip_path) {
  	return FALSE;
	}
	
	// web server
	if (!patterns_utils_is_cli()) {
		patterns_download_zip($zip_path);
	}
	
	return TRUE;
}

/**
 * Exports a pattern by importing it into the database
 * 
 * @param array $sections The sections of the pattern
 * @param array $info The info section of the pattern
 * @param array $modules The modules section of the pattern
 * @param string $format The pattern format
 * @param string $filename (optional) The final name of file 
 */
function patterns_export_to_db($sections = array(), $info = array(), $modules = array(), $format, $filename = NULL) { 
	$pattern = _patterns_export_merge_exported_patterns($sections, $info, $modules);
	//drupal_set_message(print_r($pattern,true));
	return patterns_io_save_pattern($pattern, $filename, $format);
}

/**
 * Exports a pattern as a donwlodable text file
 * 
 * @param array $sections The sections of the pattern
 * @param array $info The info section of the pattern
 * @param array $modules The modules section of the pattern
 * @param string $format The pattern format
 * @param string $filename The final name of file 
 */
function patterns_export_to_file($sections = array(), $info = array(), $modules = array(), $format, $filename) {
	$pattern = _patterns_export_merge_exported_patterns($sections, $info, $modules);
	$pattern = patterns_parser_dump($pattern, $format);
	
	// drush
	if (patterns_utils_is_cli()) {
		return file_put_contents($filename, $pattern);
	}
	// web server
	else {
		patterns_download_file($pattern, $filename . '.' . $format);
	}
}


// Helper functions



/**
 * Merges modules info and sections in one pattern
 * 
 * @param array $sections The sections of the pattern
 * @param array $info The info section of the pattern
 * @param array $modules The modules section of the pattern
 * 
 * @return array $pattern The merged pattern
 */
function _patterns_export_merge_exported_patterns($sections, $info, $modules) {
	$pattern = patterns_api_add_info_section($info);
	// Modules that needs to be enabled to run the pattern
  patterns_api_add_modules_section($modules, $pattern);

  foreach ($sections as $s => $values) {
    $pattern[$s] = $values;
  }
  // Display the exported pattern
 	//drupal_set_message(print_r($pattern, true));
  return $pattern;
}
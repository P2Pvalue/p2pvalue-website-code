<?php
/**
 * @file
 * Importing Patterns from a file or using an URL.
 */

/**
 * 
 * Prepares a zip archive of patterns files and serves the file to download
 * 
 * @param unknown_type $patterns
 * @param unknown_type $format
 */
function patterns_create_zip($patterns = array(), $format = PATTERNS_FORMAT_YAML, $path = NULL, $filename = NULL) {
 	
  if (!class_exists('ZipArchive')) {
    drupal_set_message(t('Zip extension not found.'), 'error');
    return FALSE;
  }

  $path = (empty($path)) ? patterns_path_get_tmp_dir() : $path;
  if (!file_prepare_directory($path)) {
    drupal_set_message(t('Path does not exist or is not writable: %path', array('%path' => $path)), 'error');
    return FALSE;
  }
  
  $zip = new ZipArchive();
  $zip_filename = (empty($filename)) ? "patterns" . strval(time()) . ".zip" : $filename;
  $zip_path = $path . '/' . $zip_filename;
  
  if (strrpos($zip_path, '.zip') !== (strlen($zip_path) - 4)) {
  	$zip_path.= '.zip';
  }
   
  if ($zip->open($zip_path, ZIPARCHIVE::CREATE) != TRUE) {
  	exit("Cannot locally create zip-archive. Ask your administrator for help.");
  }
	
  foreach ($patterns as $name => $p) {
  	// can be array or Stdclass
  	if (is_object($p)) {
  		$p = $p->pattern;
  		$name = $pattern->name;
  	}
  	$name = $name . '.' . $format;
  	$pp = patterns_parser_dump($p, $format);
  	$zip->addFromString($name, $pp);
  }
  
  $zip->close();
 
  if (!is_readable($zip_path)) {
  	drupal_set_message(t("An error occurred. Path is not readable: %path", array('%path' => $zip_path)), 'error');
    return FALSE; 
  }
  //drupal_set_message($zip_path);
  
  return $zip_path;
}

function patterns_download_zip($zip_path) {
  $filename = pathinfo($zip_path, PATHINFO_BASENAME);
  drupal_add_http_header("Content-type", "application/octet-stream");
  drupal_add_http_header("Content-Disposition", "attachment;filename=" . $filename);
  $err = @readfile($zip_path);
  if ($err == FALSE) {
  	drupal_set_message("An error occurred: couldn't read " . $zip_path . ".", 'error'); // TODO: t()
  } 
  //@drupal_unlink($zip_path);
  exit;
}

function patterns_download_file($file, $filename) {
  drupal_add_http_header("Content-type", " text/plain; charset=utf-8");
  drupal_add_http_header("Content-Disposition", "attachment;filename=" . $filename);
  print $file;
  exit;
}
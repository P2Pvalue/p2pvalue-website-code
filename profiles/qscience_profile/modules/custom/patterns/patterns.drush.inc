<?php
/**
 * @file
 * Drush Patterns module commands
 */

/**
 * TODO: parse additional parameters for patterns execution
 * TODO: do we need the form-helper command ?
 */

/**
 * Implements hook_drush_help().
 */
function patterns_drush_help($section) {
	switch ($section) {
		case 'drush:patterns':
			return dt("List all available patterns. Patterns in the trash bin do not appear in this list.");
			break;
		case 'drush:patterns-info':
			return dt("Show all info on a given pattern.");
			break;
		case 'drush:patterns-quickrun':
			return dt("Read a pattern file and run it without saving it into the database.");
			break;	
		case 'drush:patterns-run':
			return dt('Run a pattern file. If the pattern is not found in the database, it will try to import it.');
			break;
		case 'drush:patterns-import':
			return dt('Import a pattern file into the database.');
			break;
		case 'drush:patterns-export':
			$text = "Export data from the patterns components to file, zip archive, or database ([file|db|zip]). ";
			$text.= "Zip compression requires ZipArchive class to be installed.";
			$text.= "It is possible to select which components to extract following the syntax: 'component.tag.export'. ";
			$text.= "The relative path of extracted files is computed from the Drupal root directory. Absolute paths can be used to point to external paths. ";
			return dt($text);
			break;	
		 case 'drush:patterns-test-ci':
      return dt('Starts the automated tests for Patterns and return an exit value for continuos integration.');
      break;
	}
}

/**
 * Implements hook_drush_command().
 */
function patterns_drush_command() {

	$dependencies = array('patterns', 'patterns_components', 'patterns_phpparser', 'patterns_yamlparser', 'patterns_xmlparser', 'token');
	$deps_export = $dependencies;
	array_push($deps_export, 'patterns_export');

	// TODO: do we need the form-helper command ?
	/*
  $items['patterns-form-helper'] = array(
    'description' => 'Enable/disable patterns form helper',
    'arguments' => array(
      'action' => "Valid values: enable, disable.",
    )
  );
	*/
	
	// configuration / information
	
	$items['patterns'] = array(
    'callback' => 'drush_patterns',
    'description' => "List all available patterns."
  );
	
  $items['patterns-paths'] = array(
    'description' => "List all paths where patterns will be looked for",
    'drupal dependencies' => array('patterns'),
  );
   
  $items['patterns-components'] = array(
    'description' => "List all the available patterns components",
    'drupal dependencies' => array('patterns'),
  );

  $items['patterns-info'] = array(
    'callback' => 'drush_patterns_info',
    'description' => "Show all info on a given pattern.",
    'arguments' => array(
      'id' => "The id or the name of the pattern as stored in the database.",
    )
  );
  
  $items['patterns-allow-publish'] = array(
    'description' => 'Enable/disable patterns publishing',
    'arguments' => array(
      'action' => "Valid values: enable, disable.",
    )
  );
 
  $items['patterns-publish'] = array(
    'description' => "Publish a pattern.",
    'arguments' => array(
      'id' => "The id or the name of the pattern to publish.",
    ),
    'drupal dependencies' => array('patterns'),
  );

  $items['patterns-unpublish'] = array(
    'description' => "Unpublish a pattern.",
    'arguments' => array(
      'id' => "The id or the name of the pattern to unpublish.",
    ),
    'drupal dependencies' => array('patterns'),
  );
   
  // run / quickrun
	
	$items['patterns-quickrun'] = array(
    'callback' => 'drush_patterns_quickrun',
    'description' => 'Load a pattern file and executes it without saving it into the database',
    'examples' => array(
      'drush patterns-quickrun myPattern.yaml' => 'Read myPattern.yaml,  parses it using the XMLParser, and runs it in php mode',
 			'drush patterns-quickrun myPattern yaml batch' => 'Read myPattern,  parses it using the YAMLParser, and runs it in batch mode',
		),
    'arguments' => array(
      'file_name' => 'The path to the a pattern file',
    	'format' => 'Optional. The format of the pattern. If not specified, it will be determined by the file extension.',
     	'mode' => 'Optional. Execute via "batch" or "php". Defaults "php"',
		 	// TODO: parse additional options
			//'params' => 'Optional. Additional options for the pattern execution.',    
		),
    'drupal dependencies' => $dependencies,
    'core' => array('7'),
	);
	
	$items['patterns-run'] = array(
    'callback' => 'drush_patterns_run',
    'description' => 'Import and enable the specified pattern file',
    'examples' => array(
      'drush patterns-run path/to/patterns/file.xml' =>
        'Parses file.xml using the XMLParser, imports it into the database and runs it',
      'drush patterns-run path/to/pattern/file yaml batch' => 
      	'Parses file using the YAMLParser, imports it into the database and runs it in batch mode',
			'drush patterns-run 2' => 
      	'Runs the pattern stored in the database with id 2',
			'drush patterns-run pattern_in_db.yaml' => 
      	'Runs the pattern stored in the database with the name pattern_in_db.yaml',
      ),
    'arguments' => array(
      'pattern' => 'The path to the a pattern file, the pattern id or the pattern name',
    	'format' => 'Optional. The format of the pattern. If not specified, it will be determined by the file extension.',
     	'mode' => 'Optional. Execute via "batch" or "php". Defaults "php"',
		 	// TODO: parse additional options
			//'params' => 'Optional. Additional options for the pattern execution.',    
		),
    'drupal dependencies' => $dependencies,
    'core' => array('7'),
  );
	   
  $items['patterns-import'] = array(
    'callback' => 'drush_patterns_import_file',
    'description' => 'Import a pattern local file into the database.',
    'examples' => array(
      'drush patterns-import path/to/patterns/file.xml' =>
        'Parses file.xml using the XMLParser, and saves the results into the database.',
      	'drush patterns-import path/to/pattern/file' => 'Import the specified pattern file and give it a machine readable name'
      ),
    'arguments' => array(
      'file' => 'The path to a pattern file',
      'format' => 'Optional. A valid pattern format, e.g. xml or yaml. If not specified, it will be determined by the file extension.'
    ),
    'drupal dependencies' => $dependencies,
    'core' => array('7'),
   );
 
  $items['patterns-import-remote'] = array(
    'callback' => 'drush_patterns_import_file_remote',
    'description' => 'Import a remote pattern file into the database.',
    'examples' => array(
      'drush patterns-import-remote http://myserver.org/mypattern.yaml' =>
        'Opens the remote location, parses mypattern.yaml using the YAMLParser, and saves the results into the database.',
      'drush patterns-import-remote http://myserver.org/mypattern yaml' =>
        'Opens the remote location, parses mypattern using the YAMLParser, and saves the results into the database.',
    ),
    'arguments' => array(
      'uri' => 'The uri to a pattern file',
      'format' => 'Optional. A valid pattern format, e.g. xml or yaml. If not specified, it will be determined by the file extension.'
    ),
    'drupal dependencies' => $dependencies,
    'core' => array('7'),
  );
  
  $items['patterns-export'] = array(
    'description' => 'Export data from all or a selection of patterns components to a file, a zip file, or saves it into the database.',
    'examples' => array(
      'drush patterns-export file path/to/patterns/file.yaml' =>
        'Exports from *all* available components to the specified file. The file is *not* imported in the database.',
      'drush patterns-export db exported.xml taxonomy' => 
      	'Exports from all tags of the component "taxonomy" and saves the results in the database under the machine_name of exported.xml.',
  		'drush patterns-export zip path/to/exported.zip yaml taxonomy.term' => 
  			'Exports from tag "term" of the component "taxonomy" and saves the results in the zip archive exported.zip containing patterns in YAML format.',
			'drush patterns-export file path/to/exported.yaml taxonomy.term.all color block.block' => 
  			'Exports from export_function "all" in tag "term" of the component "taxonomy", from all tags of the component "color", and from tag "block" of component "block". Results are saved to file exported.yaml.',  
     ),
     'arguments' => array(
     	'action' => 'The export action: export to a file, to a zip archive, or import the result of the export into the database',
      'file_name' => 'The path to a pattern file',
      'format' => 'Optional. To use only in combination with the export to zip action. A valid pattern format, e.g. xml or yaml. If not specified, it will be determined by the file extension.',
 			'exports' => 'Optional. A combination of components, tags, and export functions, that determine the content of the exported file',
    ),
    'drupal dependencies' => $deps_export,
    'core' => array('7'),
  );
  
  $items['patterns-test-ci'] = array(
    'callback' => 'drush_patterns_test_regular_expression',
    'description' => "Tests for continuos integration.",
    'arguments' => array(
      'regex' => "Regular expression to filter out tests.",
    )
  );
    
	return $items;
}

// Callbacks


/**
 * patterns list command callback.
 */
function drush_patterns() {
	$patterns = patterns_io_get_patterns();
	$patterns = $patterns[PATTERNS_STATUS_OK];
	$pipe = array();
	$rows[] = array(dt('Id'), dt('Name'), dt('Title'), dt('Status'), dt('Version'));
	foreach ($patterns as $pid => $pattern) {
		$rows[] = array($pattern->pid, $pattern->name, $pattern->title, $pattern->status, $pattern->info['version']);
		$pipe[] = "$pattern->title";
	}

	drush_print_table($rows, TRUE);
	drush_print_pipe(implode(' ', $pipe));
}

/**
 * patterns list command callback.
 * @param mixed int|string $id The pattern id or name of the pattern
 */
function drush_patterns_info($id) {
	$pattern = patterns_get_pattern($id);
	if (!$pattern) {
		return drush_set_error(dt("There is no pattern registered with id or name '$id'"));
	}
	$info = "";
	$info .= sprintf("  %-18s: %s\n", 'PID', $pattern->pid);
	$info .= sprintf("  %-18s: %s\n", 'Name', $pattern->name);
	$info .= sprintf("  %-18s: %s\n", 'Description', $pattern->description);
	$info .= sprintf("  %-18s: %s\n", 'File', $pattern->file);
	$info .= sprintf("  %-18s: %s\n", 'Status', $pattern->status);
	$info .= sprintf("  %-18s: %s\n", 'Public', $pattern->public);
	$info .= sprintf("  %-18s: %s\n", 'Updated', $pattern->updated);
	$info .= sprintf("  %-18s: %s\n", 'Enabled', $pattern->enabled);

	if (is_array($pattern->pattern['info'])) {
		foreach ($pattern->pattern['info'] as $key => $value) {
			$info .= sprintf("  %-18s: %s\n", ucfirst($key), $value);
		}
	}
	if (is_array($pattern->pattern['modules'])) {
		$info .= sprintf("  %-18s: %s\n", 'Modules', implode(', ', $pattern->pattern['modules']));
	}

	drush_print($info);
}

/**
 * patterns list command callback.
 */
function drush_patterns_form_helper($action) {
	if ($action == 'enable') {
		variable_set('patterns_form_helper', TRUE);
	}
	else {
		variable_set('patterns_form_helper', FALSE);
	}
}

/**
 * patterns list command callback.
 */
function drush_patterns_allow_publish($action) {
	if (in_array($action, 'enable', 'en', 'yes', 'y')) {
		drush_print("Patterns publishing patterns enabled.");
		variable_set('patterns_allow_publish', TRUE);
	}
	else if (in_array($action, 'disable', 'dis', 'no', 'n')) {
		drush_print("Patterns publishing patterns disabled.");
		variable_set('patterns_allow_publish', FALSE);
	}
	else {
		drush_set_error(dt("Unrecognized option."));
	}
}

/**
 * List of patterns path command callback.
 */
function drush_patterns_paths() {
	drush_print("List of path containing Patterns files.");
	$paths = patterns_path_get_patterns_dirs();
	foreach ($paths as $path) {
		drush_print("- $path", 4);
	}

	drush_print_pipe(implode(', ', $paths));
}

/**
 * List of patterns path command callback.
 */
function drush_patterns_components() {
	drush_print("List of available Patterns components.");
	$paths = patterns_io_list_components_names(TRUE);
	foreach ($paths as $path) {
		drush_print("- $path", 4);
	}

	drush_print_pipe(implode(', ', $paths));
}

/**
 * patterns publish command callback.
 */
function drush_patterns_publish($pid) {
	drush_print("Publishing pattern.");
	$result = patterns_db_publish_pattern($pid);
	if ($result) {
		drush_print("Operation was successful");
	}
	else {
		drush_set_error(dt("An error occurred, patterns was not marked as public."));
	}
}

/**
 * patterns unpublish command callback.
 */
function drush_patterns_unpublish($pid) {
	drush_print("Unpublishing a pattern.");
	$result = patterns_db_unpublish_pattern($pid);
	if ($result) {
		drush_print("Operation was successful");
	}
	else {
		drush_set_error(dt("An error occurred, patterns public status could not be changed."));
	}
}

// run, quickrun, import, export

/**
 * Runs a pattern file without importing it into the database
 *
 * @param string $file The path to the pattern file
 * @param string $format Optional. The format of the pattern.
 * @param string $mode Optional. batch or php. Defaults php.
 * @param array $params
 */
function drush_patterns_quickrun($file, $format = PATTERNS_FORMAT_UNKNOWN, $mode = 'php', $params = array()) {
	drush_print(dt('Patterns quick-run.'));
	$format = patterns_drush_get_file_format($file, $format);
	if (!$format) {
		return drush_set_error('Invalid pattern format. Aborting');
	}
	drush_print(dt('Format: ' . $format));

	return patterns_load_file_and_start_engine($file, $format, $mode, $params);
}

/**
 * Imports, enables, and runs the specified pattern file
 *
 * @param string pattern file pathname
 * @param string optional machine readable pattern name
 */
function drush_patterns_run($name, $format = PATTERNS_FORMAT_UNKNOWN, $mode = 'php', $params = array()) {
	drush_print(dt('Running pattern: ' . $name));
	
	$pattern = patterns_get_pattern($name);
	if (!$pattern) {
		if (!drush_file_not_empty($name)) {
			return drush_set_error(dt('Pattern was not found in the database nor in the file system.'));
		}
		
		if (!drush_confirm(dt("Pattern '$name' has not been imported. Do you want to import it now? "))) {
			return drush_print(dt('patterns-run has been canceled.'));
		}
			
		$format = patterns_drush_get_file_format($file, $format);
		if (!$format) {
			return drush_set_error('Invalid pattern format. Aborting');
		}
		$result = patterns_io_import_file($name, $format);
		if (!$result) {
			return drush_set_error(dt('Importing pattern failed'));
		}
		
		$name = patterns_io_get_name_from_file($name);
		// Check if the file has a valid extension
  	// and in case add the format at the end
  	if (!_patterns_io_file_has_valid_extension($name)) {
    	$name = $name . '.' . $format;
  	}
		
		$pattern = patterns_get_pattern($name); 
	}
	
	return drush_patterns_run_pattern($pattern, $mode, $params);
}

/**
 * Export data from the patterns components to file, zip archive, or database
 * 
 */
function drush_patterns_export() {
	drush_print(dt('Patterns export.'));
	
	if (!patterns_utils_is_module_enabled('patterns_export')) {
		drush_set_error(dt('Please enable module patterns_export first'));
		return FALSE;
	}
  
	$args = func_get_args();
	if (empty($args)) {
		return drush_set_error(dt('You must specify the export mode and the output file'));
	}
		
  $mode = array_shift($args);
	if (!patterns_export_is_valid_mode($mode)) {
		$modes_string = '[' . PATTERNS_EXPORT_TO_DB;
		$modes_string.= '|' . PATTERNS_EXPORT_TO_FILE;
		$modes_string.= '|' . PATTERNS_EXPORT_TO_ZIP . ']';
		return drush_set_error(dt('You must specify a valid export mode: ' . $modes_string));
	}
	
	// out file
	$out = array_shift($args);
	$format = PATTERNS_FORMAT_UNKNOWN;
	
	if ($mode === PATTERNS_EXPORT_TO_ZIP) {
		if (!class_exists('ZipArchive')) {
			drush_set_error(dt('Zip extension not found.'));
			return FALSE;
		}
		
		$format = patterns_io_get_format_from_file(NULL, array_shift($args));
		if (!$format) {
			return drush_set_error(dt('You must specify the format when exporting to Zip archive.'));
		}
	}
	
	$format = patterns_io_get_format_from_file($out, $format);
	if (!$format) {
		return FALSE;
	}

	patterns_io_load_components(TRUE);
	
  if (empty($args)) {
    drush_print(dt('Exporting all components'));
    $exports = patterns_moduletags_get_index(NULL, TRUE, TRUE);
	}
	else {
		$exports = patterns_drush_filter_components($args);
	}
	
	$result = patterns_export_start_engine($out, $exports, $info, $mode, $format, 'php');
  
	if (!$result) {
		drush_set_error(dt('An error occurred while exporting. Export may have failed.'));
	}
	else {
		drush_print(dt('Patterns exported successfully'));
	}
  
	//drush_print(print_r($export_functions, true));

}

/**
 * Imports the specified patterns file
 * 
 * @param string path to the patterns file
 * @param string optional machine readable name for the pattern you are importing
 * @return mixed bool|string FALSE upon failure; the name of the imported
 * pattern upon success
 */
function drush_patterns_import_file($file, $format = PATTERNS_FORMAT_UNKNOWN) {
	drush_print(dt("Importing local pattern file: " . $file));
	return patterns_io_import_file($file, $format);
}

/**
 * Imports the specified patterns file
 * 
 * @param string path to the patterns file
 * @param string optional machine readable name for the pattern you are importing
 * @return mixed bool|string FALSE upon failure; the name of the imported
 * pattern upon success
 */
function drush_patterns_import_file_remote($file, $format = PATTERNS_FORMAT_UNKNOWN) {
	drush_print(dt("Importing pattern file from remote uri: " . $file));
	return patterns_io_import_file_remote($file, $format);
}


// HELPER FUNCTIONS

function patterns_drush_get_file_format($file, $format = PATTERNS_FORMAT_UNKNOWN) {
	$format = patterns_io_get_format_from_file($file, $format, FALSE);
	if (!$format) {
		$format = drush_prompt(dt("Unable to recognize the pattern format. Please specify it")); 
		if (!patterns_parser_exists($format)) {
			return FALSE;
		}
	}
	return $format;
}

function drush_patterns_run_pattern($pattern, $mode = 'php', $params = array()) {
	$result = patterns_start_engine($pattern, $params, $mode);
	if ($result) {
		if ($mode === 'batch') {
			$batch = &batch_get();
			$batch['progressive'] = FALSE;
			drush_backend_batch_process();
		}
		drush_print(dt('Pattern execution started.'));
	}
	else {
		drush_set_error(dt('An error occurred while processing the pattern file.'));
	}
	return $result;
}

function patterns_load_file_and_start_engine($file, $format = PATTERNS_FORMAT_UNKNOWN, $mode, $params) {
	$pattern = patterns_io_load_pattern_from_file($file, $format);
	$params = array('run-subpatterns' => TRUE);
	drush_patterns_run_pattern($pattern, $mode, $params);
}

/**
 * Validate, and filters user input for export components
 * 
 * The input is of the type
 * 
 * 		component.tag.export_id
 * 
 * and returns a valid moduletags index containing only
 * the exporting functions
 * 
 * @param array $out
 */
function patterns_drush_filter_components($components) {
	$out = array();
	// Find the components.
  $all_components = patterns_io_list_components_names(TRUE);
  //drush_print(print_r($all_components, true));
      
    foreach ($components as $c) {
    	$func = NULL;
    	$func_name = NULL;
    	$t = NULL;
    	$tag = NULL;
    	$tag_name = NULL;
    	$export_func = NULL;
    	
      list($component, $tag, $export_func) = explode('.', $c);
      
      if (!in_array($component, $all_components)) {
      	drush_set_error(dt('Component not found: ' . $component));
      	continue;
      }
      
      $info = call_user_func($component . '_patterns');
      
      if (empty($info)) {
      	drush_set_error(dt('An error occurred while retrieving information from ' . $component));
      	continue;
      }
      
      // If no tag was specified, get all export functions
      if (empty($tag)) {
      	foreach($info as $tag => $t) {
      		if (!isset($t[PATTERNS_EXPORT]) || empty($t[PATTERNS_EXPORT])) {
      		 	continue;
      		}
      		foreach ($t[PATTERNS_EXPORT] as $func_name => $func) {

      			$out[$component][$tag][PATTERNS_EXPORT][$func_name] = $func; 
      		}
      		
      		// add files
      		if (isset($t[PATTERNS_FILES])) {
      			$out[$component][$tag][PATTERNS_FILES] = $t[PATTERNS_FILES]; 
      		}
      	}		
      	continue;
      }
      
      if (!isset($info[$tag])) {
      	drush_set_error(dt('Component "' . $component . '" has no tag "' . $tag . '" defined.'));
      	continue;
      }
      
      if (!isset($info[$tag][PATTERNS_EXPORT]) || empty($info[$tag][PATTERNS_EXPORT])) {
      	drush_set_error(dt('Component "' . $component . '", tag "' . $tag . '" has no export function defined.'));
      	continue;
      }
      
    	// If no export function was specified, get all export functions for the tag 
      if (empty($export_func)) {
      	foreach ($info[$tag][PATTERNS_EXPORT] as $func_name => $func) {
      		$out[$component][$tag][PATTERNS_EXPORT][$func_name] = $func;
      	}	
      	// add files
      	if (isset($info[$tag][PATTERNS_FILES])) {
        	$out[$component][$tag][PATTERNS_FILES] = $info[$tag][PATTERNS_FILES]; 
        } 
      	
      	continue;
      }
      
      if (!isset($info[$tag][PATTERNS_EXPORT][$export_func])) {
      	drush_set_error(dt('Component "' . $component . '", tag "' . $tag . '" has no xport function "' . $export_func . '"'));
      	continue;      	
      }
      
  		$out[$component][$tag][PATTERNS_EXPORT][$export_func] = $info[$tag][PATTERNS_EXPORT][$export_func];
  		
  		// add files
  		if (isset($info[$tag][PATTERNS_FILES])) {
      	$out[$component][$tag][PATTERNS_FILES] = $info[$tag][PATTERNS_FILES]; 
      } 
    }
    
	return $out;
}

// A Drush command callback.
function drush_patterns_test_regular_expression($test_re='') {
  
  module_load_include('module', 'simpletest');
  module_load_include('inc', 'simpletest', 'simpletest.pages');
  
  global $verbose, $color;
  $verbose = is_null(drush_get_option('detail')) ? FALSE : TRUE;
  $color = is_null(drush_get_option('color')) ? FALSE : TRUE;
  $error_on_fail = is_null(drush_get_option('error-on-fail')) ? FALSE : TRUE;
  if (!preg_match("/^\/.*\//", $test_re)) {
    $test_re = "/$test_re/";
  }
  
  $all_test_classes = simpletest_test_get_all();
  //print_r($all_test_classes);
  // Check that the test class parameter has been set.
  if (empty($test_re)) {
    drush_print("\nAvailable test groups & classes");
    drush_print("-------------------------------");
    $current_group = '';
    foreach ($all_test_classes as $class => $details) {
      if (class_exists($class) && method_exists($class, 'getInfo')) {
        $info = call_user_func(array($class, 'getInfo'));
        if ($info['group'] != $current_group) {
          $current_group = $info['group'];
          drush_print('[' . $current_group . ']');
        }
        drush_print("\t" . $class . ' - ' . $info['name']);
      }
    }
    return;
  }

   
  
  // Find test classes that match
  foreach ($all_test_classes as $class => $details) {
    if (class_exists($class) && method_exists($class, 'getInfo')) {
      if (preg_match($test_re, $class)) {
        $info = call_user_func(array($class, 'getInfo'));
        $matching_classes[$class] = $info;
      }
    }
  }

  $matching_classes = $all_test_classes['Patterns'];
 
  
  //print_r($matching_classes);
  
  $tests_list = array();
  foreach ($matching_classes as $class_name => $details) {
    if (class_exists($class_name)) {
      $tests_list[] = $class_name;
    }
  }
  //print_r($matching_classes);
  
  // Limit the number of tests for now
  $tests_list = array(
    //$tests_list[0], 
    $tests_list[1],
  );
  
  
  
  $total_tests = count($tests_list);

  
  $test_id = simpletest_run_tests($tests_list, 'drupal');
  
  $batch = &batch_get();
  $batch['progressive'] = FALSE;
  drush_backend_batch_process();
  
  
  $results = simpletest_result_get($test_id);
  
  //print_r($results);
  
  $analysis = _patterns_drush_test_check_results($results);
  
  //print_r($analysis);
  
  $fail = FALSE;
  
  foreach ($analysis as $area => $failed) {
    if ($failed > 0) {
      $fail = TRUE;
      drush_set_error($area . ' ' . $failed . ' tests failed');
    }
  }
  
  if ($failed) {
    exit(1);
  }
  else {
    exit(0);
  }
  
}

 function _patterns_drush_test_check_results($results) {
  $return = array();
  
  foreach($results as $area => $tests) {
    $return[$area] = 0;
    foreach($tests as $result) {
      if ($result->status == 'fail') {
        $return[$area]++;
        drush_set_error(drush_html_to_text($result->message));
//       print $area; // . + ': ' . $result->message;
      }
    }
  }
  
  return $return;
}

<?php

/**
 * 
 * Starts the exporting process in php execution mode
 * 
 * @param string $filename The name or path of the exported file 
 * @param array $tagmodules The index of modules/tags/exports_functions 
 * @param array $info The info section of the export
 * @param string $to Optional. A valid export destionation (e.g. zip,file,database).
 * 	 Defaults, 'db'.
 * @param string $format A valid patterns format (e.g. yaml,xml,php)
 * @param string $type Defines the type of actions (CREATE or MODIFY) that are going to be created during the export process
 * 
 */
function patterns_export_php($filename, $tagmodules, $info, $to, $format, $type = PATTERNS_EXPORT_TYPE_MODIFY) {

  //Adding the arguments we will be passing in an array
  $args = array();
  $args['type'] = $type;
  
  list($sections, $modules) = patterns_export_core_php($tagmodules, $args);
   
  return patterns_export_finalize_export($to, $sections, $info, $modules, $format, $filename);
}

/**
 * Constructs the abstract representation of the exported pattern
 * 
 * Returns the array of needed modules, and the actions of the 
 * pattern.
 * 
 * @param array $tagmodules An index of modules and tag to export
 * @param array $args Set of arguments passed to the export functions
 * @return array $result An array containing modules and actions
 * 
 */
function patterns_export_core_php($tagmodules, $args) {

  $result = array();
  $modules = array();
  foreach ($tagmodules as $module_name => $module) {
	  foreach ($module as $tag => $values) {
	   	if (!isset($values[PATTERNS_EXPORT]) || empty($values[PATTERNS_EXPORT])) {
	        continue;
	    }
			
	    if (isset($values[PATTERNS_FILES])) {
	      $files = (!is_array($values[PATTERNS_FILES])) ? array($values[PATTERNS_FILES]) : $values[PATTERNS_FILES];

	      foreach ($files as $file) {
	        require_once $file;
	      }
	    }
	
	    // TODO: the name of the component may be different from the
	    // the name of the module to enable. Cover this case
	    array_push($modules, $module_name);
	    
	    
	    if (!is_array($values[PATTERNS_EXPORT])) {
	      $sections[$tag] = call_user_func($values[PATTERNS_EXPORT], $args);
	    }
	    else {
	      foreach ($values[PATTERNS_EXPORT] as $f) {
	        $result[$tag] = isset($result[$tag]) ? $result[$tag] : array();
	        $sections[$tag] = array_merge($result[$tag], call_user_func($f, $args));
	      }
	    }
	
	  }
  }

  $result[] = $sections;
  $result[] = $modules;

  return $result;
}

// TODO: implement the Batch export. Not working at the moment

/**
 * Constructs the abstract representation of the exported pattern
 * invoking the Batch API
 * 
 * Returns the array of needed modules, and the actions of the 
 * pattern.
 * 
 * @param array $tagmodules An index of modules and tag to export
 * @return array $result An array containing modules and actions
 * 
 */
function patterns_export_batch($filename, $tagmodules, $info, $to, $format, $type = PATTERNS_EXPORT_TYPE_MODIFY) {

	// Registering $_SESSION variables
	$_SESSION['patterns_export_batch_info']['info'] = $info;
	$_SESSION['patterns_export_batch_info']['to'] = $to;
	$_SESSION['patterns_export_batch_info']['format'] = $format;
	$_SESSION['patterns_export_batch_info']['filename'] = $filename;
	$_SESSION['patterns_export_batch_info']['type'] = $type;
	
  $cur_sec = 1;
  $tot_sec = count($tagmodules);
  
	$result = array();
  $modules = array();
  $operations = array();
  foreach ($tagmodules as $module_name => $module) {
	  foreach ($module as $tag => $values) {
	   	if (!isset($values[PATTERNS_EXPORT]) || empty($values[PATTERNS_EXPORT])) {
	      continue;
	    }
			
	    $action_params = array($module_name, $tag, $values[PATTERNS_EXPORT], $type);
	    
	  	$action_params[] = isset($values[PATTERNS_FILES]) ? $values[PATTERNS_FILES]
	  																										 : array();
	    
			$operations[] = array('patterns_export_batch_core', $action_params);
	  }


    if (!isset($_SESSION['patterns_export_batch_info'])) {
    	$_SESSION['patterns_export_batch_info'] = array();
    }
  	if (!isset($_SESSION['patterns_export_batch_info']['modules'])) {
    	$_SESSION['patterns_export_batch_info']['modules'] = array();
    }
  
    
    array_push($_SESSION['patterns_export_batch_info']['modules'], $module_name);
   	$cur_sec++;
  }

  $batch = array(
    	'title' => t('Patterns export'),
      'progress_message' => t('Running action @current out of @total'),
      'finished' => 'patterns_export_batch_finish',
      'operations' => $operations,
    );
	batch_set($batch);
  return TRUE;
}

function patterns_export_batch_core($module_name, $tag, $export_funcs, $type = PATTERNS_EXPORT_TYPE_MODIFY, $files = NULL, &$context) { 
	$msg_ctx = t('%component: now processing %tag. ', array('%component' => $module_name, '%tag' => $tag));
	if ($_SESSION['patterns_export_batch_info']['to'] != PATTERNS_EXPORT_TO_DB) {
		$msg_ctx .= t('After the download has started, click !back.', array('!back' => l(t('here to go back'), 'admin/patterns/export')));
	}
	$context['message'] = $msg_ctx;
	
	// Do not process other actions if a fatal error already occurred
	if (!empty($context['results']['abort'])) {
		return;
	}
	
	// Since we are in the batch, we need to load some things once more.
  module_load_include('module', 'patterns_exports');
  patterns_io_load_components();
	
	$sections = array();
	
 	if (!empty($files)) {
		$files = (!is_array($files)) ? array($files) : $files;

	  foreach ($files as $file) {
	  	require_once $file;
	  }
	}
	
 
	if (!is_array($export_funcs)) {
		$export_funcs = array($export_funcs);
	}
	
	// Setting up export options
	$args = array(
		'type' => $type,
	);
	
	$result = array();
	foreach ($export_funcs as $f) {
		
		if (!function_exists($f)) {
			$context['results']['abort'] = 1;
			$context['results']['error_message'] = t('%module.%tag: export function %f not found.', array('%f' => $f,
																																			'%module' => $module_name,
																																			'%tag' => $tag));
			return FALSE;
		}
		$result[$tag] = isset($result[$tag]) ? $result[$tag] : array();
    $sections[$tag] = array_merge($result[$tag], call_user_func($f, $args));
	}
	
	// TODO: the name of the component may be different from the
	// the name of the module to enable. Cover this case
	if (!isset($context['results']['modules'])) {
		$context['results']['modules'] = array();
	}
	if (!isset($context['results']['sections'])) {
		$context['results']['sections'] = array();
	}
		
	array_push($context['results']['modules'], $module_name);
	$context['results']['sections'] = array_merge($context['results']['sections'], $sections);
	
}

/**
 * Finishes a batch operation.
 * @TODO Doc.
 */
function patterns_export_batch_finish($success, $results, $operations) {
  $modules = $_SESSION['patterns_export_batch_info']['modules'];
  
  if (empty($results['abort'])) {
    foreach ($modules as $m) {
      drupal_set_message(t('Component "@component" ran successfully.', array('@component' => $m)));
    }
    
    $to = $_SESSION['patterns_export_batch_info']['to'];
    $info = $_SESSION['patterns_export_batch_info']['info'];
 		$format = $_SESSION['patterns_export_batch_info']['format'];
 		$filename = $_SESSION['patterns_export_batch_info']['filename'];	
    unset($_SESSION['patterns_export_batch_info']);
		
   	return patterns_export_finalize_export($to, $results['sections'], $info, $results['modules'], $format, $filename);
  }
  else {
    drupal_set_message(t('An error occurred during the export.') , 'error');
    drupal_set_message($results['error_message'], 'error');
  	unset($_SESSION['patterns_export_batch_info']);
		return FALSE;
  }
  
}
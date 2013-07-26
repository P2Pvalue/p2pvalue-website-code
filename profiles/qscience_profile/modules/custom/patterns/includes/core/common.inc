<?php

/**
 * @file
 * The common functions used by the Batch and PHP running modes.
 */

/**
 * Preparing and validating the action tags as they are written in the pattern
 * file. Concretely, it invokes operations 'prepare', and 'validate' on the
 * pattern component.
 *
 * @param array $actions An array of actions. Each action is an associative
 *   array with keys 'action' and 'data'.
 * @param array $actions_map
 *
 * @return array $results array containing the description of eventual errors
 * @TODO Doc.
 */
function patterns_prepare_action(&$action, &$data) {

  $status = PATTERNS_SUCCESS;
  $message = '';

  if (empty($action)) {
    return patterns_results(PATTERNS_ERR, t('An unspecified error occurred. No valid action found'));
  }

  // Keep a list of which modules handle what tags.
  $tag_modules = patterns_tagmodules_get_index($data);

  if (!array_key_exists($data['tag'], $tag_modules)) {
  	$msg = t('<%tag> is not a valid tag', array('%tag' => $data['tag']));
  	return patterns_results(PATTERNS_ERR, $msg);
  }

  $err_prefix = t('Errors during pre-processing.');
  
  /////////////////////////////////////////////
  // Prepare actions for validation/processing
  /////////////////////////////////////////////

  $results = patterns_invoke('prepare', $action, $data);

  if (!patterns_error_check_results($results, $err_prefix)){
  	return $results;
  }
  
  $key = &$action;
  $data = &$data;
  
	//////////////////////////////////////////////////
  // Validate tags with their appropriate components
  //////////////////////////////////////////////////
  $results = patterns_invoke('validate', $key, $data);

  patterns_error_check_results($results, $err_prefix);

  return $results;
}

/**
 * Setup and run an action.
 *
 * @param $action
 * @param $identifiers
 * @param $place
 * @param $actions_map
 *
 * @return
 * @TODO Doc.
 */
function patterns_implement_action($action, $data, &$identifiers, $place = 0, $actions_map = NULL) {

  patterns_set_error_handler();

  $status = PATTERNS_SUCCESS;
  $msg = '';

  // Refresh the list of forms based on the data we have.
  $tagmodules = patterns_tagmodules_get_index($data);
  $tag_info = patterns_tagmodules_filter($tagmodules, $data['tag']);
  
  $tag_description = isset($tag_info['description']) ? $tag_info['description'] : 'no description';
   
  $form_ids = $tag_info[$action];

  
  
  // Gather info about the action
  $action_location = patterns_locate_action($place, $actions_map);
  $index = $action_location['key'] + 1;
  $pattern_title = $action_location['title'];
  $pattern_file = $action_location['file'];

  // See which forms to execute sequentially. This is similar to what used to be called 'form_ids'.
  $results = patterns_invoke('callbacks', $action, $data);

  if (!patterns_error_check_results($results)) {
    return $results;
  }

  // If no special callback was returned, we use the functions
  // specified in the hook_patterns for the action
  if (!isset($results['result']) || empty($results['result'])) {
  	$funcs = $form_ids;
  }
  else {
  	$funcs = $results['result'];
  }
  
  // Auto include FILES
  if (!empty($tag_info[PATTERNS_FILES])) {
    foreach ($tag_info[PATTERNS_FILES] as $file) {
      require_once($file);
    }
  }
  
  // Build the action
  foreach ($funcs as $func) {
    
    $form_id = $func;
    $clone = $data; // TODO: does this prevent subsequent form_ids' communication?

    $results['action_descriptions'][$place][] = $tag_info['descr'];

    // If tokens are enabled, apply tokens to the action values
    // before processing.
    if (module_exists('token')) {
      _patterns_recurse_tokens($clone, $identifiers);
    }

    /////////////////////////////////////////////////////
    // BUILD: Get the form data for the action. This can return either
    // just the form values, or the full form_state object.
    /////////////////////////////////////////////////////
    $results = patterns_invoke('build', $action, $clone, $form_id);

    if (!patterns_error_check_results($results)) {
      return $results;
    }

    // If no special callback was returned, we use the data as it is
    if (!isset($results['result']) || empty($results['result'])) {
	  	$form_obj = $clone;
	  }
	  else {
	  	$form_obj = $results['result'];
	  }

    // We check for the 'storage' and 'submitted' values in the object to see
    // if it is a form_state instead of form_values. There could be a better way
    // to do this.
    if (array_key_exists('submitted', (array) $form_obj) && array_key_exists('storage', (array) $form_obj)) {
      $action_state = $form_obj;
      $need_buildinfo = TRUE;
    }
    else {

      if (!isset($tag_info['files'])) {
        $files = array();
      }
      else {
        $files = $tag_info['files'];
      }

      $action_state = array(
            'storage' => NULL,
            'submitted' => FALSE,
            'build_info' => array('files' => $files),
            'values' => $form_obj,
      );
      $need_buildinfo = TRUE;
    }

    ////////////////////////////////////////////////////
    // Get any extra parameters required for the action
    ////////////////////////////////////////////////////
    $results = patterns_invoke('params', $action, $clone, $form_id, $action_state);

    if (!patterns_error_check_results($results)) {
      return $results;
    }

    // A single, simple value can be returned as a parameter, which is then
    // put into an array here.
    if (!isset($results['result'])) {
      $results['result'] = NULL;
    }
    if (!is_array($results['result'])) {
      $params = array($results['result']);
    }
    else {
      $params = $results['result'];
    }

    if ($need_buildinfo) {
      $action_state['build_info']['args'] = $params;
    }

    ///////////////////
    // Execute action: pass action_state and params to a form
    ///////////////////
    patterns_execute_action($form_id, $action_state, $params);
    
    if ($errors = form_get_errors()) {
      drupal_set_message(print_r($errors, true));
      $errors_tokens =  array(
        '%num' => $index, 
        '%action' => $tag_description,
        '%title' => $pattern_title, 
        '%errors' => str_replace('][', '->', implode(', ', array_keys($errors))),
      );
      
      $msg = t('Above error(s) occurred while executing action #%num (%action) in %title pattern. Error location(s) are: %errors', $errors_tokens);
      return patterns_results(PATTERN_ERR, $msg);
    }
    
    ////////////////////
    // CLEAN UP: Let a component cleanup after each action
    ////////////////////
    $results = patterns_invoke('cleanup', $action, $clone, $form_id, $action_state);

    // Do not return here, just print on screen
    patterns_error_check_results($results);
    
  } // End: Form_id execution

  // Clear the cache in case it causes problems
  cache_clear_all();

  patterns_restore_error_handler();

  return patterns_results();
}

/**
 * Execute an action.
 *
 * @param mixed $form_id The name of function to call.
 * @param array $form_state The form_state object.
 * @param array $params Extra parameters to be passed to the form function.
 */
function patterns_execute_action($form_id, &$form_state, $params) {
  
	patterns_executing(TRUE);
	
	//$args = is_array($params) ? array_merge(array($form_state), $params)
	//													: array($form_state);
	
	// We don't know here whether we are executing a callback
	// or building a form														
	//$result = call_user_func_array($form_id, $args);
	//$result1 = $form_id($form_state, $params);

	
	$result = drupal_retrieve_form($form_id, $form_state, $params); 
	
	// Check whether it is callback or a form submission
	if (_patterns_is_patterns_results($result)) {
		// Callback: nothing to do anymore
		if (isset($result['msg'])) {
    	drupal_set_message($result['msg']);
    }
	}
	else {
		// Form: we just built it, and not we need to submit it
      	
		// Make sure we always have a clear cache for everything.
  	// Code taken from drupal_flush_all_caches().

  	// Don't clear cache_form - in-progress form submissions may break.
  	// Ordered so clearing the page cache will always be the last action.
	  $core = array('cache', 'cache_block', 'cache_filter', 'cache_page');
	  $cache_tables = array_merge(module_invoke_all('flush_caches'), $core);
	  foreach ($cache_tables as $table) {
	    cache_clear_all('*', $table, TRUE);
	  }
	
	  // This is the old way of executing the form: it is slower
	  // because it needs to rebuilds it
	  //////////////////////////////////////////////////////////////
	  //$args = array_unshift($args, $form_id);
  	//$return = call_user_func_array('drupal_form_submit', $args);
	  ///////////////////////////////////////////////////////////////
	  
		$form = $result;
		
		// Merge in default values.
  	$form_state += form_state_defaults();
  	$form_state['input'] = $form_state['values'];
  	$form_state['programmed'] = TRUE;
		// Programmed forms are always submitted.
  	$form_state['submitted'] = TRUE;
		
		// Reset form validation.
    $form_state['must_validate'] = TRUE;
    form_clear_error();

    drupal_prepare_form($form_id, $form, $form_state);
    drupal_process_form($form_id, $form, $form_state);
		
	}
	
  patterns_executing(FALSE);
}

/**
 * Outputs information about a successful execution of a section of a pattern
 * and updates its db status
 * 
 * @param mixed $pid The pid of the pattern
 * @param mixed $pattern_title The title of the pattern
 * @param mixed $section The title of the section executed
 * @param mixed $msg Optional. An additional message to be displayed
 */
function _patterns_section_success($pid, $pattern_title, $section, $msg = NULL) {
	$params =  array(
		'@pattern' => $pattern_title,
    '@section' => $section,
 	);
	drupal_set_message(t('Section "@section" of pattern "@pattern" ran successfully.', $params));
	if (!empty($msg)) {
 		drupal_set_message($msg);	
 	}
  
	$query_params = array(
		':time' => time(), // Note: time() != $_SERVER['REQUEST_TIME']
    ':pid' => $pid,
    ':en' => PATTERNS_STATUS_ENABLED,
  );
  
  db_query("UPDATE {patterns} SET status = :en, enabled = :time WHERE pid = :pid", $query_params);
}

/**
 * Outputs information about an un-successful execution of a section of a pattern
 * 
 * @param mixed $pid The pid of the pattern
 * @param mixed $pattern_title The title of the pattern
 * @param mixed $section The title of the section executed
 * @param mixed $msg Optional. An additional message to be displayed
 */
function _patterns_section_fail($pid, $pattern_title, $section, $msg = NULL) {
	$params =  array(
		'@pattern' => $pattern_title,
    '@section' => $section,
 	);  
 	drupal_set_message(t('Section "@section" of pattern "@pattern" ran with errors. Check the error messages to get more details.', $params), 'warning');
 	if (!empty($msg)) {
 		drupal_set_message($msg, 'error');	
 	}
}


function patterns_execute_action1($form_id, &$form_state, $params) {
  // Make sure we always have a clear cache for everything.
  // Code taken from drupal_flush_all_caches().

  // Don't clear cache_form - in-progress form submissions may break.
  // Ordered so clearing the page cache will always be the last action.
  $core = array('cache', 'cache_block', 'cache_filter', 'cache_page');
  $cache_tables = array_merge(module_invoke_all('flush_caches'), $core);
  foreach ($cache_tables as $table) {
    cache_clear_all('*', $table, TRUE);
  }

  $args = array($form_id, &$form_state);

  if (is_array($params)) {
    $args = array_merge($args, $params);
  }

  patterns_executing(TRUE);

  // If we are in batch mode, trick the form api to think
  // otherwise to avoid potential problems
  //$batch =& batch_get();
  //$batch_clone = $batch;
  //$batch = NULL;
  $return = call_user_func_array('drupal_form_submit', $args);
  // TODO: check for errors?

  patterns_executing(FALSE);

  //$batch = $batch_clone;

}

/**
 * Group the action and the data fields.
 *
 * @param array $actions The array of actions.
 */
function patterns_reformat_actions(&$actions) {
  $valid_actions = patterns_actions();
  $newactions = array();

  foreach ($actions as $key => $value) {
    //$found = FALSE;
    foreach ($value as $akey => $avalue) {
      if (!array_key_exists($akey, $valid_actions)) {
        // Should not happen.
        drupal_set_message(t("Invalid action: %action.", $akey), 'error');
      }
      else {
          $newactions[] = array('action' => $akey, 'data' => $avalue);
      }
    }
  }
  $actions = $newactions;
}

/**
 * Scan the actions_map array and return info about the file the action was coming
 * from with the following info:
 *
 * - index of the action within the batch_set
 * - title of the pattern
 * - file of the pattern
 *
 * @param mixed $key The key of the action in the actions_map array.
 * @param array $actions_map
 *   Array containing information about of all the actions of the batch_set.
 *
 * @return array $result Concise description of the action.
 */
function patterns_locate_action($key, $actions_map) {
  $result['key'] = $actions_map['map'][$key]['index'];
  $result['title'] = $actions_map['patterns'][$actions_map['map'][$key]['pid']]['title'];
  $result['file'] = $actions_map['patterns'][$actions_map['map'][$key]['pid']]['file'];
  return $result;
}

/**
 * Keeps tracks of whether the patterns engine is currently executing a pattern
 *
 * @param boolean $b Set the status of the engine
 * @return boolean The status of the engine
 */
function patterns_executing($b = NULL) {
  static $executing = FALSE;

  if (is_bool($b)) {
    $executing = $b;
  }

  return $executing;
}

/**
 * Heuristically determines if a function is a form or not
 * 
 * @param mixed $form_id The name of the function
 * @return boolean TRUE, if the function name maps to an existing form
 */
function patterns_is_form($form_id = NULL, $form_state = array()) {
  
 
  $result = drupal_retrieve_form($form_id, $form_state);    
  return (isset($result) && is_array($result) && count($result) > 1);
}
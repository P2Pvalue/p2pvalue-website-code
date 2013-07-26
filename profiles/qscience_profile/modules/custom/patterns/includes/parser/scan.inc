<?php

/**
 * @file
 *
 * Helping functions for the parser.
 *
 * Functions related to parsing the abstract array structure
 * 
 * TODO: Make a distinction between the cases where there are warnings only, errors or no messages at all
 *
 */


/**
 * Scans a pattern and returns a brief summary of its properties.
 *
 * @param array $pattern 
 * 	 Pattern array obtained by parsing pattern file.
 * @param bool $include (optional) 
 * 	 If TRUE, actions can be found outside of sections. Default FALSE.
 * @param integer $level (optional) 
 * 	 The level of of the analysis. Defaults PATTERNS_VALIDATE_ALL
 *
 * @return array $result Summary of the pattern.
 *
 * @TODO Expand this function to include much more detailed validation.
 * @TODO Rewrite in an object oriented fashion
 *
 * @see _patterns_scan_action()
 * @see _patterns_scan_analyze_patternscan()
 *
 */
function patterns_scan_pattern($pattern, $include = FALSE, $level = PATTERNS_VALIDATE_ALL) {	
  $generic_error = t('pattern not loaded correctly');
  $result = array(
                  'info' => 0,
                  'modules' => 0,
                  'empties' => array(),
                  'invalid_actions' => array(),
                  'extra_actions' => array(),
                  'missing_tag' => array(),
                  'other_sections' => array(),
                  'contain_includes' => FALSE,
                  'include_scans' => array(),
                  'generic_errors' => array(),
				  'unknown_tag' => array(),
                  'tag_syntactic_errors' => array(),
                  'tag_syntactic_warnings' => array(),
                  'tag_semantic_warnings' => array(),
  );

  if (empty($pattern)) {
    $result['empties'][] = $generic_error;
    return $result;
  }

  // Patterns is valid if contains:

  //  - exactly 1 topmost section 'info'
  //  - at most 1 topmost section 'modules'
  //  - at least 1 more other sections
  //
  // All the sections must be a non-empty array.
  // If a section is called 'include;
  // it means that we are including a new pattern
  // and we apply patterns_scan_pattern recursively.

  foreach ($pattern as $section_name => $section) {

    // INFO
    if ($section_name === 'info') {
      $result['info']++;
      continue;
    }

    // MODULES
    if ($section_name === 'modules') {
      $result['modules']++;
      continue;
    }

    // SECTIONS or ACTIONS (if it is an include)
    if (!_patterns_scan_is_section($section)) {
      if (!$include) {
        $result['generic_errors'][] = $section_name ;
        continue;
      }

      $result = _patterns_scan_action($section, $result);
      continue;
    }

    // SECTIONS: invalid
    // included patterns can be just the name or the id
    // of the pattern
    if (!is_array($section) && !$include) {
      // If the YAML is not loaded correctly you get 0.
      $result['empties'][] = ($section_name === 0) ? $generic_error
                                                   : $section_name;
      continue;
    }

    // SECTIONS: valid
    //$resultactions = array();
    //$newactions = array();

    // Collect info specific to each section
    $section_info = array();
    $section_info[PATTERNS_CREATE] = 0;
    $section_info[PATTERNS_MODIFY] = 0;
    $section_info[PATTERNS_DELETE] = 0;
    $section_info[PATTERNS_INCLUDE] = 0;

    foreach ($section as $key => &$action) {
      $action_name = key($action);
      if (isset($section_info[$action_name])) {
        // Incremente the count of valid actions
        $section_info[$action_name] = $section_info[$action_name]+1;
      }
      //Validation rules provides by the component are checked in this function
      $result = _patterns_scan_action($action, $result, $level);
    }

    $section_info_str = '(' . PATTERNS_CREATE . ':' . $section_info[PATTERNS_CREATE] . ', ' .
                              PATTERNS_MODIFY . ':' . $section_info[PATTERNS_MODIFY] . ', ' .
                              PATTERNS_DELETE . ':' . $section_info[PATTERNS_DELETE] . ', ' .
                              PATTERNS_INCLUDE . ':' . $section_info[PATTERNS_INCLUDE] . ')';

    $result['other_sections'][$section_name] =  $section_info_str;

  }

  return $result;
}

/**
 * Analyze the result of a call to patterns_scan_pattern, and check whether
 * the pattern was valid.
 *
 * @param array $analysis 
 * 	 The analysis array obtained from  patterns_scan_pattern.
 * @param bool $include (optional)
 * 	 If TRUE, it means that the pattern to scan is an include, 
 * 	 and a laxer scan is performed. E.g. Info section can be missing, 
 * 	 actions can be outside of a section. Default FALSE.
 * @param integer $level (optional) 
 * 	 The level of of the analysis. Defaults PATTERNS_VALIDATE_ALL
 *
 * @return TRUE if valid, FALSE otherwise
 */
function _patterns_scan_analyze_patternscan($patternscan, $include = FALSE, $level = PATTERNS_VALIDATE_ALL, $br = '<br/>') {
  $msgs = array();

  if ($patternscan['info'] == 0 && !$include) {
    $msgs[] = t('The info section is missing.');
  }

  if ($patternscan['info'] > 1) {
    $msgs[] = t('Pattern can contain only one \'info\' section.');
  }

  if ($patternscan['modules'] > 1) {
    $msgs[] = t('Pattern can contain only one \'modules\' section.');
  }

  if (count($patternscan['other_sections']) == 0 && !$include) {
    $msgs[] = t('Pattern does not contain any actions.');
  }

  if (count($patternscan['generic_errors']) != 0) {
    $msgs[] = t('Generic errors in the patterns were found. Probably a tag was misplaced. Please verify: !found',
            array('!found' => implode(', ', $patternscan['generic_errors'])));
  }

  if (count($patternscan['invalid_actions']) != 0) {
    $invalidactions = array();
    foreach ($patternscan['invalid_actions'] as $key => $value) {
      $invalidactions[] = $value['key'];
    }
    $msgs[] = t('Only %actions are valid actions. Found: %found.',
            array('%actions' => implode(', ', patterns_actions()),
                  '%found' => implode(', ', $invalidactions)));
  }

  if (count($patternscan['extra_actions']) != 0) {
    $extraactions = array();
    foreach ($patternscan['extra_actions'] as $key => $value) {
      $extraactions[] = $value['key'];
    }
    $msgs[] = t('Extra actions have been found on one level: %found.',
            array('%found' => implode(', ', $extraactions)));
  }
  
  

  // TODO: This is not yet working properly. Check when it is applicable!
  if (count($patternscan['missing_tag']) != 0) {
    foreach ($patternscan['missing_tag'] as $key => $value) {
      $msgs[] = t('A mandatory \'tag\' was missing for action %action.', array('%action' => $value['key']));
    }
  }
  
  if (count($patternscan['empties']) > 0) {
    $msgs[] = t('Pattern contains empty sections or actions:') . implode(', ', $patternscan['empties']);
  }
  
  // INCLUDE LEVEL
  ////////////////
 	if ($level < PATTERNS_VALIDATE_INCLUDE) {
  	return $msgs;
  }
  
	if (count($patternscan['include_scans']) > 0) {
    foreach ($patternscan['include_scans'] as $i) {
      //$msgs[] = print_r($patternscan['includes'], true);
      $msgs = array_merge($msgs, _patterns_scan_analyze_patternscan($i, TRUE));
    }
  }
  
  // TAG EXISTS LEVEL
  ///////////////////
  if ($level < PATTERNS_VALIDATE_TAG_EXISTS) {
  	return $msgs;
  }
  
  if (count($patternscan['unknown_tag']) != 0) {
  	$utags = implode(', ', $patternscan['unknown_tag']);
  	$msgs[] = t('The following unknown tags were found: %utags.', array('%utags' => $utags));
  }
  
  // FULL SEMANTIC VALIDATION:
  // This level ensures the pattern will run without execution errors
  ////////////////////////////
  if ($level < PATTERNS_VALIDATE_TAG_SYNTAX) {
    return $msgs;
  }

  $tag_errors = NULL;
  //Go through all the semantic warnings performing a code-description conversion
  if (count($patternscan['tag_syntactic_warnings']) > 0) {
    $tag_errors = $patternscan['tag_syntactic_warnings'];
  }else if (count($patternscan['tag_syntactic_errors']) > 0) {
    $tag_errors = $patternscan['tag_syntactic_errors'];
  }
  
  if (isset($tag_errors)) {
    $warnings = '';
    foreach ($tag_errors as $warn_data) {
    
      $warnings.= '[' . $warn_data['action'] . ':' . $warn_data['tag'] . '] ';
    
      $warnings.= $warn_data['msg'] . $br;
       
    }
    $msgs[] = t('Tag syntactic errors/warnings: ') . $br . $warnings;
  }
    
  
  
  
  // FULL SEMANTIC VALIDATION:
  // This level ensures the pattern will run without execution errors
  ////////////////////////////
  if ($level < PATTERNS_VALIDATE_SEMANTIC) {
  	return $msgs;
  }
  
  //Go through all the semantic warnings performing a code-description conversion
  if (count($patternscan['tag_semantic_warnings']) > 0) {
  	$warnings = '';
  	foreach ($patternscan['tag_semantic_warnings'] as $warn_data) {
  		
  		$warnings.= '[' . $warn_data['action'] . ':' . $warn_data['tag'] . '] ';
  		
  		$type = key($warn_data['warning']);
  		
  	  switch ($type) {
  	    case -1:
  	      $warnings .= t('[Already defined element]: ');
  	      break;
        case -2:
          $warnings .= t('[Undefined element]: ');
          break;
        case -3:
          $warnings .= t('[Unmet dependency]: ');
          break;
        case -4:
          $warnings .= t('[Remaining dependency]: ');
          break;
        case -5:
          $warnings .= t('[Not unique alias]: ');
          break;
  	  }
  	  
  	  $warnings.= reset($warn_data['warning']) . $br;
  	  
  	}
  	$msgs[] = t('Tag semantic warnings: ') . $br . $warnings;
  }

  return $msgs;
}

/**
 * Analyze the result of a call to patterns_scan_pattern, and check whether
 * the pattern was valid.
 *
 * @param array $patternscan 
 * 	 The analysis array obtained from  patterns_scan_pattern.
 * @param bool $include (optional)
 * 	 If TRUE, it means that the pattern to scan is an include, 
 * 	 and a laxer scan is performed. E.g. Info section can be missing, 
 * 	 actions can be outside of a section. Default FALSE.
 * @param integer $level (optional) 
 * 	 The level of of the analysis. Defaults PATTERNS_VALIDATE_ALL
 * @param $display_errors (optional)
 * 	 If TRUE, errors are displayed with drupal_set_message. Defaults FALSE
 *
 * @return bool TRUE if valid, FALSE otherwise.
 */
function _patterns_scan_validate_patternscan($patternscan, $include = FALSE, $level = PATTERNS_VALIDATE_ALL, $display_errors = FALSE) {
  $analysis = _patterns_scan_analyze_patternscan($patternscan, $include, $level);
  if (empty($analysis)) {
    return TRUE;
  }
  if ($display_errors) {
    drupal_set_message(t('Error(s) and/or warning(s) while processing pattern:') . '<ul><li>' . implode('</li><li>', $analysis) . '</li></ul>', 'error');
  }
  return FALSE;
}

/**
 * Determines whether an array is actually a section.
 *
 * A section is an array, whose first element is a valid
 * action.
 *
 * @param array $section The array to check
 *
 * @return Boolean TRUE if the array is a section
 *
 * @see _patterns_scan_is_action()
 */
function _patterns_scan_is_section($section = NULL) {
  if (is_null($section)) return FALSE;
  if (!is_array($section)) return FALSE;
  $action = array_pop($section);
  return _patterns_scan_is_action($action);
}

/**
 * Determines whether an array is actually an action.
 *
 * An action is an array, whose key name is a valid
 * action name.
 *
 * @param array $action The array to check
 *
 * @return Boolean TRUE if the array is an action
 *
 * @see _patterns_scan_is_section()
 */
function _patterns_scan_is_action($action = NULL) {
  if (is_null($action)) return FALSE;
  if (!is_array($action)) return FALSE;
  return patterns_parser_is_valid_action_name(key($action));
}

/**
 * Helper function for scanning patterns.
 *
 * Scans an action and add the result of the scan to the $result
 * array.
 *
 * @param array $action The action to scan
 * @param array $result (optional) The array in which inserting
 *   the scan results.
 * @param integer $level (optional) 
 * 	 The level of of the analysis. Defaults PATTERNS_VALIDATE_ALL
 * 
 * @return array $result The results of the scan.
 *
 * @see patterns_scan_pattern()
 * @see _patterns_scan_analyze_patternscan()
 *
 */
function _patterns_scan_action($action, $result = array(), $level = PATTERNS_VALIDATE_ALL) {

  $key = key($action);
  $action = current($action);
  $valid_actions = patterns_actions();
  if (!array_key_exists($key, $valid_actions)) {
    $result['invalid_actions'][] = array('actionid' => $action, 'key' => $key, 'value' => $action);
    return $result;
  }

  if (empty($action)) {
    $result['empties'][] = $key;
    return $result;
  }

  if (!is_array($action)) {
    $result['generic_errors'][] = $key . ': ' . $action;
    return $result;
  }

  // Make sure there is only one valid action in each array element.
  // NOTE: Having additional valid actions will work, but is discouraged,
  // and therefore undocumented.
  $found = FALSE;

  // Report additional valid actions.
  if ($found) {
    $result['extra_actions'][] = array('actionid' => $action, 'key' => $key, 'value' => $action);
  }
  
  if ($level <= PATTERNS_VALIDATE_SYNTAX) {
  	return $result;
  }

  // Do action specific checkings
  ////////////////////////////////

  if ($key === PATTERNS_INCLUDE) {
    $result['contain_includes'] = TRUE;

    if (!isset($action['pattern'])) {
      $result['missing_tag'][] = array('actionid' => $action, 'key' => $key, 'value' => $action);
      return $result;
    }
		
    // only if the pattern is hard-coded, then scan it
    if (is_array($action['pattern'])) {
      $result['include_scans'][] = patterns_scan_pattern($action['pattern'], TRUE);
    }
    return $result;
  }

  // Check if 'tag' is present.
  if (!isset($action['tag']) || !is_string($action['tag'])) {
    $result['missing_tag'][] = array('actionid' => $action, 'key' => $key, 'value' => $action);
  }
  
  // Try to validate the tag with the component
  else if ($level >= PATTERNS_VALIDATE_TAG_EXISTS) {
  	
  	$tag = $action['tag'];
  	
  	// We force to rebuild the index and reload the components
	  // because this method may be invoked as a service
	  $tag_modules = patterns_tagmodules_get_index($action, true, true); 

	  if (!isset($tag_modules[$tag])) {
	  	$result['unknown_tag'][] = $tag;
	  }
	  
    
    else if ($level >= PATTERNS_VALIDATE_TAG_SYNTAX) {
    	
    	patterns_invoke('prepare', $key, $action);
		$validation = patterns_invoke('validate', $key, $action);
		
		if ($validation['status'] == PATTERNS_WARN) {
		  $result['tag_syntactic_warnings'][] = array('action' => $key, 'tag' => $tag, 'msg' => $validation['msg']);
		}
		else if ($validation['status'] == PATTERNS_ERR) {
		  $result['tag_syntactic_errors'][] = array('action' => $key, 'tag' => $tag, 'msg' => $validation['msg']);
		}
		
		//Include semantic warnings if requested
		if ($level >= PATTERNS_VALIDATE_SEMANTIC) {
		  if (isset($validation['result'])) {
		    foreach ($validation['result'] as $semantic_warning) {
		      $result['tag_semantic_warnings'][] = array('action' => $key, 'tag' => $tag, 'warning' => $semantic_warning);
		    }
		  }
		}
		
			
			
			
  
    }
  	
  }
  //else {
  //  $newactions[] = array('action' => $key, 'data' => $action);
  //}
  $found = TRUE;

  return $result;
}

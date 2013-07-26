<?php
/*
 * @file
 * Patterns component for pathauto.
 */
function pathauto_patterns() {
  $files = array('sites/all/modules/pathauto/pathauto.admin.inc', 'modules/path/path.admin.inc');

  $actions['pathauto_setting'] = array(
    PATTERNS_INFO => t('Setting pathauto'),
    PATTERNS_MODIFY => array('pathauto_settings_form'),
    PATTERNS_FILES => $files,
     PATTERNS_EXPORT => array(
                            PATTERNS_EXPORT_ALL => 'pathauto_patterns_export_all_pathauto_setting',
                            ),
  );
/* delete tag url_alias?
 *  $actions['url_alias'] = array(
    PATTERNS_INFO => t('Create/Edit/Delete URL aliases'),
    PATTERNS_CREATE => array('path_admin_form'),
    PATTERNS_MODIFY => array('path_admin_form'),
    PATTERNS_DELETE => array('path_admin_delete_confirm'),
    PATTERNS_FILES => $files,
    PATTERNS_EXPORT => array(
                            PATTERNS_EXPORT_ALL => 'pathauto_patterns_export_all_url_alias',
                            ),
  );*/
  $actions['pathauto_patterns'] = array(
    PATTERNS_INFO => t('setting patterns for auto URLs aliases'),
    PATTERNS_MODIFY => array('pathauto_patterns_form', 'pathauto_bulk_update_form'),
    PATTERNS_FILES => $files,
     PATTERNS_EXPORT => array(
                            PATTERNS_EXPORT_ALL => 'pathauto_patterns_export_all_pathauto_patterns',
                            ),

  );

  return $actions;

}



function pathauto_patterns_export_all_pathauto_setting($args = NULL, &$result = NULL) {
   //$info = pathauto_patterns();
   //$form_id = current($info['url_alias'][PATTERNS_MODIFY]);
   module_load_include('inc', 'pathauto');
  $result = array();
  $data = array('tag' => 'pathauto_setting');
  $data['pathauto_verbose']=variable_get('pathauto_verbose', FALSE);
  $data['pathauto_separator'] = variable_get('pathauto_separator', '-');
  $data['pathauto_case'] = variable_get('pathauto_case', PATHAUTO_CASE_LOWER);
  $data['pathauto_max_length'] = variable_get('pathauto_max_length', 100);
  $data['pathauto_max_component_length'] = variable_get('pathauto_max_component_length', 100);
  $data['pathauto_update_action'] = variable_get('pathauto_update_action', PATHAUTO_UPDATE_ACTION_DELETE);
  $data['pathauto_transliterate'] =variable_get('pathauto_transliterate', FALSE) && module_exists('transliteration');
  $data['pathauto_reduce_ascii'] = variable_get('pathauto_reduce_ascii', FALSE);
  $data['pathauto_ignore_words'] = variable_get('pathauto_ignore_words', PATHAUTO_IGNORE_WORDS);
  $punctuation = pathauto_punctuation_chars();
  foreach ($punctuation as $name => $details) {
    $details['default'] = PATHAUTO_PUNCTUATION_REMOVE;
    if ($details['value'] == variable_get('pathauto_separator', '-')) {
      $details['default'] = PATHAUTO_PUNCTUATION_REPLACE;
    }
    $data['pathauto_punctuation_' . $name] =variable_get('pathauto_punctuation_' . $name, $details['default']);
  }
  $action = array(PATTERNS_MODIFY => $data);
  array_push($result, $action);
  
  return $result;
}
/*function pathauto_patterns_export_all_url_alias($args = NULL, &$result = NULL) {
   //$info = pathauto_patterns();
   //$form_id = current($info['url_alias'][PATTERNS_MODIFY]);
  //$loop = TRUE;
  $paths = db_query("SELECT * FROM {url_alias}", array(), array('fetch' => PDO::FETCH_ASSOC))->fetchAll();
  $result = array();
  switch ($args['type']) {
    case PATTERNS_CREATE :
  		foreach ($paths as $path) {
    	//	$path = (array) $path;
    		$data = array('tag' => 'url_alias');
    		$data['source']=$path->source;
    		$data['alias']=$path->alias;
    		$data['language']=$path->language;
    		//$data = array_merge($data, $path);
    		$action = array(PATTERNS_CREATE => $data);
    		array_push($result, $action);
   		//  $action = array(PATTERNS_PATTERNS_CREATE => $data);
   		// array_push($result, $action);

    		// It is important to user array merge. Pushing is not enough
    		//$result = array_merge($result, patterns_export_actions_from_form($form_id, $path, 'url_alias', PATTERNS_MODIFY));
  		}
  		break;
  	case PATTERNS_MODIFY :
   		foreach ($paths as $path) {
    	//	$path = (array) $path;
    		$data = array('tag' => 'url_alias');
    		$data['source']=$path->source;
    		$data['alias']=$path->alias;
    		$data['language']=$path->language;
    		//$data = array_merge($data, $path);
    		$action = array(PATTERNS_MODIFY => $data);
    		array_push($result, $action);
    	}
  		break;
  }
  return $result;
}*/
function pathauto_patterns_export_all_pathauto_patterns($args = NULL, &$result = NULL) {
   //$info = pathauto_patterns();
   //$form_id = current($info['url_alias'][PATTERNS_MODIFY]);
  //$loop = TRUE;
  $result = array();
   $data = array('tag' => 'pathauto_patterns');
   $all_settings = module_invoke_all('pathauto', 'settings');
  foreach ($all_settings as $settings) {
    $module = $settings->module;
    $patterndescr = $settings->patterndescr;
    $patterndefault = $settings->patterndefault;
    $groupheader = $settings->groupheader;
    $variable =  'pathauto_'.$module . '_pattern';
    $data[$variable] = variable_get($variable, $patterndefault);

    // If the module supports a set of specialized patterns, set
    // them up here
    if (isset($settings->patternitems)) {
      foreach ($settings->patternitems as $itemname => $itemlabel) {
        $variable = 'pathauto_'.$module . '_' . $itemname . '_pattern';
        $data[$variable] = variable_get($variable, '');
      }
    }
  }
  $action = array(PATTERNS_MODIFY => $data);
  array_push($result, $action);
  return $result;
}


// Prepare data for processing
function pathauto_patterns_prepare($action, $tag, &$data = NULL) {

  $datacopy = $data;
  if ($tag == "pathauto_patterns" || $tag == "pathauto_steeing") {
    foreach ($datacopy as $key => $value) {
    /*  if (strpos($key, 'pathauto') !== FALSE)
        continue;*/
      if (is_array($value)) {
        if ($key != "update") {
          foreach ($value as $i => $v) {
            $new =  'pathauto_' . $i . '_' . $key;
            $data[$new] = $v;
          //  unset($data[$key][$i]);
          }
        //  unset($data[$key]);
        }
        elseif ($key == "update") {
          foreach ($value as $i => $v) {
            $new =  $i . '_pathauto_bulk_update_batch_process';
            if ($v == 1) {
              $data[$key][$new] = $new;
            }
            unset($data[$key][$i]);
          }
        }
      }

    }
  }
/*  elseif ($tag == "url_alias") {
    if ($action == PATTERNS_DELETE) {
      if (isset($data['pid'])) {
        $data['path']['pid'] = $data['pid'];
      }
      $data['values']['confirm'] = 1;
    }
    if ($action == PATTERNS_MODIFY) {
      if (!isset($data['pid'])) {
      	if(isset($data['language'])){
          	 $alias= _pathauto_existing_alias_data($data['source'],$data['language']);
          }else{
          	$alias= _pathauto_existing_alias_data($data['source']);
          }
        if($alias)
         $data['pid'] =$alias->pid;
      }
    }
  }*/
  
   // dirty fix for the null/0/false/'' problem
  if (isset($data['pathauto_verbose']) && $data['pathauto_verbose'] === 0)
    $data['pathauto_verbose'] = NULL;
  if (isset($data['pathauto_reduce_ascii']) && $data['pathauto_reduce_ascii'] === 0)
    $data['pathauto_reduce_ascii'] = NULL;
  return patterns_results();
}

// Validate the values for an action before running the pattern
function pathauto_patterns_validate($action, $tag, &$data = NULL) {
	$result = array();
  $status = PATTERNS_SUCCESS;
  $msg = '';
	/*
 * Syntactic validation
 * 
 */
 switch ($tag) {
    case 'pathauto_setting':
	switch ($action) {
        case PATTERNS_CREATE:
        	$msg .= t('pathauto_setting could not be created. Only modify action is possible for pathauto_setting.<br>');
            return patterns_results(PATTERNS_ERR, $msg);
          break;
        case PATTERNS_MODIFY:
         	//No mandatory fields in this case.
          //In this case we will need to define as well the attributes generated by the hook_prepare: uid and pass
          $interpretable_attributes = 
          	array('pathauto_verbose','pathauto_separator','pathauto_case','pathauto_max_length','pathauto_max_component_length','pathauto_update_action','pathauto_transliterate','pathauto_reduce_ascii','pathauto_ignore_words');
          module_load_include('inc', 'pathauto');
          $punctuation = pathauto_punctuation_chars();
  		  foreach ($punctuation as $name => $details) {
    		$interpretable_attributes[]='pathauto_punctuation_' . $name;
  		  }
          if(_patterns_has_uninterpretable_attributes($data, $interpretable_attributes, $msg)) {
            $status = PATTERNS_WARN;
          }
          break;
        case PATTERNS_DELETE:
          //Check mandatory fields, in this case is only one.
          $msg .= t('pathauto_setting could not be Deleted. Only modify action is possible for pathauto_setting.<br>');
            return patterns_results(PATTERNS_ERR, $msg);
          break;
      }
      break;
/*   case 'url_alias':
      switch ($action) {
        case PATTERNS_CREATE:
        	//Check mandatory fields, in this case is only one.
          $mandatory_attributes = array('source','alias');
          if (!_patterns_has_all_mandatory_attributes($data, $mandatory_attributes, $msg)) {
            return patterns_results(PATTERNS_ERR, $msg);
          }
          //In this case we will need to define as well the attributes generated by the hook_prepare: uid and pass
          $interpretable_attributes = array('source','alias','language');
          if(_patterns_has_uninterpretable_attributes($data, $interpretable_attributes, $msg)) {
            $status = PATTERNS_WARN;
          }
          break;
        case PATTERNS_MODIFY:
          //Check mandatory fields
          $mandatory_attributes = array('source');
          if (!_patterns_has_all_mandatory_attributes($data, $mandatory_attributes, $msg)) {
            return patterns_results(PATTERNS_ERR, $msg);
          }
          
          $interpretable_attributes = array('pid','source','alias','language');
          if(_patterns_has_uninterpretable_attributes($data, $interpretable_attributes, $msg)) {
            $status = PATTERNS_WARN;
          }
          break;
        case PATTERNS_DELETE:
           //Check mandatory fields
          $mandatory_attributes = array('pid');
          if (!_patterns_has_all_mandatory_attributes($data, $mandatory_attributes, $msg)) {
            return patterns_results(PATTERNS_ERR, $msg);
          }
          //In this case there are not optional attributes, so we can also use this set as interpretable attributes
          if(_patterns_has_uninterpretable_attributes($data, $mandatory_attributes, $msg)) {
            $status = PATTERNS_WARN;
          }
          break;
      }
      break;*/
   case 'pathauto_patterns':
      switch ($action) {
        case PATTERNS_CREATE:
        	$msg .= t('pathauto_patterns could not be created. Only modify action is possible for pathauto_patterns.<br>');
            return patterns_results(PATTERNS_ERR, $msg);
          break;
        case PATTERNS_MODIFY:
          //No mandatory fields in this case. 
          $interpretable_attributes=array();
          $all_settings = module_invoke_all('pathauto', 'settings');
  		  foreach ($all_settings as $settings) {
    		$module = $settings->module;
    		$variable =  'pathauto_'.$module . '_pattern';
    		$interpretable_attributes[]=$variable;
    		// If the module supports a set of specialized patterns, include them  here
    		if (isset($settings->patternitems)) {
      			foreach ($settings->patternitems as $itemname => $itemlabel) {
        			$variable = 'pathauto_'.$module . '_' . $itemname . '_pattern';
        			$interpretable_attributes[]=$variable;
      			}
    	  	}
  		  }     
          if(_patterns_has_uninterpretable_attributes($data, $interpretable_attributes, $msg)) {
            $status = PATTERNS_WARN;
          }
          break;
        case PATTERNS_DELETE:
          //Check mandatory fields, in this case is only one.
          $msg .= t('pathauto_patterns could not be Deleted. Only modify action is possible for pathauto_patterns.<br>');
            return patterns_results(PATTERNS_ERR, $msg);
          break;
      }
      break;
 }
 
      /*
 * Semantic validation
 *    
	switch ($tag) {
	case 'url_alias':
	  switch ($action) {
        case PATTERNS_CREATE:
            //Create semantic warning if the role name has already been defined
          if(isset($data['language'])){
          	 $exist= _pathauto_existing_alias_data($data['source'],$data['language']);
          }else{
          	$exist= _pathauto_existing_alias_data($data['source']);
          }	  
          if ($exist) {
            $result[] = array(PATTERNS_WARNING_ALREADY_DEFINED_ELEMENT => 
              t('The url alias for source %source already exists in the system.', array('%source' =>$data['source'])));
          }
          break;
        case PATTERNS_MODIFY:
          //Create semantic warning if the rid does not exist
          $exist=path_load(array('pid' => $data['pid']));
          if (!$exist) {
            $result[] = array(PATTERNS_WARNING_ELEMENT_UNDEFINED => 
              t('The url alias for source %source dose not exist in the system.', array('%source' =>$data['source'])));
          }
          break;
        case PATTERNS_DELETE:
        	$exist=path_load(array('pid' => $data['pid']));
          if (!$exist ) {
            $result[] = array(PATTERNS_WARNING_ELEMENT_UNDEFINED => 
             t('The url alias with id %pid dose not exist in the system.', array('%pid' =>$data['pid'])));
          }
          break;
      }
      break;
  }*/
  return patterns_results($status, $msg, $result);
/*  $status = PATTERNS_SUCCESS;
  $msg = '';
  if ($tag == 'url_alias' && ( $action == PATTERNS_MODIFY || $action == PATTERNS_DELETE)) {
    if (!isset($data['pid'])) {
      $status = PATTERNS_ERR;
      $msg = t('Attempt to %update a nonexistent url alias.', array('%update' => $action));
    }
    else {
      $exist = db_query("SELECT COUNT(*) FROM {url_alias} WHERE pid = :pid", array('pid' => $data['pid']))->fetchField(); // TODO: count()
      if (!$exist) {
        $status = PATTERNS_ERR;
        $msg = t('Attempt to %update a nonexistent url alias.', array('%update' => $action));
      }
    }
  }
  elseif ($tag == 'url_alias' && $action == PATTERNS_CREATE) {
    $lan = isset($data['language']) ? $data['language'] : 'und';
    $exist = db_query("SELECT COUNT(*) FROM {url_alias} WHERE alias = :name and language = :lan", array('name' => $data['alias'], 'lan' => $lan))->fetchField(); // TODO: count()
    if ($exist) {
      $status = PATTERNS_ERR;
      $msg = t('The alias %path is already in use in this language.', array('%path' => $data['alias']));
    }
  }
  return patterns_results($status, $msg);*/
}

// Prepare for valid processing of this type of component
function pathauto_patterns_build($action, $tag, &$data = NULL, $form_id) {
  if ($data['pathauto_reset']) {
    $result = db_query("SELECT name FROM {variable} WHERE name LIKE 'pathauto_%'");
    while ($var = db_fetch_array($result)) {
      variable_del($var['name']);
    }
    cache_clear_all('variables', 'cache');
    return;
  }

  module_load_include('inc', 'pathauto', 'pathauto.admin');
  module_load_include('inc', 'path', 'path.admin');
  return patterns_results(PATTERNS_SUCCESS,   t('Execution successful'), $data);
}




/*
 *
 */
function pathauto_patterns_get_arguments($action, $tag, $form_id, &$loop = FALSE) {
  $return = array();
  if ($form_id == "path_admin_form" && $action == PATTERNS_MODIFY) {
    $loop = TRUE;
    $paths = db_query("SELECT * FROM {url_alias}", array(), array('fetch' => PDO::FETCH_ASSOC))->fetchAll();
    foreach ($paths as $path) {
      array_push($return, array($path));
    }
    return $return;
  }
  elseif ($form_id == "path_admin_form") {
    $result = array(array(
      'source' => '',
      'alias' => '',
      'language' => 'und',
      'pid' => NULL,
      )
    );
  return $result;
  }
}

// Build a patterns actions and parameters
function pathauto_patterns_params($action, $form_id, &$data = NULL, &$a) {
  if ($form_id == "path_admin_form" && $action == PATTERNS_MODIFY) {
    $pid = $data['pid'];
    $path = db_query("SELECT * FROM {url_alias} WHERE pid = :pid", array('pid' => $data['pid']))->fetchField();
    $result = array($path);
  }
  elseif ($form_id == "path_admin_form") {
    $result = array(array(
        'source' => '',
        'alias' => '',
        'language' => 'und',
        'pid' => NULL,
      ));
  }
  elseif ($form_id == "path_admin_delete_confirm") {
    $result = array($data['path']);
  }
  return patterns_results(PATTERNS_SUCCESS, t('Execution successful'), $result);
}

function pathauto_patterns_cleanup($action, $tag, &$data = NULL) {
  unset($_POST['op']);
  return patterns_results();
}

// Return which callback functions to actually use.
function pathauto_patterns_callbacks($action, $tag, &$data = NULL) {
  $desc = pathauto_patterns();
  $result = $desc[$tag][$action];
  return patterns_results(PATTERNS_SUCCESS, t('Execution successful'), $result);
}



/*
function pathauto_patterns($op, $id = NULL, &$data = NULL) {
 switch ($op) {
 // Return the valid tags that this component can prepare and process
 case 'tags':
 return array('pathauto');
 break;

 // Return a list of forms/actions this component can handle
 case 'actions':
 return array(
 'pathauto_admin_settings' => t('Pathauto: Configure pathauto settings'),
 );
 break;

 // Return a summary of an action
 case 'summary':
 return t('Setup pathauto urls');
 break;

 // Prepare data for processing
 case 'prepare':
 foreach ($data as $key => $value) {
 if (is_array($value)) {
 $mappings = array('default' => 'pattern');
 $skip = array('bulkupdate', 'applytofeeds');
 foreach ($value as $i => $v) {
 if (in_array($i, $skip)) {
 $new = $key . '_' . $i;
 }
 elseif (array_key_exists($i, $mappings)) {
 $new = $key . '_' . $mappings[$i];
 }
 // remove prefix "vid_" required by XML because
 // element names can't begin with a number
 elseif ($key == 'taxonomy' && strpos($i, 'vid_') === 0) {
 $new = $key . '_' . str_replace('vid_', '', $i) . '_pattern';
 }
 else {
 $new = $key . '_' . $i . '_pattern';
 }

 $data[$new] = $v;
 unset($data[$key][$i]);
 }
 unset($data[$key]);
 }
 }

 $mappings = array('update' => 'update_action');
 foreach ($data as $key => $value) {
 if (array_key_exists($key, $mappings)) {
 $new = $mappings[$key];
 }
 else {
 $new = 'pathauto_' . $key;
 }

 if (strpos($key, 'pathauto_') === FALSE && $key != 'reset') {
 $data[$new] = $value;
 unset($data[$key]);
 }
 }
 break;

 // Pre validate actions
 case 'pre-validate':
 break;

 // Return the form_id('s) for each action
 case 'form_id':
 module_load_include('inc', 'pathauto', 'pathauto.admin');
 return 'pathauto_admin_settings';
 break;

 // Prepare for valid processing of this type of component
 case 'build':
 if ($data['reset']) {
 $result = db_query("SELECT name FROM {variable} WHERE name LIKE 'pathauto_%'");
 while ($var = db_fetch_array($result)) {
 variable_del($var['name']);
 }
 cache_clear_all('variables', 'cache');
 return;
 }

 return $data;
 break;

 // Validate the values for an action before running the pattern
 case 'validate':
 break;

 // Build a patterns actions and parameters
 case 'params':
 break;

 // Cleanup any global settings after the action runs
 case 'cleanup':
 unset($_POST['op']);
 break;
 }
 }*/

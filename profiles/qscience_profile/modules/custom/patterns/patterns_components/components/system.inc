<?php
/*
 * TODO: Check if it is possible to simplify the prepare hook. 
 * TODO: Check variables delete validation case
 * TODO: Add more descriptive messages after merging with the new core branch (custom function cases)
 * @file
 * Patterns component for system related operations.
 */

/**
 * 
 * Implements hook_patterns().
 * 
 * @param string $data
 * 
 */
function system_patterns($data) {

  $files = array('modules/system/system.admin.inc');

  //Function not run through form.
  $actions['module'] = array('descr' => t('Enable/Disable Modules'),
    PATTERNS_CREATE => array('modules_enable'),
    PATTERNS_DELETE => array('modules_disable'),
    PATTERNS_EXPORT => array(PATTERNS_EXPORT_ALL => 'system_patterns_export_all_modules', ),   
    PATTERNS_FILES  => $files,
  );

  //Function not run through form.
  $actions['theme'] = array('descr' => t('Enable (and set default)/Disable Themes'),
    PATTERNS_MODIFY => array('set_theme_properties'),
    PATTERNS_EXPORT => array(PATTERNS_EXPORT_ALL => 'system_patterns_export_all_themes', ),
    PATTERNS_FILES  => $files,
  );

  // Determine necessary forms for action 'form' based on the data.
  $forms = array();
  if ($data) {
    if ($data['tag'] == 'form' && isset($data['form_id'])) {
      $forms[] = $data['form_id'];
    }
  }

  $actions['form'] = array('descr' => t('Submit Custom Forms'),
    PATTERNS_CREATE => $forms,
    PATTERNS_FILES  => $files,
  );

  //Function not run through form.
  $actions['call_php_func'] = array('descr' => t('Call PHP Functions'),
    PATTERNS_CREATE => array('call_php_func_form'),
  );

  //Function not run through form.
  $actions['variables'] = array('descr'  => t('Set/Modify/Delete System Variables'),
    PATTERNS_MODIFY => array('variables_execute'),
    PATTERNS_EXPORT => array(PATTERNS_EXPORT_ALL => 'system_patterns_export_all_variables', ),
  );
  return $actions;

}


/**
 * 
 * Returns a set of PATTERNS_CREATE actions with the whole set of modules
 * currently enabled in the system.
 *
 * @param string $args
 * @param string $result
 * @return array $actions
 */
function system_patterns_export_all_modules($args = NULL, &$result = NULL) {
  $actions = array(PATTERNS_CREATE => array('tag' => 'module'));
  foreach (module_list() as $module => $value) {
    $action = array('value' => $module);
    array_push($actions[PATTERNS_CREATE], $action);
  }
  $result = array($actions);
  return $result;
}

/**
 * 
 * Returns a set of PATTERNS_MODIFY actions with the whole set of themes'
 * settings currently stored in the system.
 *
 * @param string $args
 * @param string $result
 * @return array $actions
 */
function system_patterns_export_all_themes($args = NULL, &$result = NULL) {
  //Retrieve the current list of themes and the default and admin values
  $themes = list_themes();
  $default_theme = variable_get('theme_default');
  $admin_theme = variable_get('admin_theme');
  $result = array();
  
  foreach ($themes as $theme) {
    //Prepare date for modify operation
    $data = array('tag' => 'theme');
    $data['value'] = $theme->name;
    $data['status'] = $theme->status;
    $data['default'] = ($theme->name == $default_theme);
    $data['admin'] = ($theme->name == $admin_theme);
    $action = array(PATTERNS_MODIFY => $data);
    array_push($result, $action);
  }
  return $result;
}

/**
 *
 * Returns a set of PATTERNS_MODIFY actions with 
 * the whole set of variables currently stored in the system.
 *
 * @param string $args
 * @param string $result
 * @return array $actions
 */
function system_patterns_export_all_variables($args = NULL, &$result = NULL) {
  //Get all the currently stored variables
  $query = db_select('variable', 'v')->extend('TableSort');
  $query->fields('v', array('name', 'value'));
  $qresult = $query->execute();
  
  //Got through all the variables and prepare set of CREATE or MODIFY actions
  $actions = array(PATTERNS_MODIFY => array('tag' => 'variables'));
  foreach ($qresult as $row) {
    $name = $row->name;
    $value = variable_get($name);
    //Apply htmlspecialchars() function recursively to keep the same value input by the user
    _htmlspecialchars_decode($value);
    $action = array('name' => $name, 'value' => $value);
    array_push($actions[PATTERNS_MODIFY], $action);
  }
    
  $result = array($actions);
  return $result;
}


/**
 * 
 * Prepare data for processing
 * 
 * @param string $action
 * @param string $tag
 * @param array $data
 * 
 */
function system_patterns_prepare($action, $tag, &$data) {
  if ($tag == 'module') {
    // Make a <modules>modulename</modules> style tag work
    if (is_string($data) || (isset($data['value']) && !isset($data[0]))) {
      $data = array($data);
    }
    // Ensure proper data format for cases when <modules> tag contains
    // only single <module> tag.
    if (!empty($data['module']) && is_string($data['module'])) {
      $data[0]['value'] = $data['module'];
      unset($data['module']);
    }
    foreach ($data as &$item) {
      // Ensure that modules with tags like <module>modulename</module>
      // are represented as an array instead of a string
      if (is_string($item)) {
        $item = array('value' => $item);
      }
    }
    // proccess alias for delete
    if (isset($data['disable']) && !isset($data['delete'])) {
      $data['delete'] = $data['disable'];
    }
    unset($data['disable']);
  }
  elseif ($tag == 'form') {
  }
  else {
    switch ($tag) {
      case 'variable':
        // Turn <variable>value</variable> style tag function the same as <variables>
        $tag = 'variables';
        $data = array($data);
      case 'variables':
        // Make a <variables>modulename</variables> style tag work
        if (!$data[0]) {
          if ($data['variable'] && isset($data['variable'][0])) {
            $data = $data['variable'];
            unset($data['variable']);
          }
          elseif ($data['variable'] && isset($data['variable']['value'])) {
            $data[0] = $data['variable'];
            unset($data['variable']);
          }
          else {
            $temp = $data;
            $data[0] = $temp;
            unset($data['id'], $data['value'], $data['delete']);
          }
        }
        for ($i = 0, $total = count($data); $i < $total; $i++) {
          $item = &$data[$i];
          if (!isset($item['value']) && (!isset($item['delete']) || !$item['delete'])) {
            $value = $item;
            unset($value['name']);
            $item = array('name' => $item['name'], 'value' => $value);
          }
        }
        break;
    }
  }
  return patterns_results();
}


/**
 *
 * @param string $action Type of action being executed
 * @param string $tag Type of tag to be validated
 * @param array $data Associative array containing the data action processed from the pattern
 *
 * @return mixed through patterns_results($status, $msg, $result) function. Status of the operation,
 * error messages and semantic warnings through $result
 */
function system_patterns_validate($action, $tag, &$data) {
  $result = array();
  $status = PATTERNS_SUCCESS;
  $msg = '';

  /*
   * Syntactic validation:
  * - Syntactic warnings: They refer to wrong grammar statements that will not
  * provoke any execution error. Ex.: non-interpretable attributes.
  * - Syntactic errors: They refer to wrong grammar statements that will provoke
  * an execution error. Ex.: misspelling in required attribute.
  *
  */
  switch ($tag) {
    case 'module':
        switch ($action) {
        case PATTERNS_CREATE:
        case PATTERNS_DELETE:
          //In this case even the delete case is the same because the 'value' attribute is added by
          //the hook_prepare(). We will keep the code (switch with same cases) like this for future extendibility reasons
          $mandatory_attributes = array('value');
          //Go through all the variables to check if they have the mandatory attributes
          foreach ($data as $module) {
            if (!_patterns_has_all_mandatory_attributes($module, $mandatory_attributes, $msg)) {
              return patterns_results(PATTERNS_ERR, $msg);
            }
          }
          //In this case there are not optional attributes, so we can also use this set as interpretable attributes. Go through all variables
          foreach ($data as $module) {
            if(_patterns_has_uninterpretable_attributes($module, $mandatory_attributes, $msg)) {
              $status = PATTERNS_WARN;
            }
          }
          break;
        default:
          $msg = t('Action %action is uncompatible for tag %tag.', array('%action' =>$action, '%tag' =>$tag,));
          return patterns_results(PATTERNS_ERR, $msg);
      }
      break;
    case 'theme':
      //The only actions syntactically valid with theme is MODIFY
      if($action == PATTERNS_MODIFY) {
        //The only mandatory attribute in this case is the value
        $mandatory_attributes = array('value');
        if (!_patterns_has_all_mandatory_attributes($data, $mandatory_attributes, $msg)) {
          return patterns_results(PATTERNS_ERR, $msg);
        }
        //In this case we will need to define as well the attributes generated by the hook_prepare(): uid and pass
        $interpretable_attributes = array('value', 'status', 'default', 'admin');
        if(_patterns_has_uninterpretable_attributes($data, $interpretable_attributes, $msg)) {
          $status = PATTERNS_WARN;
        }
      }else{
        //Rest of  actions for the permission case are considered as syntactical errors
        $msg = t('Action %action is uncompatible for tag %tag.', array('%action' =>$action, '%tag' =>$tag,));
        return patterns_results(PATTERNS_ERR, $msg);
      }
      break;
    case 'form':
      //Original validation ported to syntactic case as error
      if (!isset($data['form_id'])) {
       $msg = t('"form_id" is missing.');
       return patterns_results(PATTERNS_ERR, $msg);
      }
      // Attempt to load required include file from menu.
      list($menu, $masks) = menu_router_build();
      foreach ($menu as $item) {
       if (isset($item['page arguments'][0]) && $item['page arguments'][0] == $data['form_id']
           && !empty($item['include file'])) {
         $data['include'] = $item['include file'];
         break;
       }
      }
      
      if (!empty($data['include']) && is_file($data['include'])) {
       require_once $data['include'];
      }
      elseif (is_array($data['include'])) {
       $data['include'] = drupal_get_path('module', $data['include']['module']) . '/' . $data['include']['file'];
      }
      
      if (!function_exists($data['form_id'])) {
       if (!empty($data['module']) && is_string($data['module'])) {
         $modules = module_list();
         if (in_array($data['module'], $modules)) {
           // try most common include file names
           module_load_include('inc', $data['module']);
           module_load_include('inc', $data['module'], $data['module'] . '.admin');
           module_load_include('inc', $data['module'], $data['module'] . '.page');
         }
       }
      }
      if (!function_exists($data['form_id'])) {
       $msg = t("Couldn't load the form %form. Check if all required modules are enabled and try to define 'include' or 'module' for this action.", array('%form' => $data['form_id']));
       return patterns_results(PATTERNS_ERR, $msg);
      }
      break;
    case 'call_php_func':
      //Original validation ported to syntactic case as error
      if (empty($data['function'])) {
       $msg = t("A function is required for this tag");
       return patterns_results(PATTERNS_ERR, $msg);
      }elseif (!empty($data['type']) && empty($data['module'])) {
       $msg = t("If you specify a type you must specify a module. See the documentation for module_load_include.");
       return patterns_results(PATTERNS_ERR, $msg);
      }elseif (empty($data['type']) && !empty($data['module'])) {
       $msg = t("If you specify a module you must specify a type. See the documentation for module_load_include.");
       return patterns_results(PATTERNS_ERR, $msg);
      }elseif (!empty($data['filepath']) && !file_exists($data['filepath'])) {
       $msg = t('The file that you specified does not exist: %file', array('%file' => $data['filepath']));
       return patterns_results(PATTERNS_ERR, $msg);
      }
      break;
    case 'variables':
      //The only actions syntactically valid with variables is MODIFY
      if($action == PATTERNS_MODIFY) {
        //Prepare mandatory attributes
        $mandatory_attributes = array('name', 'value');
        //Go through all the variables to check if they have the mandatory attributes
        foreach ($data as $variable) {
          if (!_patterns_has_all_mandatory_attributes($variable, $mandatory_attributes, $msg)) {
            return patterns_results(PATTERNS_ERR, $msg);
          }
        }
        //In this case there are not optional attributes, so we can also use this set as interpretable attributes. Go through all variables
        foreach ($data as $variable) {
          if(_patterns_has_uninterpretable_attributes($variable, $mandatory_attributes, $msg)) {
            $status = PATTERNS_WARN;
          }
        }
      }else{
        //Rest of  actions for the permission case are considered as syntactical errors
        $msg = t('Action %action is uncompatible for tag %tag.', array('%action' =>$action, '%tag' =>$tag,));
        return patterns_results(PATTERNS_ERR, $msg);
      }
      break;
  }

  /*
   * Semantic validation:
  * - Semantic warnings: They refer to the meaning of the pattern itself, and they
  * might provoke execution errors if they are not solved.
  *
  */
  switch ($tag) {
    case 'module':
        $enabled_modules = module_list();
        switch ($action) {
        case PATTERNS_CREATE:
          //Check the modules are not already enabled
          foreach ($data as $module) {
            if (in_array($module['value'], $enabled_modules)) {
              $result[] = array(PATTERNS_WARNING_ALREADY_DEFINED_ELEMENT =>
                t('The module %mod is already enabled.', array('%mod' =>$module['value'],)));
            }  
          }
          break;
        case PATTERNS_DELETE:
          //Check the modules are not already disabled
          foreach ($data as $module) {
            if (!in_array($module['value'], $enabled_modules)) {
              $result[] = array(PATTERNS_WARNING_ALREADY_DEFINED_ELEMENT =>
                t('The module %mod is already disabled.', array('%mod' =>$module['value'],)));
            }  
          }
          break;
      }
      break;
    case 'theme':
      //We raised a semantic error if the theme does not exist. Since only MODIFY actions are supported, it is not necessary to switch
      if (!array_key_exists($data['value'], list_themes())) {
        $result[] = array(PATTERNS_WARNING_ELEMENT_UNDEFINED =>
            t('The theme %theme is not currently installed in the system.', array('%theme' =>$data['value'],)));
      }
      break;
    case 'variables':
      //We raised a semantic error for each variable that does not exist. Since only MODIFY actions are supported, it is not necessary to switch
      foreach ($data as $variable) {
        if (!variable_get($variable['name'])) {
          $result[] = array(PATTERNS_WARNING_ELEMENT_UNDEFINED =>
              t('The variable %var does not exist in the system.', array('%var' =>$variable['name'],)));
        }
      }
      break;
  }
  return patterns_results($status, $msg, $result);
}


/**
 * 
 * Prepare for valid processing of this type of component
 * 
 * @param string $action
 * @param string $form_id
 * @param string $data
 * @param array $a
 * 
 */
function system_patterns_build($action, $form_id, &$data = NULL, &$a) {
  $status = PATTERNS_SUCCESS;
  $msg = '';
  $result = NULL;

  if ($form_id == 'call_php_func_form') {
    $data['type'] = empty($data['type']) ? '' : $data['type'];
    $data['module'] = empty($data['module']) ? '' : $data['module'];
    $data['name'] = empty($data['name']) ? '' : $data['name'];
    $data['filepath'] = empty($data['filepath']) ? '' : $data['filepath'];

    if (empty($data['arguments'])) {
      $data['arguments'] = array();
    }
    elseif (!is_array($data['arguments'])) {
      $data['arguments'] = array($data['arguments']);
    }
    _call_php_func_include_files($data); // TODO: this works here?
    if (!is_callable($data['function'])) {
      $status = PATTERNS_ERR;
      $msg = t('The given function %func is not callable', array('%func' => $data['function']));
    }
    // $result = $data;
  } // TODO: other forms?
  else { // Running a user-provided form through tag 'form'.
    $form_state = $data['form_state_extras'];
    unset($data['form_state_extras']);
    unset($data['include']);
    $form_state['values'] = $data;
    $form_state['submitted'] = FALSE;
    if (!isset($form_state['storage'])) {
      $form_state['storage'] = NULL;
    }
    $result = $form_state;
  }
  return patterns_results($status, $msg, $result);
}

/**
 * 
 * Build a patterns actions and parameters
 * 
 * @param string $action
 * @param string $form_id
 * @param string $data
 * @param array $a
 * 
 */
function system_patterns_params($action, $form_id, &$data = NULL, &$a) {
  $result = NULL;

  if ($form_id == 'call_php_func_form') {
    $result = array($data['function'], $data['type'], $data['module'], $data['name'], $data['filepath'], $data['arguments']);
  }else { // Running a user-provided form through tag 'form'.
    if (isset($data['args'])) {
      $params = $data['args'];
      unset($data['args']);
      $result = params;
    }
  }
  return patterns_results(PATTERNS_SUCCESS, t('Execution successful'), $result);
}

/**
 * 
 * Cleanup any global settings after the action runs
 * 
 * @param string $action
 * @param string $tag
 * @param array $data
 * 
 */
function system_patterns_cleanup($action, $tag, &$data) {
  if ($tag == 'module') {
    menu_rebuild(); // TODO: only do this if there has been a module enabled/disabled
  }
  return patterns_results();
}


/**
 * Wraps the call to drupal_form_submit to enable modules. It enables automatically all the dependencies
 *
 * @param string $form_id  String containing the form ID. In the case of custom functions the value is empty.
 * @param array $form_state  Set of values after parsing the action.
 */
function modules_enable($form_id, &$form_state) {
  $modules_enabled = array();
  foreach ($form_state['values'] as $module) {
    module_enable(array($module['value']), TRUE);
    $modules_enabled[] = $module['value'];
  }
  
  $msg = ((count($modules_enabled) > 0) ? t('Module(s) %modules enabled.', 
      array('%modules' => implode(', ', $modules_enabled))) : t('No modules have been enabled.'));
  return patterns_results(PATTERN_SUCCESS, $msg);
}

/**
 * Wraps the call to drupal_form_submit to disable modules. It disables automatically all the dependencies.
 *
 * @param string $form_id  String containing the form ID. In the case of custom functions the value is empty.
 * @param array $form_state  Set of values after parsing the action.
 */
function modules_disable($form_id, &$form_state) {
  $modules_disabled = array();
  foreach ($form_state['values'] as $module) {
    module_disable(array($module['value']), TRUE);
    $modules_disabled[] = $module['value'];
  }
  
  $msg = ((count($modules_disabled) > 0) ? t('Module(s) %modules disabled.', 
      array('%modules' => implode(', ', $modules_disabled))) : t('No modules have been disabled.'));
  return patterns_results(PATTERN_SUCCESS, $msg);
}

/**
 * Wraps the call to drupal_form_submit for setting the values of the variables.
 * Goes through all the expected values from the form and prepare them for the query.
 *
 * @param string $form_id  String containing the form ID. In the case of custom functions the value is empty.
 * @param array $form_state  Set of values after parsing the action.
 */
function variables_execute($form_id, &$form_state) {
  $names = array();
  foreach ($form_state['values'] as $variable) {
    $var = variable_get($variable['name'], NULL);
    if (is_array($var)) {
      //Make sure we don't lose parts of the array that were not defined by pattern's action
      $var = array_merge($var, $variable['value']);
      variable_set($variable['name'], $var);
    }
    else {
      variable_set($variable['name'], $variable['value']);
    }
    $names[] = $variable['name'];
  }
  
  $msg = t('Variable(s) %vars updated.', array('%vars' => implode(', ', $names)));
  return patterns_results(PATTERNS_SUCCESS, $msg);
}

/**
 * Wraps the call to drupal_form_submit to set the theme properties.
 *
 * @param string $form_id  String containing the form ID. In the case of custom functions the value is empty.
 * @param array $form_state  Set of values after parsing the action.
 */
function set_theme_properties($form_id, &$form_state) {
  //At this point we can ensure there is a theme value, since is part of the mandatory syntactic analysis
  $theme = $form_state['values']['value'];
  
  //Enable or disable the if there is any valid value in status
  if (isset($form_state['values']['status']) && $form_state['values']['status'] == TRUE) {
    theme_enable(array($theme));
  } else if(isset($form_state['values']['status']) && $form_state['values']['status'] == FALSE) {
    theme_disable(array($theme));
  }
  //Set this theme as default if default is set and is true
  if (isset($form_state['values']['default']) && $form_state['values']['default']) {
    variable_set('theme_default', $theme);
  }
  //Set this theme as administrative if admin is set and is true
  if (isset($form_state['values']['admin']) && $form_state['values']['admin']) {
    variable_set('admin_theme', $theme);
  }
  
  $msg = t('Properties(s) of theme %theme updated.', array('%theme' => $theme));
  return patterns_results(PATTERNS_SUCCESS, $msg);
}

/**
 * TODO: Document function
 * 
 * @param unknown $form
 * @param unknown $form_state
 * @param unknown $func
 * @param unknown $type
 * @param unknown $module
 * @param unknown $name
 * @param unknown $filepath
 * @param unknown $args
 * @return multitype:multitype:unknown
 */
function call_php_func_form($form, $form_state, $func, $type, $module, $name, $filepath, $args) {
  return array(
    '#call_php_func' => array(
      'function' => $func,
      'type' => $type,
      'module' => $module,
      'name' => $name,
      'filepath' => $filepath,
      'arguments' => $args,
      ),
    );
}

/**
 * TODO: Document function
 * 
 * @param unknown $form
 * @param unknown $form_state
 */
function call_php_func_form_submit($form, &$form_state) {
  $values = $form['#call_php_func'];
  _call_php_func_include_files($values);

  call_user_func_array($values['function'], $values['arguments']);
}

/**
 * TODO: Document function
 * 
 * @param unknown $values
 */
function _call_php_func_include_files($values) {
  if ($values['type'] && $values['module']) {
    module_load_include($values['type'], $values['module'], $values['name']);
  }
  elseif ($values['filepath']) {
    require_once($values['filepath']);
  }
}

/**
 * 
 * Convert special HTML entities back to characters recursively.
 * 
 * @param string &$value
 */
function _htmlspecialchars_decode(&$value) {
  if (!is_array($value)) {
    $value = htmlspecialchars_decode($value);
  }else{
    foreach ($value as &$v) {
      _htmlspecialchars_decode($v);
    }
  }
}

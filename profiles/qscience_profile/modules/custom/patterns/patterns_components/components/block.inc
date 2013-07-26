<?php
/*
 * @file
 * Patterns component for blocks.
 * 
 * The actions are executed and exported in a different way depending 
 * on the type of block (custom or not).
 * Please visit for further details about the syntax and semantic of 
 * this component: @TO-DO Add link from drupal.org
 * 
 */
function block_patterns($data) {

  $files = array('modules/block/block.admin.inc', 'modules/block/block.api.php');
  
  //All block functions are applied through custom functions
  
  $actions['block'] = array(
    PATTERNS_INFO => t('Create/Modify/Delete Blocks'),
    PATTERNS_CREATE => array('block_create'),
    PATTERNS_MODIFY => array('block_modify'),
    PATTERNS_DELETE => array('block_delete'),
    PATTERNS_FILES  => $files,
    PATTERNS_EXPORT => array(PATTERNS_EXPORT_CURRENT_THEMES => 'block_patterns_export_all_blocks', ),
  );
  
  $actions['block_node_type'] = array(
      PATTERNS_INFO => t('Create and delete relationships between Blocks and Content Types'),
      PATTERNS_CREATE => array('block_node_type_create'),
      PATTERNS_DELETE => array('block_node_type_delete'),
      PATTERNS_FILES  => $files,
      PATTERNS_EXPORT => array(PATTERNS_EXPORT_ALL => 'block_patterns_export_all_block_node_types', ),
  );
  
  $actions['block_role'] = array(
      PATTERNS_INFO => t('Create and delete relationships between Blocks and Roles'),
      PATTERNS_CREATE => array('block_role_create'),
      PATTERNS_DELETE => array('block_role_delete'),
      PATTERNS_FILES  => $files,
      PATTERNS_EXPORT => array(PATTERNS_EXPORT_ALL => 'block_patterns_export_all_block_roles', ),
  );

  return $actions;

}

/**
 *
 * Returns a set of PATTERNS_CREATE or PATTERNS_MODIFY actions
 * with the whole set of blocks currently stored in the system.
 * In the case of custom blocks some extra fields from table 
 * block_custom will be exported.
 * Different actions are employed to gather the blocks depending on the
 * type of action to return.
 *
 * @param array $args  Set of arguments received from the form.
 * @param string $result  Current set of actions for the whole process.
 *
 * @return array $result  Set of actions after performing the changes.
 */
function block_patterns_export_all_blocks($args = NULL, &$result = NULL) {
  $result = array();
  //Got through all the blocks and prepare set of actions according to the type of export process.
  //Depending on the action we will use a different method to gather the blocks
  switch ($args['type']) {
    case PATTERNS_CREATE:
      $blocks = _block_rehash();
      //Custom blocks are exported without delta, but including the fields on block_custom
      foreach ($blocks as $block) {
        $data = array('tag' => 'block');
        foreach ($block as $key => $value) {          
          //We discard bid and info for all the cases
          if ($key != 'bid' && $key != 'info') {
            $data[$key] = $value;
          }
          //In case the block is a custom one (created by block) we will need to
          //add some extra fields from block_custom table while processing the 'module' field
          if ($key == 'module' && $value == 'block') {
            //This query must return a maximum of 1 record
            $block_custom_metadata = db_select('block_custom', 'bc')
            ->fields('bc', array('body', 'info', 'format'))
            ->condition('bid', $block['delta'])
            ->range(0,1)
            ->execute();
            $metadata = $block_custom_metadata->fetch();
            //We check the existance to avoid warnings with records inserted manually
            if ($metadata) {
              $data['body'] = $metadata->body;
              $data['info'] = $metadata->info;
              $data['format'] = $metadata->format;
            }
          }
        }
        //Unset the delta field in data in case we have just processed a custom block
        if ($data['module'] == 'block') {
          unset($data['delta']);
        }
        $action = array(PATTERNS_CREATE => $data);
        array_push($result, $action);
      }
      break;
    case PATTERNS_MODIFY :
      $blocks = db_query('SELECT * from block');
      foreach ($blocks as $block) {
        $data = array('tag' => 'block');
        foreach ($block as $key => $value) {
          //We discard the bid for all the cases
          if ($key != 'bid') {
            $data[$key] = $value;
          }
        }
        $action = array(PATTERNS_MODIFY => $data);
        array_push($result, $action);
      }
      break;
  }
  
  return $result;
}


/**
 *
 * Returns a set of PATTERNS_CREATE actions with the relationship between blocks
 * and node types currently stored in the system.
 *
 * @param array $args  Set of arguments received from the form.
 * @param string $result  Current set of actions for the whole process.
 *
 * @return array $result  Set of actions after performing the changes.
 */
function block_patterns_export_all_block_node_types($args = NULL, &$result = NULL) {
  $result = array();
  $block_node_types = db_query('SELECT * from block_node_type');

  //In this case the export is only as a set of modifys
  foreach ($block_node_types as $node_type) {
    $data = array('tag' => 'block_node_type');
    foreach ($node_type as $key => $value) {
      $data[$key] = $value;
    }
    $action = array(PATTERNS_CREATE => $data);
    array_push($result, $action);
  }
  return $result;
}


/**
 *
 * Returns a set of PATTERNS_CREATE actions with the relationship between blocks
 * and roles currently stored in the system.
 *
 * @param array $args  Set of arguments received from the form.
 * @param string $result  Current set of actions for the whole process.
 *
 * @return array $result  Set of actions after performing the changes.
 */
function block_patterns_export_all_block_roles($args = NULL, &$result = NULL) {
  $result = array();
  $block_node_roles = db_query('SELECT * from block_role');

  //In this case the export is only as a set of modifys
  foreach ($block_node_roles as $role) {
    $data = array('tag' => 'block_role');
    foreach ($role as $key => $value) {
      //Transform the rid into the role name
      if ($key == 'rid') {
        $role = user_role_load($value);
        $data['role'] = $role->name;
      }else{
        $data[$key] = $value;
      }
    }
    $action = array(PATTERNS_CREATE => $data);
    array_push($result, $action);
  }
  return $result;
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
function block_patterns_validate($action, $tag, &$data) {
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
    case 'block':
      switch ($action) {
        case PATTERNS_CREATE:
          //Assign mandatory fields depending on the type of block (custom required some extra fields but does not use delta)
          //$mandatory_attributes = array('module', 'delta', 'title');
          $mandatory_attributes = ($data['module'] == 'block') ? array('module', 'title', 'body', 'info', 'format'): array('module', 'delta', 'title');
          if (!_patterns_has_all_mandatory_attributes($data, $mandatory_attributes, $msg)) {
            return patterns_results(PATTERNS_ERR, $msg);
          }
          //Define interpretable attributes depending on the type of block (custom required some extra fields but does not use delta)
          $interpretable_attributes = ($data['module'] == 'block') ? array('module', 'title', 'theme', 'status', 'weight', 'region',
            'custom', 'visibility', 'pages', 'cache', 'body', 'info', 'format'): array('module', 'delta', 'title', 'theme', 'status', 'weight', 'region',
            'custom', 'visibility', 'pages', 'cache');
          if(_patterns_has_uninterpretable_attributes($data, $interpretable_attributes, $msg)) {
            $status = PATTERNS_WARN;
          }
          break;
        case PATTERNS_MODIFY:
          //Assign mandatory fields depending, custom_block fields are not covered by modify actions
          $mandatory_attributes = array('module', 'delta', 'title');
          if (!_patterns_has_all_mandatory_attributes($data, $mandatory_attributes, $msg)) {
            return patterns_results(PATTERNS_ERR, $msg);
          }
          //Define interpretable attributes, custom_block fields are not covered by modify actions
          $interpretable_attributes = array('module', 'delta', 'title', 'theme', 'status', 'weight', 'region',
              'custom', 'visibility', 'pages', 'cache');
          if(_patterns_has_uninterpretable_attributes($data, $interpretable_attributes, $msg)) {
            $status = PATTERNS_WARN;
          }
          break;
        case PATTERNS_DELETE:
          //Check mandatory fields
          $mandatory_attributes = array('module', 'delta');
          if (!_patterns_has_all_mandatory_attributes($data, $mandatory_attributes, $msg)) {
            return patterns_results(PATTERNS_ERR, $msg);
          }
          //In this case mandatory and interpretable are the same
          if(_patterns_has_uninterpretable_attributes($data, $mandatory_attributes, $msg)) {
            $status = PATTERNS_WARN;
          }
          break;
      }
      break;
    case 'block_node_type':
      switch ($action) {
        case PATTERNS_CREATE:
        case PATTERNS_DELETE:
          //Check mandatory fields
          $mandatory_attributes = array('module', 'delta', 'type');
          if (!_patterns_has_all_mandatory_attributes($data, $mandatory_attributes, $msg)) {
            return patterns_results(PATTERNS_ERR, $msg);
          }
          //In this case mandatory and interpretable are the same
          if(_patterns_has_uninterpretable_attributes($data, $mandatory_attributes, $msg)) {
            $status = PATTERNS_WARN;
          }
          break;
        default:
          $msg = t('Action %action is uncompatible for tag %tag.', array('%action' =>$action, '%tag' =>$tag,));
          return patterns_results(PATTERNS_ERR, $msg);
      }
      break;
    case 'block_role':
        switch ($action) {
        case PATTERNS_CREATE:
        case PATTERNS_DELETE:
          //Check mandatory fields
          $mandatory_attributes = array('module', 'delta', 'role');
          if (!_patterns_has_all_mandatory_attributes($data, $mandatory_attributes, $msg)) {
            return patterns_results(PATTERNS_ERR, $msg);
          }
          //In this case mandatory and interpretable are the same
          if(_patterns_has_uninterpretable_attributes($data, $mandatory_attributes, $msg)) {
            $status = PATTERNS_WARN;
          }
          break;
        default:
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
    case 'block':
      switch ($action) {
        case PATTERNS_CREATE:
          if ($data['module']== 'block') {
            //Check info field is unique in the case of custom blocks
            if (_block_patterns_info_exists($data['info'])) {
              $result[] = array(PATTERNS_WARNING_NOT_UNIQUE_ALIAS =>
                  t('A block defined by module %module with info field named %info already exists in the system.',
                      array('%module' =>$data['module'], '%info' =>$data['info'])));
            }
          }else{
            //Create semantic warning if the combination of module+delta already exists (only for not custom)
            if (_block_patterns_module_delta_exists($data['module'], $data['delta'])) {
              $result[] = array(PATTERNS_WARNING_ALREADY_DEFINED_ELEMENT =>
                  t('A block defined by module %module with delta %delta already exists in the system.',
                      array('%module' =>$data['module'], '%delta' =>$data['delta'])));
            }        
          }
          //We raised a semantic error if the theme does not exist in any case
          if (isset($data['theme']) && !array_key_exists($data['theme'], list_themes())) {
            $result[] = array(PATTERNS_WARNING_UNMET_DEPENDENCY =>
                t('The theme %theme is not currently installed in the system.', array('%theme' =>$data['theme'],)));
          }
          break;
        case PATTERNS_MODIFY:
          //Create semantic warning if the combination of module+delta+theme does not exist
          if (!_block_patterns_mdt_exists($data['module'], $data['delta'], $data['theme'])) {
            $result[] = array(PATTERNS_WARNING_ELEMENT_UNDEFINED =>
                t('There are not any blocks defined by module %module whose delta is %delta for theme %theme in the system.',
                    array('%module' =>$data['module'], '%theme' => $data['theme'], '%delta' => $data['delta'])));
          }
          break;
        case PATTERNS_DELETE:
          //Create semantic warning if the combination of module+delta does not exist
          if (!_block_patterns_module_delta_exists($data['module'], $data['delta'])) {
            $result[] = array(PATTERNS_WARNING_ELEMENT_UNDEFINED =>
                t('There are not any blocks defined by module %module whose delta is %delta in the system.',
                    array('%module' =>$data['module'], '%delta' =>$data['delta'])));
          }
          //Create semantic warning if the operation is performed in a block that is not custom.
          if ($data['module']!='block') {
            $result[] = array(PATTERNS_WARNING_INCONSISTENT_OPERATION =>
                t('Delete operations are not supported for blocks created by the module %module. The execution 
                    of this action might create some inconsistencies in the Database. Use MODIFY if you want to disable the block instead.',
                    array('%module' =>$data['module'])));
          }
          break;
      }
      break;
    case 'block_node_type':
      switch ($action) {
        case PATTERNS_CREATE:
          //Create semantic warning if the combination of module+delta does not exist
          if (!_block_patterns_module_delta_exists($data['module'], $data['delta'])) {
            $result[] = array(PATTERNS_WARNING_ELEMENT_UNDEFINED =>
                t('There are not any blocks defined by module %module whose delta is %delta in the system.',
                    array('%module' =>$data['module'], '%delta' =>$data['delta'])));
          }
          //Create semantic warning if the content type does not exist
          if (!array_key_exists($data['type'], node_type_get_names())) {
            $result[] = array(PATTERNS_WARNING_UNMET_DEPENDENCY =>
                t('There are not any content types named %type in the system.',
                    array('%type' =>$data['type'])));            
          }
          //Create semantic warning if the combination of module+delta+type already exists
          if (_block_patterns_mdtype_exists($data['module'], $data['delta'], $data['type'])) {
            $result[] = array(PATTERNS_WARNING_NOT_UNIQUE_ALIAS =>
                t('An entry to restrict the visualization of the block defined by module %module with delta %delta 
                    for the Content Type %type has already being set in the system.',
                    array('%module' =>$data['module'], '%delta' =>$data['delta'], '%type' =>$data['type'])));
          }
          break;
        case PATTERNS_DELETE:
          //Create semantic warning if the combination of module+delta+type does not exist
          if (!_block_patterns_mdtype_exists($data['module'], $data['delta'], $data['type'])) {
            $result[] = array(PATTERNS_WARNING_ELEMENT_UNDEFINED =>
                t('The restriction for the visualization of the block defined by module %module with delta %delta
                    for the Content Type %type trying to be deleted does not exists in the system.',
                    array('%module' =>$data['module'], '%delta' =>$data['delta'], '%type' =>$data['type'])));
          }
          break;
      }
      break;
    case 'block_role':
      switch ($action) {
        case PATTERNS_CREATE:
          //Create semantic warning if the combination of module+delta does not exist
          if (!_block_patterns_module_delta_exists($data['module'], $data['delta'])) {
            $result[] = array(PATTERNS_WARNING_ELEMENT_UNDEFINED =>
                t('There are not any blocks defined by module %module whose delta is %delta in the system.',
                    array('%module' =>$data['module'], '%delta' =>$data['delta'])));
          }
          //Raise semantic warning if the role name does not exist
          if (!user_role_load_by_name($data['role'])) {
            $result[] = array(PATTERNS_WARNING_UNMET_DEPENDENCY =>
                t('The role %role does not exist in the system.', array('%role' =>$data['role'],)));
          }
          //Create semantic warning if the combination of module+delta+role already exists
          if (_block_patterns_mdtype_role($data['module'], $data['delta'], $data['role'])) {
            $result[] = array(PATTERNS_WARNING_NOT_UNIQUE_ALIAS =>
                t('An entry to restrict the visualization of the block defined by module %module with delta %delta 
                    for role %role has already being set in the system.',
                    array('%module' =>$data['module'], '%delta' =>$data['delta'], '%role' =>$data['role'])));
          }
          break;
        case PATTERNS_DELETE:
          //Create semantic warning if the combination of module+delta+role does not exist
          if (!_block_patterns_mdtype_role($data['module'], $data['delta'], $data['role'])) {
            $result[] = array(PATTERNS_WARNING_ELEMENT_UNDEFINED =>
                t('The restriction for the visualization of the block defined by module %module with delta %delta
                    for the role %role trying to be deleted does not exists in the system.',
                    array('%module' =>$data['module'], '%delta' =>$data['delta'], '%role' =>$data['role'])));
          }
          break;
      }
      break;
  }
  return patterns_results($status, $msg, $result);
}


/**
 * 
 * Implements hook_patterns_cleanup().
 * 
 * @param string $action  Action currently run
 * @param string $tag  Tag processed. This is set with the form_id (or the name of the function in this component)
 * @param string $data  Set of values after parsing the action
 * 
 * @return Call to patterns_results
 */
function block_patterns_cleanup($action, $tag, &$data) {
  //Clear all caches after running any block operations ($tag is reset to the function name, acting as a form id)
  if ($tag=='block_create' || $tag == 'block_modify' || $tag == 'block_delete') {
    cache_clear_all();
  }
  return patterns_results(PATTERNS_SUCCESS, t('Cache successfully cleaned'));
}

/**
 *
 * Wraps several calls to several drupal_form_submit to create a block. Depending on the 
 * type of block (custom or not), a different amount of records will be inserted.
 *
 * @param string $form_id  String containing the form ID. In the case of custom functions the value is empty.
 * @param array $form_state  Set of values after parsing the action.
 */
function block_create($form_id, &$form_state) {
  //In case the block is a custom one (created by block) we will need
  //to perform and extra insertion in the block_custom table (all fields are mandatory)
  //and to add an entry for each theme as in function block_add_block_form_submit() 
  if ($form_state['values']['module']=='block') {
    //Insert an unique specific record for block_custom
    $delta = db_insert('block_custom')
    ->fields(array(
      'body' => $form_state['values']['body'],
      'info' => $form_state['values']['info'],
      'format' => $form_state['values']['format'],
    ))
    ->execute();
    // Store block delta to allow other modules to work with new block.
    $form_state['values']['delta'] = $delta;
    
    //Insert specific record for the theme given in the pattern
    $query = db_insert('block')
    ->fields(array('visibility', 'pages', 'custom', 'title', 'module', 'theme', 'status', 'weight', 'delta', 'cache', 'region'));
    $query->values(array(
        'visibility' => (isset($form_state['values']['visibility'])) ? (int) $form_state['values']['visibility']: 1,
        'pages' => (isset($form_state['values']['pages'])) ? trim($form_state['values']['pages']): '',
        'custom' => (isset($form_state['values']['custom'])) ? (int) $form_state['values']['custom']: 0,
        'title' => $form_state['values']['title'],
        'module' => $form_state['values']['module'],
        'theme' => (isset($form_state['values']['theme'])) ? $form_state['values']['theme']: variable_get('theme_default', 'bartik'),
        'status' => (isset($form_state['values']['status'])) ? $form_state['values']['status']: 0,
        'weight' => (isset($form_state['values']['weight'])) ? $form_state['values']['weight']: 0,
        'delta' => $form_state['values']['delta'],
        'cache' => (isset($form_state['values']['cache'])) ? $form_state['values']['cache']: -1,
        'region' => (isset($form_state['values']['region'])) ? $form_state['values']['region']: -1,
    ));
    $query->execute();
    
    //Insert rest of values, for all themes but the one already given in the pattern
    $query = db_insert('block')->fields(array('visibility', 'pages', 'custom', 'title', 'module', 'theme', 'status', 'weight', 'delta', 'cache'));
    foreach (list_themes() as $key => $theme) {
      if ($theme->status && $theme->name != $form_state['values']['theme']) {
        $query->values(array(
            'visibility' => (int) $form_state['values']['visibility'],
            'pages' => trim($form_state['values']['pages']),
            'custom' => (int) $form_state['values']['custom'],
            'title' => $form_state['values']['title'],
            'module' => $form_state['values']['module'],
            'theme' => $theme->name,
            'status' => 0,
            'weight' => 0,
            'delta' => $form_state['values']['delta'],
            'cache' => DRUPAL_NO_CACHE,
        ));
      }
    }
    $query->execute(); 
  }else{
    //If it is not a custom block, we will just insert the new record with the values provided in the pattern
    $query = db_insert('block')
    ->fields(array('visibility', 'pages', 'custom', 'title', 'module', 'theme', 'status', 'weight', 'delta', 'cache', 'region'));
    $query->values(array(
        'visibility' => (isset($form_state['values']['visibility'])) ? (int) $form_state['values']['visibility']: 1,
        'pages' => (isset($form_state['values']['pages'])) ? trim($form_state['values']['pages']): '',
        'custom' => (isset($form_state['values']['custom'])) ? (int) $form_state['values']['custom']: 0,
        'title' => $form_state['values']['title'],
        'module' => $form_state['values']['module'],
        'theme' => (isset($form_state['values']['theme'])) ? $form_state['values']['theme']: variable_get('theme_default', 'bartik'),
        'status' => (isset($form_state['values']['status'])) ? $form_state['values']['status']: 0,
        'weight' => (isset($form_state['values']['weight'])) ? $form_state['values']['weight']: 0,
        'delta' => $form_state['values']['delta'],
        'cache' => (isset($form_state['values']['cache'])) ? $form_state['values']['cache']: -1,
        'region' => (isset($form_state['values']['region'])) ? $form_state['values']['region']: -1,
    ));
    $query->execute();    
  }
  return patterns_results(PATTERNS_SUCCESS, t('New block successfully added'));
}

/**
 *
 * Wraps several calls to several drupal_form_submit to update the settings of a block.
 * This operation affects only the block table, block_custom table is not affected in case
 * is a custom block
 *
 * @param string $form_id  String containing the form ID. In the case of custom functions the value is empty.
 * @param array $form_state  Set of values after parsing the action.
 */
function block_modify($form_id, &$form_state) {
  //We can use the all the optional fields provided by the pattern after removing the ones that compound the key
  $fields_to_remove = array('tag' => $form_state['values']['tag'], 'module' => $form_state['values']['module'] , 
      'delta' => $form_state['values']['delta'], 'theme' => $form_state['values']['theme']);
  $fields = array_diff_key($form_state['values'], $fields_to_remove);
  
  $query = db_update('block')
  ->fields($fields)
  ->condition('module', $form_state['values']['module'])
  ->condition('delta', $form_state['values']['delta'])
  ->condition('theme', $form_state['values']['theme'])
  ->execute();
  
  $msg = t('Updated the following fields for block created by module %module with delta %delta for theme %theme: %fields.',
      array('%module' => $form_state['values']['module'], '%delta' => $form_state['values']['delta'],
          '%theme' => $form_state['values']['theme'],'%fields' => implode(', ', array_keys($fields))));
  
  return patterns_results(PATTERNS_SUCCESS, $msg);
}

/**
 *
 * Wraps several a call to drupal_form_submit to delete the settings of a block.
 * This operation is only available for custom blocks (as well as it happens in the UI)
 * since blocks created by custom modules cannot be deleted (only disabled through status)
 *
 * @param string $form_id  String containing the form ID. In the case of custom functions the value is empty.
 * @param array $form_state  Set of values after parsing the action.
 */
function block_delete($form_id, &$form_state) {
  //This operation is only available for custom modules, and we need to delete
  //the information for all the tables
  db_delete('block_custom')
  ->condition('bid', $form_state['values']['delta'])
  ->execute();
  db_delete('block')
  ->condition('module', $form_state['values']['module'])
  ->condition('delta', $form_state['values']['delta'])
  ->execute();
  db_delete('block_role')
  ->condition('module', $form_state['values']['module'])
  ->condition('delta', $form_state['values']['delta'])
  ->execute();

  $msg = t('Removed all the configuration for custom block delta %delta.',
      array('%delta' => $form_state['values']['delta']));
  return patterns_results(PATTERNS_SUCCESS, $msg);
}


/**
 *
 * Wraps a call to drupal_form_submit to define a visualization relationship
 * between the given block and content type
 *
 * @param string $form_id  String containing the form ID. In the case of custom functions the value is empty.
 * @param array $form_state  Set of values after parsing the action.
 */
function block_node_type_create($form_id, &$form_state) {
  //All the fields are mandatory, so it is not necessary to perform any checking
  $query = db_insert('block_node_type')
  ->fields(array('module', 'delta', 'type'))
  ->values(array(
      'module' => $form_state['values']['module'],
      'delta' => $form_state['values']['delta'],
      'type' => $form_state['values']['type'],
  ))
  ->execute();
  $msg = t('The block defined by module %module with delta %delta will be shown for Content type %type.',
      array('%module' => $form_state['values']['module'], '%delta' => $form_state['values']['delta'],
          '%type' => $form_state['values']['type']));
  return patterns_results(PATTERNS_SUCCESS, $msg);
}

/**
 *
 * Wraps a call to drupal_form_submit to remove a visualization relationship
 * between the given block and content type
 *
 * @param string $form_id  String containing the form ID. In the case of custom functions the value is empty.
 * @param array $form_state  Set of values after parsing the action.
 */
function block_node_type_delete($form_id, &$form_state) {
  //All the fields are mandatory, so it is not necessary to perform any checking
  $query = db_delete('block_node_type')
  ->condition('module', $form_state['values']['module'])
  ->condition('delta', $form_state['values']['delta'])
  ->condition('type', $form_state['values']['type'])
  ->execute();
  $msg = t('The block defined by module %module with delta %delta will not be shown for Content type %type.',
      array('%module' => $form_state['values']['module'], '%delta' => $form_state['values']['delta'],
          '%type' => $form_state['values']['type']));
  return patterns_results(PATTERNS_SUCCESS, $msg);
}

/**
 *
 * Wraps a call to drupal_form_submit to define a visualization relationship
 * between the given block and role
 *
 * @param string $form_id  String containing the form ID. In the case of custom functions the value is empty.
 * @param array $form_state  Set of values after parsing the action.
 */
function block_role_create($form_id, &$form_state) {
  //Obtain the rid from the rolename, we don't need to check existance since we did it previously
  $rid = user_role_load_by_name($form_state['values']['role'])->rid;
  //All the fields are mandatory, so it is not necessary to perform any checking
  $query = db_insert('block_role')
  ->fields(array('module', 'delta', 'rid'))
  ->values(array(
      'module' => $form_state['values']['module'],
      'delta' => $form_state['values']['delta'],
      'rid' => $rid,
  ))
  ->execute();
  $msg = t('The block defined by module %module with delta %delta will be shown for role %role.',
      array('%module' => $form_state['values']['module'], '%delta' => $form_state['values']['delta'],
          '%role' => $form_state['values']['role']));
  return patterns_results(PATTERNS_SUCCESS, $msg);
}

/**
 *
 * Wraps a call to drupal_form_submit to remove a visualization relationship
 * between the given block and role
 *
 * @param string $form_id  String containing the form ID. In the case of custom functions the value is empty.
 * @param array $form_state  Set of values after parsing the action.
 */
function block_role_delete($form_id, &$form_state) {
  //Obtain the rid from the rolename, we don't need to check existance since we did it previously
  $rid = user_role_load_by_name($form_state['values']['role'])->rid;
  //All the fields are mandatory, so it is not necessary to perform any checking
  $query = db_delete('block_role')
  ->condition('module', $form_state['values']['module'])
  ->condition('delta', $form_state['values']['delta'])
  ->condition('rid', $rid)
  ->execute();
  $msg = t('The block defined by module %module with delta %delta will not be shown for role %role.',
      array('%module' => $form_state['values']['module'], '%delta' => $form_state['values']['delta'],
          '%type' => $form_state['values']['role']));
  return patterns_results(PATTERNS_SUCCESS, $msg);
}

/**
 * 
 * Checks the existance of a block with the given module and delta values.
 * 
 * @param string $module  Module from which the block originates
 * @param string $delta  Unique ID for a block within a module
 * 
 * @return boolean  TRUE if there is a block matching both parameters, FALSE otherwise
 */
function _block_patterns_module_delta_exists($module, $delta) {
  $query = db_select('block', 'b')
  ->fields('b')
  ->condition('module', $module)
  ->condition('delta', $delta)
  ->execute();

  return $query->rowCount() > 0;
}

/**
 *
 * Checks the existance of a block with the given module, delta and theme values (unique constraint).
 *
 * @param string $module  Module from which the block originates
 * @param string $delta  Unique ID for a block within a module
 * @param string $theme  Theme key
 *
 * @return boolean  TRUE if there is a block matching both parameters, FALSE otherwise
 */
function _block_patterns_mdt_exists($module, $delta, $theme) {
  $query = db_select('block', 'b')
  ->fields('b')
  ->condition('module', $module)
  ->condition('delta', $delta)
  ->condition('theme', $theme)
  ->execute();

  return $query->rowCount() > 0;
}

/**
 *
 * Checks the existance of a block info in the block_custom table (unique constraint).
 *
 * @param string $info  Info field to be checked
 *
 * @return boolean  TRUE if there is a block matching both parameters, FALSE otherwise
 */
function _block_patterns_info_exists($info) {
  $query = db_select('block_custom', 'bc')
  ->fields('bc')
  ->condition('info', $info)
  ->execute();

  return $query->rowCount() > 0;
}

/**
 *
 * Checks the existance of a block - content type relationship with the given 
 * module, delta and content type values (unique constraint).
 *
 * @param string $module  Module from which the block originates
 * @param string $delta  Unique ID for a block within a module
 * @param string $type  Content Type in machine_name format
 *
 * @return boolean  TRUE if there is a configuration matching all the parameters, FALSE otherwise
 */
function _block_patterns_mdtype_exists($module, $delta, $type) {
  $query = db_select('block_node_type', 'bnt')
  ->fields('bnt')
  ->condition('module', $module)
  ->condition('delta', $delta)
  ->condition('type', $type)
  ->execute();

  return $query->rowCount() > 0;
}

/**
 *
 * Checks the existance of a block - role relationship with the given
 * module, delta and rolename values (unique constraint).
 *
 * @param string $module  Module from which the block originates
 * @param string $delta  Unique ID for a block within a module
 * @param string $role  Role in rolename format
 *
 * @return boolean  TRUE if there is a configuration matching all the parameters, FALSE otherwise
 */
function _block_patterns_mdtype_role($module, $delta, $role) {
  //Obtain the rid from the rolename, we don't need to check existance since we did it previously
  $rid = user_role_load_by_name($role)->rid;
  $query = db_select('block_role', 'br')
  ->fields('br')
  ->condition('module', $module)
  ->condition('delta', $delta)
  ->condition('rid', $rid)
  ->execute();

  return $query->rowCount() > 0;
}

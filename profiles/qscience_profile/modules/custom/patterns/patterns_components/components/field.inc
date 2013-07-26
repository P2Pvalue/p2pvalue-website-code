<?php
/**
 * @file
 * Patterns component for Fields.
 */
function field_patterns($data= NULL) {

  $files = array('modules/field_ui/field_ui.admin.inc', 'modules/field/field.info.inc');
  $actions['field'] = array(
    PATTERNS_INFO => t('Create/Modify/Delete field'),
    PATTERNS_MODIFY => array('field_ui_field_settings_form'),
    PATTERNS_FILES => $files,
    PATTERNS_EXPORT => array(
      PATTERNS_EXPORT_ALL => 'field_patterns_export_all_fields',
    ),
  );
  $actions['instance'] = array(
    PATTERNS_INFO => t('Create/Modify/Delete field'),
    PATTERNS_CREATE => array('field_ui_field_overview_form'),
    PATTERNS_MODIFY => array('field_ui_field_edit_form'),
    PATTERNS_DELETE => array('field_ui_field_delete_form'),
    PATTERNS_FILES => $files,
    PATTERNS_EXPORT => array(
      PATTERNS_EXPORT_ALL => 'field_patterns_export_all_ins',
    ),
  );
/*  $actions['field_weight'] = array(
    PATTERNS_INFO => t('Order the fields'),
    PATTERNS_MODIFY => array('field_ui_field_overview_form'),
    PATTERNS_FILES => $files,
  );*/

  return $actions;

}

function field_patterns_export_all_ins($args = NULL, &$result = NULL) {
  //$info = field_patterns();
  _field_info_collate_fields(true);
  _field_info_collate_types(true);
  // $form_id = current($info['instance'][PATTERNS_MODIFY]);
  $instances = field_read_instances();
  $result = array();
  switch ($args['type']) {
    case PATTERNS_CREATE :
  		foreach ($instances as $voc) {
    		$field=(array) field_info_instance($voc['entity_type'], $voc['field_name'], $voc['bundle']);
    		$data = array('tag' => 'instance');
    		$data['field_name']=$voc['field_name'];
    		$data['entity_type']=$voc['entity_type'];
    		$data['bundle']=$voc['bundle'];
    		$data['label']=$voc['label'];
    		$data['description']=$voc['description'];
    		$data['settings']=$voc['settings'];
    		$data['widget']=$voc['widget'];
    		$data['display']=$voc['display'];
    		$data['required']=$voc['required'];
    		$action = array(PATTERNS_CREATE => $data);
    		array_push($result, $action);
  		}
  		break;
  	case PATTERNS_MODIFY :
  		foreach ($instances as $voc) {
    		$field=(array) field_info_instance($voc['entity_type'], $voc['field_name'], $voc['bundle']);
    		$data = array('tag' => 'instance');
    		$data['field_name']=$voc['field_name'];
    		$data['entity_type']=$voc['entity_type'];
    		$data['bundle']=$voc['bundle'];
    		$data['label']=$voc['label'];
    		$data['description']=$voc['description'];
    		$data['settings']=$voc['settings'];
    		$data['widget']=$voc['widget'];
    		$data['display']=$voc['display'];
    		$data['required']=$voc['required'];
   	 		//$data = array_merge($data, $field);
    		$action = array(PATTERNS_MODIFY => $data);
    		array_push($result, $action);
  		}
  		break;
  }
  return $result;

}

function field_patterns_export_all_fields($args = NULL, &$result = NULL) {
  //  $info = field_patterns(NULL);
  _field_info_collate_fields(true);
  _field_info_collate_types(true);
  //  $form_id = current($info['field'][PATTERNS_MODIFY]);
  $instances = field_read_instances();
  $result = array();
  foreach ($instances as $voc) {
    $voc=(array) $voc;
    $data = array('tag' => 'field');
    $field = field_info_field($voc['field_name']);
    $data['field_name']=$voc['field_name'];
    $data['entity_type']=$voc['entity_type'];
    $data['bundle']=$voc['bundle'];
    $data['settings']=$field['settings'];
  //  $data = array_merge($data, $voc);
    $action = array(PATTERNS_MODIFY => $data);

    array_push($result, $action);
  }
  return $result;

}

/*
function field_patterns_export_all_weight($args = NULL, &$result = NULL){
//  $info = field_patterns(NULL);
  _field_info_collate_fields(true);
   _field_info_collate_types(true);
 //  $form_id = current($info['field_weight'][PATTERNS_MODIFY]);
    $instances = field_read_instances();
    $result = array();
    $returns=array();
    foreach ($instances as $voc) {
       $return = array(
        'entity_type' => $voc['entity_type'],
        'bundle' => $voc['bundle'],
      );
      $j = 0;
      foreach ($returns as $v) {
        if ($v['entity_type'] == $voc['entity_type'] && $v['bundle'] == $voc['bundle']) {
          $j = 1;
          break;
        }
      }
      if($j==0){
         array_push($returns, $return);

       $data = array('tag' => 'field_weight');
     $data = array_merge($data, $return);
  $action = array(PATTERNS_CREATE => $data);

      array_push($result, $action);
      }

    }
     return $result;

}*/
// returns arguments needed for the forms when processing automatically
// $loop is set if we want to extract all values
function field_patterns_get_arguments($action, $tag, $form_id, &$loop = FALSE) {
  _field_info_collate_fields(true);
  _field_info_collate_types(true);
  $return = array();
  if ($form_id == 'field_ui_field_settings_form') {

    $loop = TRUE;
    $instances = field_read_instances();
    foreach ($instances as $voc) {
      array_push($return, array($voc));
    }
  }
  elseif ($form_id == 'field_ui_field_edit_form') {
    $loop = TRUE;
    $instances = field_read_instances();
    foreach ($instances as $voc) {
      array_push($return, array(field_info_instance($voc['entity_type'], $voc['field_name'], $voc['bundle'])));
    }
  }
 /* elseif ($form_id == 'field_ui_field_overview_form') {
    $loop = TRUE;
    $instances = field_read_instances();
    foreach ($instances as $voc) {
      $result = array(
        'entity_type' => $voc['entity_type'],
        'bundle' => $voc['bundle'],
      );
      $j = 0;
      foreach ($return as $v) {
        if ($v['entity_type'] == $voc['entity_type'] && $v['bundle'] == $voc['bundle']) {
          $j = 1;
          break;
        }
      }
      if ($j == 0) {
        array_push($return, $result);
      }
    }
  }*/

  return $return;
}

// Prepare data for processing
function field_patterns_prepare($action, $tag, &$data = NULL) {
  $prefix = '';
  if ($action == PATTERNS_DELETE || $action == PATTERNS_MODIFY) {
    $prefix = 'field_';
  }
// @TODO: remove unnecessary stuff from $data array;
  if (isset($data['name'])) {
    $data['field_name'] = $prefix . $data['name'];
    unset($data['name']);   
  }
  if (isset($data['entity'])) {
    $data['entity_type'] = $data['entity'];
    unset($data['entity']);
  }
  // FIELD
  if ($tag == 'instance') {
    // DELETE
    if ($action == PATTERNS_DELETE) {
      $data['confirm'] =  1;
      $data['op'] = 'Delete';
    }
    // CREATE/MODIFY
    else {
      $data = _field_patterns_prepare_instance($action, $data);
    }
  }
  // End: $id = field
  elseif ($tag == 'field') {
    $data = _field_patterns_prepare_field($action, $data);
  }
 /* else {
    foreach ($data['weight'] as $key => $lev) {
      $name =  $prefix . $key;
      $data['fields'][$name]['weight'] = $lev;
    }
    unset($data['weight']);
  }*/

  return patterns_results();
}

// Validate the values for an action before running the pattern
function field_patterns_validate($action, $tag, &$data) {
		$result = array();
  $status = PATTERNS_SUCCESS;
  $msg = '';
	/*
 * Syntactic validation
 * 
 */
 switch ($tag) {
    case 'field':
	switch ($action) {
        case PATTERNS_CREATE:
        	$msg .= t('Field could not be created without identify a instance.<br>');
            return patterns_results(PATTERNS_ERR, $msg);
          break;
        case PATTERNS_MODIFY:
         	//Check mandatory fields.
          $mandatory_attributes = array('field_name','entity_type','bundle');
          if (!_patterns_has_all_mandatory_attributes($data, $mandatory_attributes, $msg)) {
            return patterns_results(PATTERNS_ERR, $msg);
          }
          //In this case we will need to define as well the attributes generated by the hook_prepare: uid and pass
          $interpretable_attributes = array('field_name','entity_type','bundle','settings','field');
          if(_patterns_has_uninterpretable_attributes($data, $interpretable_attributes, $msg)) {
            $status = PATTERNS_WARN;
          }
          break;
        case PATTERNS_DELETE:
          //Check mandatory fields, in this case is only one.
          $msg .= t('Field could not be deleted independently outside a indtance.<br>');
            return patterns_results(PATTERNS_ERR, $msg);
          break;
      }
      break;
   case 'instance':
      switch ($action) {
        case PATTERNS_CREATE:
        	//Check mandatory fields.
          $mandatory_attributes = array('field_name','entity_type','bundle','widget');
          if (!_patterns_has_all_mandatory_attributes($data, $mandatory_attributes, $msg)) {
            return patterns_results(PATTERNS_ERR, $msg);
          }
          //In this case we will need to define as well the attributes generated by the hook_prepare: uid and pass
          $interpretable_attributes = array('field_name','entity_type','bundle','label','description','settings','required','widget','display','fields');
          if(_patterns_has_uninterpretable_attributes($data, $interpretable_attributes, $msg)) {
            $status = PATTERNS_WARN;
          }
          break;
        case PATTERNS_MODIFY:
          //Check mandatory fields
          $mandatory_attributes = array('field_name','entity_type','bundle');
          if (!_patterns_has_all_mandatory_attributes($data, $mandatory_attributes, $msg)) {
            return patterns_results(PATTERNS_ERR, $msg);
          }
          
          $interpretable_attributes = array('field_name','entity_type','bundle','instance','field','fields','settings','required','widget','display',$data['field_name']);
          if(_patterns_has_uninterpretable_attributes($data, $interpretable_attributes, $msg)) {
            $status = PATTERNS_WARN;
          }
          break;
        case PATTERNS_DELETE:
           //Check mandatory fields
          $mandatory_attributes = array('field_name','entity_type','bundle','confirm','op');
          if (!_patterns_has_all_mandatory_attributes($data, $mandatory_attributes, $msg)) {
            return patterns_results(PATTERNS_ERR, $msg);
          }
          //In this case there are not optional attributes, so we can also use this set as interpretable attributes
          if(_patterns_has_uninterpretable_attributes($data, $mandatory_attributes, $msg)) {
            $status = PATTERNS_WARN;
          }
          break;
      }
      break;
 }
      /*
 * Semantic validation
 */
 $exist = field_read_instance($data['entity_type'], $data['field_name'],$data['bundle']); // TODO: count()
    
	switch ($tag) {
	case 'field':
      switch ($action) {
        case PATTERNS_MODIFY:
          //Create semantic warning if the rid does not exist
          if (!$exist) {
            $result[] = array(PATTERNS_WARNING_ELEMENT_UNDEFINED => 
              t('The field instance %field_name with entity type %entity_type in bundle %bundle dose not exist in the system.', array('%field_name' =>$data['field_name'],'%entity_type'=>$data['entity_type'],'%bundle'=>$data['bundle'])));
          }
          break;
          break;
      }
     break;
      
    case 'instance':     
      switch ($action) {
        case PATTERNS_CREATE:
            //Create semantic warning if the role name has already been defined
          if ($exist) {
            $result[] = array(PATTERNS_WARNING_ALREADY_DEFINED_ELEMENT => 
              t('The field instance %field_name with entity type %entity_type in bundle %bundle already exists in the system.', array('%field_name' =>$data['field_name'],'%entity_type'=>$data['entity_type'],'%bundle'=>$data['bundle'])));
          }
          break;
        case PATTERNS_MODIFY:
          //Create semantic warning if the rid does not exist
          if (!$exist) {
            $result[] = array(PATTERNS_WARNING_ELEMENT_UNDEFINED => 
              t('The field instance %field_name with entity type %entity_type in bundle %bundle dose not exist in the system.', array('%field_name' =>$data['field_name'],'%entity_type'=>$data['entity_type'],'%bundle'=>$data['bundle'])));
          }
          break;
        case PATTERNS_DELETE:
          if (!$exist ) {
            $result[] = array(PATTERNS_WARNING_ELEMENT_UNDEFINED => 
             t('The field instance %field_name with entity type %entity_type in bundle %bundle dose not exist in the system.', array('%field_name' =>$data['field_name'],'%entity_type'=>$data['entity_type'],'%bundle'=>$data['bundle'])));
          }
          break;
      }
      break;
  }
  return patterns_results($status, $msg, $result);
	
 /* $status = PATTERNS_SUCCESS;
  $msg = '';
  if ($tag == 'field_weight') {
    if (!isset($data['entity_type'])) {
      $status = PATTERNS_ERR;
      $msg = t('FieldException: Field entity type is required.');
    }
    elseif (!isset($data['bundle'])) {
      $status = PATTERNS_ERR;
      $msg = t('FieldException: Field bundle is required.');
    }
  }
  if ($tag == 'field') {
    if (!isset($data['field_name'])) {
      $status = PATTERNS_ERR;
      $msg = t('FieldException: Field name is required.');
    }
  }
  if ($tag == 'instance') {
    $exist = db_query("SELECT COUNT(*) FROM {field_config_instance} WHERE field_name = :name and entity_type = :type and bundle = :bundle", array('bundle' => $data['bundle'], 'type' => $data['entity_type'], 'name' => $data['field_name']))->fetchField(); // TODO: count()
    if (!isset($data['field_name'])) {
      $status = PATTERNS_ERR;
      $msg = t('FieldException: Field name is required.');
    }
    elseif (!isset($data['entity_type'])) {
      $status = PATTERNS_ERR;
      $msg = t('FieldException: Field entity type is required.');
    }
    elseif (!isset($data['bundle'])) {
      $status = PATTERNS_ERR;
      $msg = t('FieldException: Field bundle is required.');
    }
    elseif ($action == PATTERNS_CREATE && $exist) {
      $status = PATTERNS_ERR;
      $msg = t('Add new field: the field name %name already exists.', array('%name' => $data['field_name']));
    }
    elseif ($action == PATTERNS_MODIFY && !$exist) {
      $status = PATTERNS_ERR;
      $msg = t('FieldException: Attempt to update an instance of a nonexistent field.');
    }
    elseif ($action == PATTERNS_DELETE && !$exist) {
      $status = PATTERNS_ERR;
      $msg = t('FieldException: Attempt to delete an instance of a nonexistent field');
    }
  }

  return patterns_results($status, $msg);*/

}

// Return which callback functions to actually use.
function field_patterns_callbacks($action, $tag, &$data) {
  $desc = field_patterns();
  $result = $desc[$tag][$action];
  return patterns_results(PATTERNS_SUCCESS, t('Execution successful'), $result);
}

// Prepare for valid processing of this type of component
function field_patterns_build($action, $tag, &$data = NULL, $form_id) {
  //content_clear_type_cache();
  module_load_include('inc', 'field_ui', 'field_ui.admin');
  return patterns_results(PATTERNS_SUCCESS,     t('Execution successful'), $data);
}

// Build a patterns actions and parameters
function field_patterns_params($action, $form_id, &$data) {
  if ($form_id == 'field_ui_field_overview_form') {
    $result = array($data['entity_type'], $data['bundle']);
  }
  elseif ($form_id == 'field_ui_field_edit_form' ) {
    _field_info_collate_fields(true);
    _field_info_collate_types(true);
    $instance = field_info_instance($data['entity_type'], $data['field_name'], $data['bundle']);
    $result = array($instance);
  }
  elseif ( $form_id == 'field_ui_field_delete_form') {

    /*
     * delete all the field instance with the field_name
     * $fname = $data['field_name'];
    $instance = db_query("SELECT * FROM {field_config_instance} WHERE field_name = :fname", array('fname' => $fname))->fetchAssoc();
     */
    $result = array(array(
      'bundle' => $data['bundle'],
      'entity_type' => $data['entity_type'],
      'field_name' => $data['field_name'],
    ));
  }
  elseif ($form_id == 'field_ui_field_settings_form') {
    $result = array(array(
      'bundle' => $data['bundle'],
      'entity_type' => $data['entity_type'],
      'field_name' => $data['field_name'],
    ));
  }
  return patterns_results(PATTERNS_SUCCESS, t('Execution successful'), $result);
}

function field_patterns_cleanup($action, $tag, &$data) {
  return patterns_results();
}

function _field_patterns_prepare_instance($action, $data) {

  $existing_f  = '_add_existing_field';
  $new_f       = '_add_new_field';

  $all_fields = field_info_fields();
  $existing = array_key_exists($data['field_name'], $all_fields) ? $existing_f : $new_f;
  $data['fields'] = array();
  $fname = $data['field_name'];

  // Setting default values which can be overriden later

  $default_type = 'text'; //text_textfield';
  // $default_parent = '';
  $default_weight = 1;
  $default_widget_type = 'text_textfield';
  $default_cardinality = 1;
  $default_label = $fname;
  if ($action == PATTERNS_CREATE) {
    // NAME
    $data['fields'][$existing]['field_name'] = $fname;
   // unset($data['name']);
    // LABEL
    if (isset($data['label'])) {
      $data['fields'][$existing]['label'] = $data['label'];
      unset($data['label']);
    }
    else {
      $data['fields'][$existing]['label'] = $default_label;
    }

    // WEIGHT
    if (isset($data['weight'])) {
      $data['fields'][$existing]['weight'] = $data['weight'];
      unset($data['weight']);
    }
    else {
      $data['fields'][$existing]['weight'] = $default_weight;
    }

    // PARENT(unused parameter?)
    /*
     if (isset($data['parent'])) {
     $data['fields'][$EXISTING]['parent'] = $data['parent'];
     unset($data['parent']);
     }
     else {
     $data['fields'][$EXISTING]['parent'] = $default_parent;
    }*/

    // TYPE
    if (isset($data['type'])) {
      $data['fields'][$existing]['type'] = $data['type'];
      unset($data['type']);
    }
    else {
      $data['fields'][$existing]['type'] = $default_type;
    }

    // WIDGET

    // @TODO: check for incompatible widget_type and field_type
    // @TODO: add aliases for long names

    // Term Reference:
    // taxonomy_autocomplete
    // options_buttons
    if (isset($data['widget'])) {

      switch ($data['widget']) {

      case 'autocomplete':
        $w = 'taxonomy_autocomplete';
        break;
      case 'buttons':
        $w = 'options_buttons';
        break;
      default:
        $w = $data['widget'];
      }
      $data['fields'][$existing]['widget_type'] = $w;
     // unset($data['widget']);
    }
    else {
      $data['fields'][$existing]['widget_type'] =  $default_widget_type;
    }
  }
  // INSTANCE
  elseif ($action == PATTERNS_MODIFY) {
    // INSTANCE
    if (isset($data['label'])) {
      $data['instance']['label'] = $data['label'];
    }
    // REQUIRED
    if (isset($data['required']) && $data['required']) {
      $data['instance']['required'] = 1;
      unset($data['required']);
    }
    // HELP DESCR
    if (isset($data['description'])) {
      $data['instance']['description'] = $data['description'];
      unset($data['description']);
    }
    // CARDINALITY
    if (isset($data['cardinality'])) {
      //$c = array('und' => array(array('value' =>$data['cardinality'],'safe_value'=>$data['cardinality'])));
      $data['field']['cardinality'] = $data['cardinality'];
      unset($data['cardinality']);
    }
    else {
      $data['field']['cardinality'] = $default_cardinality;
    }
    if (isset($data['text_processing'])) {
      //$c = array('und' => array(array('value' =>$data['cardinality'],'safe_value'=>$data['cardinality'])));
      $data['instance']['settings']['text_processing'] = $data['text_processing'];
      unset($data['text_processing']);
    }
    if (isset($data['max_length'])) {
      //$c = array('und' => array(array('value' =>$data['cardinality'],'safe_value'=>$data['cardinality'])));
      $data['field']['settings']['max_length'] = $data['max_length'];
      unset($data['max_length']);
    }
    if (isset($data['summary'])) {
      //$c = array('und' => array(array('value' =>$data['cardinality'],'safe_value'=>$data['cardinality'])));
      $data['instance']['settings']['display_summary'] = $data['summary'];
      unset($data['summary']);
    }
    if (isset($data['rows'])) {
      //$c = array('und' => array(array('value' =>$data['cardinality'],'safe_value'=>$data['cardinality'])));
      $data['instance']['widget']['settings']['rows'] = $data['rows'];
      $data['instance']['widget']['active'] = 1;
      unset($data['rows']);
    }
    if (isset($data['size'])) {
      //$c = array('und' => array(array('value' =>$data['cardinality'],'safe_value'=>$data['cardinality'])));
      $data['instance']['widget']['settings']['size'] = $data['size'];
      $data['instance']['widget']['active'] = 1;
      unset($data['size']);
    }
    //default_value
    if (isset($data['default_value'])) {
      $data[$fname]['und'][0]['value'] = $data['default_value'];
      unset($data['default_value']);
    }

    //other special field type settings form
    if (isset($data['instance_settings'])) {
      foreach ($data['instance_settings'] as $key => $setting) {
        if (is_array($setting)) {
          foreach ($setting as $key1 => $set) {
            $data['instance']['settings'][$key][$key1] = $set;
          }
        }
        else {
          $data['instance']['settings'][$key] = $setting;
        }
      }
      unset($data['instance_settings']);
    }
    if (isset($data['field_settings'])) {
      foreach ($data['field_settings'] as $key => $setting) {
        if (is_array($setting)) {
          foreach ($setting as $key1 => $set) {
            $data['field']['settings'][$key][$key1] = $set;
          }
        }
        else {
          $data['field']['settings'][$key] = $setting;
        }
      }
      unset($data['field_settings']);
    }
    if (isset($data['widget_settings'])) {

      foreach ($data['widget_settings'] as $key => $setting) {
        if (is_array($setting)) {
          foreach ($setting as $key1 => $set) {
            $data['instance']['widget']['settings'][$key][$key1] = $set;
          }
        }
        else {
          $data['instance']['widget']['settings'][$key] = $setting;
        }
      }
      $data['instance']['widget']['active'] = 1;
      unset($data['widget_settings']);
    }
  }
  return $data;
}

function _field_patterns_prepare_field($action, $data) {
  $data['field'] = array();
  foreach ($data['settings'] as $key => $setting) {
    $data['field']['settings'][$key] = $setting;
  }
  //unset($data['settings']);
  return $data;
}


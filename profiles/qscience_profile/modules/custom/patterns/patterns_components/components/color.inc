<?php
/*
 * @file
 * Patterns component for pathauto.
 */
function color_patterns($data= NULL) {
  $files = array('modules/system/system.module', 'modules/system/system.admin.inc');

  $actions['color'] = array(
    PATTERNS_INFO => t('Setting color'),
    PATTERNS_MODIFY => array('system_theme_settings'),
    PATTERNS_FILES => $files,
    PATTERNS_EXPORT => array(
    PATTERNS_EXPORT_ALL => 'color_patterns_export_all_color', ),
  );

  return $actions;

}


function color_patterns_export_all_color($args = NULL, &$result = NULL) {
//  $info = color_patterns(NULL);
//  $form_id = current($info['color'][PATTERNS_MODIFY]);
  $themes = system_rebuild_theme_data();

  $result = array();
  // TODO: rename variables.
  foreach ($themes as $voc) {
    if (!empty($voc->info['hidden'])) {
      continue;
    }
    if (color_get_info($voc->name)) {
      $vocp =array('palette' => variable_get('color_' . $voc->name . '_palette', array()));

      $data = array('tag' => 'color', 'theme' => $voc->name);
      $data = array_merge($data, $vocp);
      
      $current_scheme = variable_get('color_' . $voc->name . '_palette', array());
      if (empty($current_scheme)) {
      	$scheme_name = array('scheme'=>'default');
      	$data = array_merge($data, $scheme_name);
      }
      
      $action = array(PATTERNS_MODIFY => $data);
      array_push($result, $action);

      //   $result = array_merge($result, patterns_export_actions_from_form($form_id, $voc->name, 'color', PATTERNS_MODIFY));
    }
  }
  return $result;
}
// returns arguments needed for the forms when processing automatically
// $loop is set if we want to extract all values
function color_patterns_get_arguments($action, $tag, $form_id, &$loop = FALSE) {
  $return = array();
  $themes = system_rebuild_theme_data();
  $loop = TRUE;
  foreach ($themes as $voc) {
    if (!empty($theme->info['hidden'])) {
      continue;
    }
    if (color_get_info($voc->name)) {
      array_push($return, array($voc->name));
    }
  }

  return $return;
}

// Prepare data for processing
function color_patterns_prepare($action, $tag, &$data) {
  if (!isset($data['scheme'])) {
    $data['scheme'] = '';
  }
  return patterns_results();
}

// Validate the values for an action before running the pattern
function color_patterns_validate($action, $tag, &$data) {
	$result = array();
  $status = PATTERNS_SUCCESS;
  $msg = '';
	/*
 * Syntactic validation
 * 
 */
	switch ($action) {
        case PATTERNS_CREATE:
        	$msg .= t('Color could not be created. Only modify action is possible for Color.<br>');
            return patterns_results(PATTERNS_ERR, $msg);
          break;
        case PATTERNS_MODIFY:
          //Check mandatory fields
          $mandatory_attributes = array('theme');
          if (!_patterns_has_all_mandatory_attributes($data, $mandatory_attributes, $msg)) {
            return patterns_results(PATTERNS_ERR, $msg);
          }
          //All the fields but uid are in this case optional
          $interpretable_attributes = array('theme','palette', 'scheme');
          if(_patterns_has_uninterpretable_attributes($data, $interpretable_attributes, $msg)) {
            $status = PATTERNS_WARN;
          }
          break;
        case PATTERNS_DELETE:
          //Check mandatory fields, in this case is only one.
          $msg .= t('Color could not be Deleted. Only modify action is possible for Color.<br>');
            return patterns_results(PATTERNS_ERR, $msg);
          break;
      }
      /*
 * Semantic validation
 */
  	$themes = system_rebuild_theme_data();
  	$current_themes=array();
  	foreach ($themes as $key=>$voc) {
    	$current_themes[$key]=$voc->name;
    }
	 switch ($action) {
        case PATTERNS_MODIFY:
          $theme = $data['theme'];
          //Create semantic warning if the theme does not exist
          if (!array_key_exists($theme, $current_themes)) {
            $result[] = array(PATTERNS_WARNING_ELEMENT_UNDEFINED => 
              t('The theme %theme does not exist in the system.', array('%theme' =>$theme)));
          }
          break;
      }
      return patterns_results($status, $msg, $result);
	
 /* $status = PATTERNS_SUCCESS;
  $msg = '';
  foreach ($data['palette'] as $key => $color) {
    if (!preg_match('/^#([a-f0-9]{3}){1,2}$/iD', $color)) {
      $msg = t('%name must be a valid hexadecimal CSS color value.', array('%name' => $data['palette'][$key]));
      break;
    }
  }
  return patterns_results($status, $msg);*/
}

// Prepare for valid processing of this type of component
function color_patterns_build($action, $tag, &$data, $form_id) {

  return patterns_results(PATTERNS_SUCCESS, t('Execution successful'), $data);
}

// Build a patterns actions and parameters
function color_patterns_params($action, $form_id, &$data, &$a) {
  $result = array($data['theme']);

  return patterns_results(PATTERNS_SUCCESS, t('Execution successful'), $result);
}

function color_patterns_cleanup($action, $tag, &$data) {
  return patterns_results();
}

// Return which callback functions to actually use.
function color_patterns_callbacks($action, $tag, &$data) {
  $desc = color_patterns();
  $result = $desc[$tag][$action];
  return patterns_results(PATTERNS_SUCCESS, t('Execution successful'), $result);
}

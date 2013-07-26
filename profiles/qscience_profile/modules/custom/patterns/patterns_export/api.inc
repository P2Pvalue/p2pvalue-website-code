<?php
/**
 * @file
 * Export the components configuration to a Pattern file.
 *
 */

/**
 * TODO: finish Export documentation
 * TODO: Export through the Batch API
 */

/**
 * Constructs patterns actions based on the current state of a form 
 * 
 * @param string $form_id The name of the form
 * @param array $args (optional)
 *   Extra arguments to be passed to the form
 * @param string $tag (optional) 
 * 	 The tag associated with the action
 * @param string $action (optional)
 *   The pattern action (CREATE / MODIFY / DELETE). Defaults PATTERNS_MODIFY
 *   
 * @return array $result The pattern action
 */
function patterns_export_actions_from_form($form_id, $args = array(), $tag = NULL, $action = PATTERNS_MODIFY) {

  set_error_handler("patterns_export_error_handler", E_ERROR | E_CORE_ERROR | E_COMPILE_ERROR | E_USER_ERROR);
  // Move this inside the Macro module
  // Macro should check that only one user is using that
  $form_state = array();
  $form_state['values'] = array();
  variable_set('patterns_extract_actions', TRUE);
  variable_set('macro_enabled', TRUE);
  variable_set('macro_submissions', array());
  $args = (!is_array($args)) ? array($args) : $args;

  $form_state['build_info']['args'] = $args;
  drupal_form_submit($form_id, $form_state);

  $result = macro_get_macro();

  variable_set('macro_enabled', FALSE);
  variable_set('patterns_extract_actions', FALSE);

  if (!empty($result) || !is_null($tag)) {
    $out = array();
    foreach ($result as $entry) {
      $a = $entry['values'];
      $out[] = array($action => array_merge(array('tag' => $tag), $a));
    }
    $result = $out;
  }

  return $result;
}

/**
 * Implements of hook_form_alter()
 * 
 * Needed for deactivating actual submission of the forms when submitting, i.e.
 * we only want to know what gets submitted without the submission getting
 * processed
 * 
 */
function patterns_export_form_alter(&$form, $form_state, $form_id) {
  // Add the record callback on submit and delete all others
  if ($form_id != 'macro_import_macro' && variable_get('macro_enabled', FALSE) && variable_get('patterns_extract_actions', FALSE)) {
    $form['#submit'] = array();
    $form['#submit'][] = 'macro_record_macro';
  }
}

/**
 * Halts script execution in case of error
 */
function patterns_export_error_handler($errno, $errstr) {
  variable_set('macro_enabled', FALSE);
  variable_set('macro_submissions', array());
  variable_set('patterns_extract_actions', FALSE);
  restore_error_handler();
  // Normal error handler should take over from here.
  return FALSE;
}
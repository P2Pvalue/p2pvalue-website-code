<?php
/**
 * @file
 * Functions for adjusting the settings of D2D.
 */


function d2d_show_settings_general() {
  $build['general_settings'] = array(
    '#title' => t('General Settings'),
    '#type' => 'fieldset',
  );
  $build['general_settings'][] = drupal_get_form('d2d_form_settings_general');
  d2d_notify();
  return $build;
}
function d2d_form_settings_general() {
  $form = array();
  $form['utc_offset'] = array(
    '#type' => 'textfield',
    '#title' => t('UTC offset'),
    '#description' => t('Number of seconds that are added to the system time (in case the system time is incorrect and can not be adjusted directly).'),
    '#default_value' => variable_get('d2d_utc_offset', D2D_UTC_OFFSET_DEFAULT),
    '#size' => 10,
    '#maxlength' => 10,
    '#required' => TRUE,
  );
  $form['timeout'] = array(
    '#type' => 'textfield',
    '#title' => t('Timeout'),
    '#description' => t('Maximum time in seconds a remote procedure call may take.'),
    '#default_value' => variable_get('d2d_timeout', D2D_TIMEOUT_DEFAULT),
    '#size' => 10,
    '#maxlength' => 10,
    '#required' => TRUE,
  );
  $form['delta0'] = array(
    '#type' => 'textfield',
    '#title' => t('Incoming requests lower bound'),
    '#description' => t('Number of seconds the timestamp of an incoming request / function call may be older than the system time of receival.'),
    '#default_value' => variable_get('d2d_delta_0', D2D_DELTA_0_DEFAULT),
    '#size' => 10,
    '#maxlength' => 10,
    '#required' => TRUE,
  );
  $form['delta1'] = array(
    '#type' => 'textfield',
    '#title' => t('Incoming requests upper bound'),
    '#description' => t('Number of seconds the timestamp of an incoming request / function call may be newer than the system time of receival.'),
    '#default_value' => variable_get('d2d_delta_1', D2D_DELTA_1_DEFAULT),
    '#size' => 10,
    '#maxlength' => 10,
    '#required' => TRUE,
  );
  $form['submit'] = array(
    '#type' => 'submit', 
    '#value' => t('Save'),
  );
  return $form;
}
function d2d_form_settings_general_validate($form, &$form_state) {
  $utc_offset = $form_state['values']['utc_offset'];
  if (!d2d_check_convert_int($utc_offset)) {
    form_set_error('utc_offset', t('Offset must be an integer.'));
  }
  elseif ($utc_offset < D2D_UTC_OFFSET_MIN || $utc_offset > D2D_UTC_OFFSET_MAX) {
    form_set_error('utc_offset', t('Offset must be between @min and @max.', array('@min' => D2D_UTC_OFFSET_MIN, '@max' => D2D_UTC_OFFSET_MAX)));
  }
  $timeout = $form_state['values']['timeout'];
  if (!d2d_check_convert_int($timeout)) {
    form_set_error('timeout', t('Time until timeout must be an integer.'));
  }
  elseif ($timeout < D2D_TIMEOUT_MIN || $timeout > D2D_TIMEOUT_MAX) {
    form_set_error('timeout', t('Time until timeout must be between @min and @max.', array('@min' => D2D_TIMEOUT_MIN, '@max' => D2D_TIMEOUT_MAX)));
  }
  $delta0 = $form_state['values']['delta0'];
  if (!d2d_check_convert_int($delta0)) {
    form_set_error('delta0', t('Incoming requests lower bound must be an integer.'));
  }
  elseif ($delta0 < D2D_DELTA_0_MIN|| $delta0 > D2D_DELTA_0_MAX) {
    form_set_error('delta0', t('Incoming requests lower bound must be between @min and @max.', array('@min' => D2D_DELTA_0_MIN, '@max' => D2D_DELTA_0_MAX)));
  }
  $delta1 = $form_state['values']['delta1'];
  if (!d2d_check_convert_int($delta1)) {
    form_set_error('delta1', t('Incoming requests upper bound must be an integer.'));
  }
  elseif ($delta1 < D2D_DELTA_1_MIN|| $delta1 > D2D_DELTA_1_MAX) {
    form_set_error('delta1', t('Incoming requests upper bound must be between @min and @max.', array('@min' => D2D_DELTA_1_MIN, '@max' => D2D_DELTA_1_MAX)));
  }
}
function d2d_form_settings_general_submit($form, &$form_state) {
  $utc_offset = $form_state['values']['utc_offset'];
  $timeout = $form_state['values']['timeout'];
  $delta0 = $form_state['values']['delta0'];
  $delta1 = $form_state['values']['delta1'];
  d2d_check_convert_int($utc_offset);
  d2d_check_convert_int($timeout);
  d2d_check_convert_int($delta0);
  d2d_check_convert_int($delta1);
  variable_set('d2d_utc_offset', $utc_offset);
  variable_set('d2d_timeout', $timeout);
  variable_set('d2d_delta_0', $delta0);
  variable_set('d2d_delta_1', $delta1);
  drupal_set_message(t('Settings have been saved.'));
}
function d2d_show_settings_advanced() {
  $build['advanced_settings'] = array(
    '#title' => t('Advanced Settings'),
    '#type' => 'fieldset',
  );
  $build['advanced_settings'][] = drupal_get_form('d2d_form_settings_advanced');
  d2d_notify(array('offline'));
  return $build;
}
function d2d_form_settings_advanced() {
  $form = array();
  $form['online'] = array(
    '#type' => 'checkbox',
    '#default_value' => variable_get('d2d_online', FALSE),
    '#title' => t('D2D instance online'),
    '#description' => t('If set, this instance is reachable for other instances and will periodically communicate with other instances. Note that a public / private key pair has to be associated with this instance for being online.'),
  );
  $form['submit'] = array(
    '#type' => 'submit', 
    '#value' => t('Save'),
  );
  $public_key = d2d_api_own_public_key_get();
  $private_key = d2d_api_own_private_key_get();
  if (!variable_get('d2d_online', FALSE) && !d2d_check_key_pair($public_key, $private_key)) {
    $form['#disabled'] = TRUE;
    drupal_set_message(t('You must !set a valid public / private key pair before going online.', array('!set' => l(t('set'), 'admin/d2d/settings/keys'))), 'warning');
  }
  return $form;
}
function d2d_form_settings_advanced_validate($form, &$form_state) {
}
function d2d_form_settings_advanced_submit($form, &$form_state) {
  variable_set('d2d_online', $form_state['values']['online'] ? TRUE : FALSE);
  drupal_set_message(t('Settings have been saved.'));
}
function d2d_show_settings_keys() {
  $build['keys'] = array(
    '#title' => t('Public / Private Key Pair'),
    '#type' => 'fieldset',
  );
  $build['keys'][] = drupal_get_form('d2d_form_settings_keys');
  d2d_notify();
  return $build;
}
function d2d_form_settings_keys() {

  $form = array();
  $form['public_key'] = array(
    '#type' => 'textarea',
    '#title' => t('Public key'),
    '#description' => t('The public key of this instance'),
    '#rows' => 4,
    '#cols' => 60,
    '#default_value' => d2d_api_own_public_key_get(),
  );
  $form['private_key'] = array(
    '#type' => 'textarea',
    '#title' => t('Private key'),
    '#description' => t('The private key of this instance'),
    '#rows' => 4,
    '#cols' => 60,
    '#default_value' => d2d_api_own_private_key_get(),
  );
  $form['submit'] = array(
    '#type' => 'submit', 
    '#value' => t('Save'),
    '#validate' => array('d2d_form_settings_keys_validate'),
    '#submit' => array('d2d_form_settings_keys_submit'),
  );
  $form['generate_keys'] = array(
    '#type' => 'submit', 
    '#value' => t('Generate keys'),
    '#validate' => array('d2d_form_settings_keys_generate_validate'),
    '#submit' => array(),
  );
  if (d2d_is_online()) {
    $form['submit']['#disabled'] = TRUE;
    $form['generate_keys']['#disabled'] = TRUE;
    drupal_set_message(t('You must be in !link to change the public / private key pair.', array('!link' => l('offline mode', 'admin/d2d/settings/advanced'))), 'warning');
  }
  return $form;
}

function d2d_form_settings_keys_validate($form, &$form_state) {
  $public_key = $form_state['values']['public_key'];
  $private_key = $form_state['values']['private_key'];
  if (!d2d_check_key_pair($public_key, $private_key)) {
    form_set_error('', t('Public / private key pair erroneous.'));
  }
}

function d2d_form_settings_keys_submit($form, &$form_state) {
  $public_key = $form_state['values']['public_key'];
  $private_key = $form_state['values']['private_key'];  
  d2d_api_own_public_key_set($public_key);
  d2d_api_own_private_key_set($private_key);
  drupal_set_message(t('Settings have been saved.'));
}

function d2d_form_settings_keys_generate_validate($form, &$form_state) {
  if (!d2d_create_keys($public_key, $private_key)) {
    form_set_error('', t('Failed creating public / private key pair.'));
    return;
  }
  $form_state['complete form']['public_key']['#value'] = $public_key;
  $form_state['complete form']['private_key']['#value'] = $private_key;
  form_set_error(''); // TODO: this is basically a hack
  drupal_set_message(t('Public / private key pair has been generated. Click \'Save\' to proceed with that pair.'));
}

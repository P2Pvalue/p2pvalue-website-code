<?php
/**
 * @file
 * Functions for adjusting the settings of D2D.
 */

function d2d_show_init() {
  $build['init'] = array(
    '#title' => t('Welcome to D2D'),
    '#type' => 'fieldset',
  );
  $build['init'][] = drupal_get_form('d2d_form_init');
//  d2d_notify();
  return $build;
}
function _d2d_suggest_instance_name($base_url) {
  $array = array_reverse(explode('/', $base_url));
  foreach ($array as $token) {
    if (!empty($token)) {
      return $token;
    }
  }
  return 'My D2D';
}
function d2d_form_init() {
  $form = array();

  $form['introduction'] = array(
      '#markup' => t('Before using D2D, please provide a @length characters long D2D identifier. This identifier should be unique among all installations of D2D. It is recommended to generate that identifier randomly (e.g. by using the button below). If you installed D2D before, you can reuse the identifier of your old installation.', array('@length' => D2D_INSTANCE_IDENTIFIER_LENGTH)),
  );
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#description' => t('A short name describing your instance.'),
    '#default_value' => _d2d_suggest_instance_name($GLOBALS['base_url']),
    '#size' => D2D_INSTANCE_NAME_MAX_LENGTH,
    '#maxlength' => D2D_INSTANCE_NAME_MAX_LENGTH,
    '#required' => FALSE,
  );
  $form['id'] = array(
    '#type' => 'textfield',
    '#title' => t('D2D Identifier'),
    '#description' =>
      t(
        'Globally unique identifier consisting of exactly @length hexadecimal characters (A-F, 0-9).<br/>' . 
        'Note: once you have saved the global identifier, it cannot be changed anymore.',
        array('@length' => D2D_INSTANCE_IDENTIFIER_LENGTH)
      ),
    '#default_value' => d2d_random_d2d_id(),
    '#size' => D2D_INSTANCE_IDENTIFIER_LENGTH,
    '#maxlength' => D2D_INSTANCE_IDENTIFIER_LENGTH,
    '#required' => FALSE, // length is checked anyway, no error if 'generate'-button is pressed
  );
  $form['address'] = array(
    '#type' => 'textfield',
    '#title' => t('Address'),
    '#description' => t('The address under which this instance is reachable.'),
    '#default_value' => $GLOBALS['base_url'] . '/xmlrpc.php',
    '#size' => 40,
    '#maxlength' => D2D_INSTANCE_URL_MAX_LENGTH,
    '#required' => TRUE,
  );
  $form['auto_keys_and_online'] = array(
    '#type' => 'checkbox',
    '#default_value' => FALSE,
    '#title' => t('Automatically select public / private key pair and go online.'),
    '#description' => t('If selected, a random public / private key pair is automatically chosen and the instance will be marked as online, i.e. other instances will be able to see this instance and to communicate with this instance. Do not select this option if you want to manually set your public / private key pair, e.g. to reuse keys you have used with an old installation or if you do not want your instance to be online immediatelly.'),
  );
  $form['submit'] = array(
    '#type' => 'submit', 
    '#value' => t('Save and continue'),
  );
  $form['generate'] = array(
    '#type' => 'submit', 
    '#value' => t('Generate different random identifier'),
    '#validate' => array('d2d_form_init_generate_validate'),
    '#submit' => array(),
  );
  return $form;
}
function d2d_form_init_validate($form, &$form_state) {
  $id = $form_state['values']['id'];
  if (!d2d_check_d2d_id_length($id)) {
    form_set_error('id', t('Identifier must constist of exactly @length characters.', array('@length' => D2D_INSTANCE_IDENTIFIER_LENGTH)));
  }
  if (!d2d_is_hex_string($id)) {
    form_set_error('id', t('Identifier must consists only of hexadecimal characters (A-F, 0-9).'));
  }
  if (!d2d_check_url($form_state['values']['address'])) {
    form_set_error('address', t('Address must start with \'http://\' or \'https://\'.'));
  }
}
function d2d_form_init_submit($form, &$form_state) {
  // we create the key pair first to return on error without mofifying
  // the database
  if ($form_state['values']['auto_keys_and_online']) {
    if (!d2d_create_keys($my_public_key, $my_private_key)) {
      drupal_set_message(t('Failed creating public / private key pair.'), 'error');
      return;
    }
  }
  $my_d2d_id = $form_state['values']['id'];
  // add own instance to database
  $instance = array(
    'name' => $form_state['values']['name'],
    'url' => $form_state['values']['address'],
    'd2d_id' => $my_d2d_id,
    'description' => 'this instance.',
  );
  $my_id = d2d_api_instance_add($instance, FALSE);
  d2d_api_own_instance_id_set($my_id);
  d2d_api_own_d2d_id_set($my_d2d_id);
  if ($form_state['values']['auto_keys_and_online']) {
    $my_public_key_id = d2d_api_own_public_key_set($my_public_key);
    d2d_api_own_private_key_set($my_private_key);
    variable_set('d2d_online', TRUE);
  }
  $content = d2d_implode(array(
    'subject' => t('Welcome to D2D!'),
    'text' => t('As a first step you might want to add other Drupal installations running D2D to your list of instances and send them a friendship request. Have fun!'),
  ));
  d2d_api_notification_insert('d2d_message', $my_d2d_id, $content);
  menu_rebuild();
  drupal_set_message(t('Settings have been saved.'));
}
function d2d_form_init_generate_validate($form, &$form_state) {
  $id = d2d_random_d2d_id();
  $form_state['complete form']['id']['#value'] = $id;
  form_set_error(''); // TODO: this is basically a hack, better ways to do this?
  drupal_set_message(t('Click \'Save & continue\' to proceed with this random identifier.'));
}

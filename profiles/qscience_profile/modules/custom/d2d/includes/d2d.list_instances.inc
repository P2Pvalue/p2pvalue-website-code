<?php

/**
 * @file
 * Functions directly related to the list instances tab.
 */

function d2d_form_add_instance() {
  $form = array();
  $default_d2d_id = '';
  $default_url = '';
  if (isset($_SESSION['d2d_add_instance'])) {
    if ($_SESSION['d2d_add_instance']['prefill']) {
      $default_d2d_id = $_SESSION['d2d_add_instance']['d2d_id'];
      $default_url = check_plain($_SESSION['d2d_add_instance']['url']);
    }
    unset($_SESSION['d2d_add_instance']);
  }
  // receiver of the request
  $form['url'] = array(
    '#type' => 'textfield',
    '#title' => t('Address'),
    '#description' => t('Address of the instance, e.g. http://www.example.com/xmlrpc.php'),
    '#default_value' => $default_url,
    '#size' => 40,
    '#maxlength' => D2D_INSTANCE_URL_MAX_LENGTH,
    '#required' => TRUE,
  );
  $form['d2d_id'] = array(
    '#type' => 'textfield',
    '#title' => t('D2D identifier'),
    '#description' =>
      t(
        'Global unique identifier consisting of exactly @length hexadecimal characters (A-F, 0-9).<br/>',
        array('@length' => D2D_INSTANCE_IDENTIFIER_LENGTH)
      ),
    '#default_value' => $default_d2d_id,
    '#size' => D2D_INSTANCE_IDENTIFIER_LENGTH,
    '#maxlength' => D2D_INSTANCE_IDENTIFIER_LENGTH,
    '#required' => FALSE, // length is checked anyway, no error if 'generate'-button is pressed
  );
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#description' => t('A short name describing the instance.'),
    '#default_value' => '',
    '#size' => D2D_INSTANCE_NAME_MAX_LENGTH,
    '#maxlength' => D2D_INSTANCE_NAME_MAX_LENGTH,
    '#required' => FALSE,
  );
  $form['description'] = array(
    '#type' => 'textarea',
    '#title' => t('Description of the instance'),
    '#description' => t('An optional longer desciption of the newly added instance, no longer than @length characters.', array('@length' => D2D_INSTANCE_DESCRIPTION_MAX_LENGTH)),
    '#rows' => 4,
    '#cols' => 60,
    '#default_value' => t('N/A'),
    '#required' => FALSE,
  );
  $form['submit'] = array(
    '#type' => 'submit', 
    '#value' => t('Add instance'),
    '#validate' => array('d2d_form_add_instance_submit_validate'),
    '#submit' => array('d2d_form_add_instance_submit_submit'),
  );
  $form['auto_complete'] = array(
    '#type' => 'submit', 
    '#value' => t('Auto complete form'),
    '#validate' => array('d2d_form_add_instance_auto_complete_validate'),
  );
  return $form;
}
function d2d_form_add_instance_submit_validate($form, &$form_state) {
  d2d_form_add_instance_validate($form, $form_state);
  if (strlen($form_state['values']['description']) > D2D_INSTANCE_DESCRIPTION_MAX_LENGTH) {
    form_set_error('description', t('Provided description must be no longer than @max_length characters'), array('@max_length' => D2D_INSTANCE_DESCRIPTION_MAX_LENGTH));
  }
  if (!d2d_check_d2d_id($form_state['values']['d2d_id'])) {
    form_set_error('d2d_id', t('D2D identifier not valid.'));
  }
  else {
    if ($instance = d2d_api_instance_get($form_state['values']['d2d_id'])) {
      form_set_error(
        'd2d_id',
        t(
          'Instance with provided D2D identifier already exists in database, click !link to edit that instance.',
          array('!link' => l(t('here'), 'admin/d2d/instances/' . $instance['id'] . '/details'))
        )
      );
    }
  }
}
function d2d_form_add_instance_auto_complete_validate($form, &$form_state) {
  if (d2d_form_add_instance_validate($form, $form_state)) {
    $url = $form_state['values']['url'];
    $res = xmlrpc($url, array('d2d.getInstanceInformation' => array()));
    if ($res === FALSE) {
      form_set_error('', t('Error retrieving information from other instance.'));
    }
    else {
      $exploded_res = d2d_explode($res);
      if ($exploded_res === FALSE || !d2d_check_array($exploded_res, array('d2d_id' => 'd2d_check_d2d_id', 'name' => 'd2d_check_name', 'description' => 'd2d_check_description'))) {
        form_set_error('', t('Information received from other instance is invalid'));
      }
      else {
        $form_state['complete form']['d2d_id']['#value'] = $exploded_res['d2d_id'];
        if (strlen($form_state['values']['d2d_id']) > 0 && $form_state['values']['d2d_id'] != $exploded_res['d2d_id']) {
          drupal_set_message(t('D2D id has been overwritten by a different D2D id.'), 'warning');
        }
        $form_state['complete form']['name']['#value'] = check_plain($exploded_res['name']);
        $form_state['complete form']['description']['#value'] = check_plain($exploded_res['description']);
        drupal_set_message(t('Successfully received information from other instance.'));
        form_set_error(''); // TODO: this is basically a hack, again :)
      }
    }
  }
}
function d2d_form_add_instance_validate($form, &$form_state) {
  $okay = TRUE;
  if (!d2d_check_url($form_state['values']['url'])) {
    form_set_error('url', t('Address must start with \'http://\' or \'https://\'.'));
    $okay = FALSE;
  }
  return $okay;
}
function d2d_form_add_instance_submit_submit($form, &$form_state) {
  $instance = array(
    'd2d_id' => $form_state['values']['d2d_id'],
    'name' => $form_state['values']['name'],
    'url' => $form_state['values']['url'],
    'description' => $form_state['values']['description'],
  );
  $id = d2d_api_instance_add($instance);
  drupal_set_message(
    t(
      'Instance inserted in database. Click !link to configure it, associate a !public_key_link with it and send a friendship request.',
      array('!link' => l(t('here'), 'admin/d2d/instances/' . $id . '/details'), '!public_key_link' => l(t('public key'), 'admin/d2d/instances/' . $id . '/public_key'))
    )
  );
}
function theme_d2d_table_instances($variables) {
  $form = $variables['form'];
  $header = array(
    'id' => t('ID'),
    'name' => t('Name'),
    'd2d_id' => t('D2D ID'),
    'time' => t('Inserted'),
    'public_key' => t('Public key'),
    'friend' => t('Friend'),
    'operation' => array('data' => t('Operations'), 'colspan' => 2),
  );
  $rows = array();
  foreach (element_children($form['instances']) as $key) {
    $row = array();
    foreach ($header as $column_name => $title) {
      if (is_array($title)) {
        $column_count = $title['colspan'];
        for ($i = 1; $i <= $column_count; $i++) {
          $row[] = array(
            'data' => drupal_render($form['instances'][$key][$column_name . $i]),
          );
        }
      }
      else {
        $row[] = array(
          'data' => drupal_render($form['instances'][$key][$column_name]),
        );
      }
    }
    $rows[] = $row;
  }
  return theme('table', array(
    'header' => $header, 
    'rows' => $rows,
  ));
}
function d2d_table_instances($form, &$form_state) {
  $instances = d2d_api_instance_get_by_id();
  $form['instances'] = array(
    '#tree' => TRUE,
    '#theme' => 'd2d_table_instances',
  );
  foreach ($instances as $id => $instance) {
    $form['instances']['id' . $id]['id'] = array(
      '#type' => 'item',
      '#markup' => $id,
    );
    $form['instances']['id' . $id]['name'] = array(
      '#type' => 'item',
      '#markup' => check_plain($instance['name']),
      '#description' => check_plain($instance['url']),
    );
    $form['instances']['id' . $id]['d2d_id'] = array(
      '#type' => 'item',
      '#markup' => $instance['d2d_id'],
    );
    $form['instances']['id' . $id]['time'] = array(
      '#type' => 'item',
      '#markup' => d2d_get_absolute_time($instance['time_inserted']),
      '#description' => d2d_get_relative_time($instance['time_inserted']),
    );
    $public_key_text = (is_null($instance['public_key']) ? t('N/A') : t('available'));
    if (!$instance['is_me']) {
      $public_key_text = l($public_key_text, 'admin/d2d/instances/' . $id . '/public_key');
    }
    $form['instances']['id' . $id]['public_key'] = array(
      '#type' => 'item',
      '#markup' => $public_key_text,
    );
    $friend_text = '';
    $friend_description = '';
    if (!$instance['is_me']) {
      $friend_text = ($instance['is_friend'] ? t('yes') : t('no'));
      $friend_text = l($friend_text, 'admin/d2d/instances/' . $id . '/friendship');
      if ($instance['friendship_state'] == 1) {
        $friend_description = t('request sent');
      }
      elseif ($instance['friendship_state'] == 2) {
        $friend_description = t('request received');
      }
    }
    $form['instances']['id' . $id]['friend'] = array(
      '#type' => 'item',
      '#markup' => $friend_text,
      '#description' => $friend_description,
    );
    $form['instances']['id' . $id]['operation1'] = array(
      '#type' => 'item',
      '#markup' => l(t('configure'), 'admin/d2d/instances/' . $instance['id'] . '/details'),
    );
    $form['instances']['id' . $id]['operation2'] = array(
      '#type' => 'item',
      // TODO
      //'#markup' => $instance['is_me'] ? '' : t('delete'),
      '#markup' => '',
      //l(t('configure'), 'admin/d2d/instances/' . $instance['id'] . '/details'),
    );
  }
  return $form;
}

function d2d_list_instances() {
  d2d_notify(array('public_key'));
  $instances = d2d_api_instance_get_by_id();
  $rows = array();
  foreach ($instances as $instance) {
    $rows[] = array(
      l($instance['id'], 'admin/d2d/instances/' . $instance['id'] . '/details'),
      check_plain($instance['name']),
      $instance['d2d_id'],
      check_plain($instance['url']),
      d2d_get_absolute_time($instance['time_inserted']),
      is_null($instance['public_key']) ? t('N/A') : t('available'),
      $instance['is_me'] ? '' : ($instance['is_friend'] ? t('yes') : t('no')),
      l(t('edit'), 'admin/d2d/instances/' . $instance['id'] . '/details'),
    );
  }
  $build = array();
//  $build['instances'] = array(
//    '#title' => 'Instances in the database',
//    '#type' => 'fieldset',
//    //'#description' => 'For showing and modifying details about an instance click on its ID.',
//  );
//  $build['instances'][] = array(
//    '#theme' => 'table',
//    '#header' => array('ID', 'Name', 'D2D ID', 'URL', 'Inserted', /*'Last Seen',*/ /*'Description',*/ 'Public Key', 'Friend', 'Operation'),
//    '#rows' => $rows,
//    '#empty' => t('No instance in database.'),
//  );
  $build['instances2'] = array(
    '#title' => 'Instances in the database',
    '#type' => 'fieldset',
  );
  $build['instances2'][] = drupal_get_form('d2d_table_instances');
  $build['add_instance'] = array(
    '#title' => 'Add instance',
    '#type' => 'fieldset',
  );
  $build['add_instance'][] = drupal_get_form('d2d_form_add_instance');
  return $build;
}

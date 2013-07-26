<?php

/**
 * @file
 * Functions directly related to the edit instance tab.
 */

function d2d_form_delete_public_key($form, &$form_state, $id) {
  $form = array();
  $form['id'] = array(
    '#type' => 'hidden',
    '#value' => $id,
  );
  $form['submit'] = array(
    '#type' => 'submit', 
    '#value' => t('Remove public key'),
  );
  return $form;
}

function d2d_form_delete_public_key_submit($form, &$form_state) {
  $id = $form_state['values']['id'];
  $num_updated = d2d_api_public_key_unselect($id);
  if ($num_updated > 0) {
    drupal_set_message(t('Public key has been removed.'));
    $instance = d2d_api_instance_get_by_id($id);
    d2d_incoming_requests_reevaluate_signatures($instance);
  }
  else {
    drupal_set_message(t('Error removing public key'), 'error');
  }
}

function d2d_form_receive_public_key($form, &$form_state, $id) {
  $form = array();
  $form['id'] = array(
    '#type' => 'hidden',
    '#value' => $id,
  );
  $form['submit'] = array(
    '#type' => 'submit', 
    '#value' => t('Receive public key'),
  );
  return $form;
}

function d2d_form_receive_public_key_submit($form, &$form_state) {
  $id = $form_state['values']['id'];
  $instance = d2d_api_instance_get_by_id($id);
  if ($instance === FALSE) {
    drupal_set_message(t('Instance not found.'), 'error');
    return;
  }
  $res = xmlrpc($instance['url'], array('d2d.getPublicKey' => array($instance['d2d_id'])));
  if ($res === FALSE) {
    $xmlrpc_error = xmlrpc_error();
    if ($xmlrpc_error != NULL) {
      drupal_set_message(t('Error retrieving public key.<br/>Error code @code<br/>"@message"', array('@code' => $xmlrpc_error->code, '@message' => $xmlrpc_error->message)), 'error');
    }
    else {
      drupal_set_message(t('Error retrieving public key.'), 'error');
    }
  }
  else {
    drupal_set_message(t('Public key has been received.'));
    // TODO: check length / format of received string
    $public_key_id = d2d_api_public_key_return_id($instance['d2d_id'], $res);
    if ($public_key_id !== FALSE) {
      drupal_set_message(t('Public key with id @id is already available in database.', array('@id' => $public_key_id)), 'error');
    }
    else {
      d2d_api_public_key_set($id, $res);
      drupal_set_message(t('Public key has been inserted into database. Click on \'select\' to associate the newly inserted public key with this instance.'));
    }
  }
}

function d2d_form_edit_instance($form, &$form_state, $id, $instance) {
  $form = array();
  $form['url'] = array(
    '#type' => 'textfield',
    '#title' => t('Address'),
    '#description' => t('Address of the instance, e.g. http:/www.example.com/xmlrpc.php'),
    '#default_value' => $instance['url'],
    '#size' => 40,
    '#maxlength' => D2D_INSTANCE_URL_MAX_LENGTH,
    '#required' => TRUE,
  );
  $form['description'] = array(
    '#type' => 'textarea',
    '#title' => t('Description of the instance'),
    '#description' => t('An optional desciption of this instance, no longer than @max_length characters.', array('@max_length' => D2D_INSTANCE_DESCRIPTION_MAX_LENGTH)),
    '#rows' => 4,
    '#cols' => 60,
    '#default_value' => $instance['description'],
    '#required' => FALSE,
  );
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#description' => t('A short name describing the instance.'),
    '#default_value' => $instance['name'],
    '#size' => D2D_INSTANCE_NAME_MAX_LENGTH,
    '#maxlength' => D2D_INSTANCE_NAME_MAX_LENGTH,
    '#required' => FALSE,
  );
  $form['id'] = array(
    '#type' => 'hidden',
    '#value' => $id,
  );
  $form['submit'] = array(
    '#type' => 'submit', 
    '#value' => 'Save',
  );
  return $form;
}

function d2d_form_edit_instance_validate($form, &$form_state) {
  if (strlen($form_state['values']['description']) > D2D_INSTANCE_DESCRIPTION_MAX_LENGTH) {
    form_set_error('description', t('Provided description must be no longer than @max_length characters', array('@max_length' => D2D_INSTANCE_DESCRIPTION_MAX_LENGTH)));
  }
  if (!d2d_check_url($form_state['values']['url'])) {
    form_set_error('url', t('Address must start with \'http://\' or \'https://\'.'));
  }
}

function d2d_form_edit_instance_submit($form, &$form_state) {
  $id = $form_state['values']['id'];
  $name = $form_state['values']['name'];
  $url = $form_state['values']['url'];
  $description = $form_state['values']['description'];
  $update = array(
    'name' => $name,
    'url' => $url,
    'description' => $description,
  );
  d2d_api_instance_update($id, $update);
  drupal_set_message(t('Details have been saved.'));
}

function d2d_form_delete_or_choose_candidate_public_key($form, &$form_state, $id, $public_key_id) {
  $form = array();
  $form['id'] = array(
    '#type' => 'hidden',
    '#value' => $id,
  );
  $form['public_key_id'] = array(
    '#type' => 'hidden',
    '#value' => $public_key_id,
  );
  $form['submit'] = array(
    '#type' => 'submit', 
    '#value' => 'Remove public key from database',
    '#submit' => array('d2d_form_delete_candidate_public_key_submit'),
  );
  $form['submit2'] = array(
    '#type' => 'submit', 
    '#value' => 'Choose this public key',
    '#submit' => array('d2d_form_choose_candidate_public_key_submit'),
  );
  return $form;
}

function d2d_form_delete_candidate_public_key_submit($form, &$form_state) {
  $public_key_id = $form_state['values']['public_key_id'];
  $num_updated = d2d_api_public_key_delete($public_key_id);
  if ($num_updated > 0) {
    drupal_set_message(t('Removed public key from the database'));
  }
  else {
    drupal_set_message(t('Error removing public key from the database'), 'error');
  }
}

function d2d_form_choose_candidate_public_key_submit($form, &$form_state) {
  $id = $form_state['values']['id'];
  $public_key_id = $form_state['values']['public_key_id'];
// TODO!!
//  d2d_invalidate($id);
  $num_updated = d2d_api_instance_update($id, array('public_key_id' => $public_key_id));
  
  if ($num_updated > 0) {
    drupal_set_message(t('Chosen new public key.'));
  }
  else {
    drupal_set_message(t('Error choosing public key'), 'error');
  }
  $instance = d2d_api_instance_get_by_id($id);
  d2d_incoming_requests_reevaluate_signatures($instance);
}

function d2d_form_groups_by_instance($form, &$form_state, $id) {
  $rows = array();
  $default_value = array();
  $groups = d2d_api_group_membership_get($id);
  foreach ($groups as $group) {
    $group_entry = array(
      '#type' => 'item',
      '#markup' => $group['name'],
      '#description' => $group['description'],
    );
    $row = array($group['id'], drupal_render($group_entry));
    $rows['id' . $group['id']] = $row;
    $default_value['id' . $group['id']] = $group['is_member'] ? 'id' . $group['id'] : 0;
  }
  $form['id'] = array(
    '#type' => 'hidden',
    '#value' => $id,
  );
  $form['groups_table'] = array
  (
    '#type' => 'tableselect',
    '#header' => array('ID', 'Name'),
    '#options' => $rows,
    '#empty' => t('No groups in database.'),
    '#default_value' => $default_value,
  );
  $form['submit'] = array(
    '#type' => 'submit', 
    '#value' => 'Save changes',
  );
  return $form;
}

function d2d_form_groups_by_instance_submit($form, &$form_state) {
  $id = $form_state['values']['id'];
  $ids = _d2d_get_ids($form_state, 'groups_table', 2);
  d2d_api_group_membership_update($id, $ids);
  drupal_set_message(t('The changes have been saved.'));
}

function d2d_form_check_public_keys($form, &$form_state, $id, $d2d_id) {
  $form = array();
  $form['id'] = array(
    '#type' => 'hidden',
    '#value' => $id,
  );
  $form['d2d_id'] = array(
    '#type' => 'hidden',
    '#value' => $d2d_id,
  );
  $form['submit'] = array(
    '#type' => 'submit', 
    '#value' => t('Check friends\' public keys'),
  );
  return $form;
}

function d2d_form_check_public_keys_submit($form, &$form_state) {
  $id = $form_state['values']['id'];
  $d2d_id= $form_state['values']['d2d_id'];
  $friends = d2d_api_friend_get();
  $n_checked = 0;
  foreach ($friends as $friend) {
    if ($id == $friend['id']) {
      continue;
    }
    $error_string = '';
    $res = d2d_call_secure_rpc($friend, 'd2d_get_public_key', array('d2d_id' => $d2d_id), $error_string);
    if ($res === FALSE) {
      drupal_set_message(t('Friend with D2D id @d2d_id (@name, @url) returned @error', array('@d2d_id' => $friend['d2d_id'], '@name' => $friend['name'], '@url' => $friend['url'], '@error' => $error_string)), 'error');
    }
    else {
      if (!$res) {
        drupal_set_message(t('Friend with D2D id @d2d_id (@name, @url) has no public key for this instance in the database.', array('@d2d_id' => $friend['d2d_id'], '@name' => $friend['name'], '@url' => $friend['url'])), 'error');
      }
      elseif (($public_key_id = d2d_api_public_key_return_id($d2d_id, $res)) !== FALSE) {
        drupal_set_message(t('Friend with D2D id @d2d_id (@name, @url) uses public key with id @id for this instance.', array('@d2d_id' => $friend['d2d_id'], '@name' => $friend['name'], '@url' => $friend['url'], '@id' => $public_key_id)));
      }
      else {
        drupal_set_message(t('Friend with D2D id @d2d_id (@name, @url) has a public key in the database that is not listed here.', array('@d2d_id' => $friend['d2d_id'], '@name' => $friend['name'], '@url' => $friend['url'])));
      }
    }
    $n_checked++;
  }
  if ($n_checked == 0) {
    drupal_set_message(t('No friends to compare public key with'));
  }
}

function _d2d_edit_instance_build_header($instance) {
  d2d_notify();
  $build = array();
  $build['main'] = array(
    '#type' => 'fieldset',
    '#title' => t('Instance #') . $instance['id'],
  );
  if ($instance['is_me']) {
    $build['main']['#description'] = t('This is your instance!');
  }
  $rows = array();
  $rows[] = array(t('URL'), check_plain($instance['url']));
  $rows[] = array(t('Name'), check_plain($instance['name']));
  $rows[] = array(t('D2D Identifier'), check_plain($instance['d2d_id']));
  $rows[] = array(t('Inserted'), d2d_get_absolute_time($instance['time_inserted']));
  $rows[] = array(t('Public Key'), is_null($instance['public_key']) ? 'N/A' : $instance['public_key']);
  $build['main']['overview'] = array(
    '#theme' => 'table',
    '#header' => array(t('Type'), t('Value')),
    '#rows' => $rows,
  );
  return $build;
}

function d2d_form_remove_instance_submit($form, &$form_state) {

  $id = $form_state['values']['id'];
  $notify = !empty($form_state['values']['notify']);
  $n_deleted = d2d_api_instance_delete($id, $notify);
  drupal_set_message(t('Successfully removed instance from database.'));
  if ($notify && !$n_deleted) {
    drupal_set_message(t('Notification to instance could not be sent.'), 'warning');
  }
  drupal_goto('admin/d2d');
}

function d2d_form_remove_instance_validate($form, &$form_state) {
  if (!$form_state['values']['remove']) {
    form_set_error('remove', t('Mark the checkbox below to show that you understand the consequences of removing this instance.'));
  }
}

function d2d_form_remove_instance($form, &$form_state, $instance) {
  $form = array();
  $form['id'] = array(
    '#type' => 'hidden',
    '#value' => $instance['id'],
  );
  $form['remove'] = array(
    '#type' => 'checkbox',
    '#default_value' => FALSE,
    '#title' => t('I understand that this will not only remove the details about this instance in the database but also the associated public key, permissions etc.'),
  );
  if ($instance['friendship_state'] != 0) {
    $message = array(
      1 => t('Notify the other instance of the revokation of your friendship request.'),
      2 => t('Notify the other instance of the rejection of its open friendship request.'),
      3 => t('Notify the other instance of the termination of the friendship.'),
    );
    $form['notify'] = array(
      '#type' => 'checkbox',
      '#default_value' => FALSE,
      '#title' => $message[$instance['friendship_state']],
      '#description' => t('Note: The notification is being sent immediately.'),
      '#disabled' => !d2d_is_online(),
    );
  }
  $form['submit'] = array(
    '#type' => 'submit', 
    '#value' => t('Remove instance'),
  );
  return $form;
}

function d2d_edit_instance_details($instance) {
  $build = _d2d_edit_instance_build_header($instance);
  $form_state = array();
  $form_state['build_info']['args'] = array($instance['id'], $instance);
  $build['main']['details'] = array(
    '#type' => 'fieldset',
    '#title' => t('Change details about this instance'),
  );
  $build['main']['details']['edit'] = drupal_build_form('d2d_form_edit_instance', $form_state);
  if (!$instance['is_me']) {
    $build['main']['remove'] = array(
      '#type' => 'fieldset',
      '#title' => t('Remove this instance'),
    );
    $form_state_remove = array();
    $form_state_remove['build_info']['args'] = array($instance);
    $build['main']['remove'][] = drupal_build_form('d2d_form_remove_instance', $form_state_remove);
  }
  return $build;
}

function d2d_select_public_key($instance, $public_key_id) {
  $num_updated = 0;
  $instance_id = $instance['id'];
  $public_key_obj = d2d_api_public_key_get_by_id($public_key_id);
  if ($public_key_obj && $public_key_obj->instance_id == $instance['id'] && $instance['public_key_id'] != $public_key_id) {
    $num_updated = d2d_api_public_key_select($instance_id, $public_key_id);
    $instance = d2d_api_instance_get_by_id($instance_id);
    d2d_incoming_requests_reevaluate_signatures($instance); // TODO: recover on failure?
  }
  if ($num_updated > 0) {
    drupal_set_message(t('Selected public key.'));
  }
  else {
    drupal_set_message(t('Error selecting public key.'), 'error');
  }
  drupal_goto('admin/d2d/instances/' . $instance_id . '/public_key');
}

function d2d_unselect_public_key($instance, $public_key_id) {
  $num_updated = 0;
  $instance_id = $instance['id'];
  if ($instance['public_key_id'] == $public_key_id) {
    $num_updated = d2d_api_public_key_unselect($instance_id);
    $instance = d2d_api_instance_get_by_id($instance_id);
    d2d_incoming_requests_reevaluate_signatures($instance); // TODO: recover on failure?
  }
  if ($num_updated > 0) {
    drupal_set_message(t('Public key has been unselected.'));
  }
  else {
    drupal_set_message(t('Error unselecting public key.'), 'error');
  }
  drupal_goto('admin/d2d/instances/' . $instance_id . '/public_key');
}

function d2d_delete_public_key($instance, $public_key_id) {
  $num_updated = 0;
  if (!$instance['is_me'] && $instance['public_key_id'] != $public_key_id && d2d_check_convert_int($public_key_id)) {
    $num_updated = d2d_api_public_key_delete($public_key_id);
  }
  if ($num_updated > 0) {
    drupal_set_message(t('Removed candidate public key from the database'));
  }
  else {
    drupal_set_message(t('Error removing public key from the database'), 'error');
  }
  drupal_goto('admin/d2d/instances/' . $instance['id'] . '/public_key');
}

function d2d_edit_instance_public_key($instance) {
  $build = _d2d_edit_instance_build_header($instance);
  $build['pk_manage'] = array(
    '#title' => t('Manage public keys'),
    '#type' => 'fieldset',
  );
  if ($instance['is_me']) {
    $build['pk_manage']['#description'] = t('You can change the public key together with the private key of your instance !link.', array('!link' => l(t('here'), 'admin/d2d/settings/keys')));
  }
  else {
    $form_state = array();
    $form_state['build_info']['args'] = array($instance['id']);
    $build['pk_manage'][] = drupal_build_form("d2d_form_receive_public_key", $form_state);
    $result = d2d_api_public_key_get($instance['d2d_id']);
    $n_public_keys = 0;
    $rows = array();
    foreach ($result as $record) {
      if ($record->id == $instance['public_key_id']) {
        $select_link = l(t('unselect'), 'admin/d2d/instances/' . $instance['id'] . '/unselect_public_key/' . $record->id);
        $remove_link = '-';
      }
      else {
        $select_link = l(t('select'), 'admin/d2d/instances/' . $instance['id'] . '/select_public_key/' . $record->id);
        $remove_link = l(t('remove'), 'admin/d2d/instances/' . $instance['id'] . '/delete_public_key/' . $record->id);
      }
      $row = array(
        $record->id,
        $record->public_key,
        $select_link,
        $remove_link,
      );
      $rows[] = $row;
    }
    if (!empty($rows)) {
      $form_state = array();
      $form_state['build_info']['args'] = array($instance['id'], $instance['d2d_id']);
      $build['pk_manage']['pk_check'] = drupal_build_form('d2d_form_check_public_keys', $form_state);
    }
    $build['pk_manage'][] = array(
      '#theme' => 'table',
      '#header' => array(t('ID'), t('Public key'), array('data' => t('Operations'), 'colspan' => 2)),
      '#rows' => $rows,
      '#empty' => t('No candidate public keys available.'),
    );
  }
  return $build;
}

function d2d_form_force_friendship_submit($form, &$form_state) {
  $id = $form_state['values']['id'];
  $instance = d2d_api_instance_get_by_id($id);
  $new_friendship_state = $form_state['values']['new_friendship_state'];
  d2d_api_friendship_update($id, $new_friendship_state, $instance['friendship_state'], d2d_api_own_instance_id_get());
  drupal_set_message(t('Manually changed the friendship state.'));
}

function d2d_form_force_friendship_validate($form, &$form_state) {
  $new_friendship_state = $form_state['values']['new_friendship_state'];
  if (!is_numeric($new_friendship_state)) {
    form_set_error('new_friendship_state', t('No option selected.'));
  }
}

function d2d_form_change_friendship($form, &$form_state, $id, $old_friendship_state, $has_public_key) {
  $form = array();
  $form['id'] = array(
    '#type' => 'hidden',
    '#value' => $id,
  );
  if (empty($form_state['input']['old_friendship_state'])) {
    $form['old_friendship_state'] = array(
      '#type' => 'hidden',
      '#value' => $old_friendship_state,
    );
  }
  switch ($old_friendship_state) {
    case 0: $options = array(1 => t('send friendship request')); break;
    case 1: $options = array(0 => t('revoke friendship offer')); break;
    case 2: $options = array(3 => t('accept friendship'), 4 => t('reject friendship')); break;
    case 3: $options = array(0 => t('remove friendship')); break;
  }
  if ($old_friendship_state != 2) {
    $options[8 + $old_friendship_state] = t('resend current friendship state');
  }
  switch ($old_friendship_state) {
    case 1: $friendship_text = t('friendship request sent but not accepted'); break;
    case 2: $friendship_text = t('friendship request received but not accepted'); break;
    case 3: $friendship_text = t('friendship established'); break;
    default: $friendship_text = t('no friendship'); break;
  }
  $form['action'] = array(
    '#type' => 'radios',
    '#default_value' => min(array_keys($options)),
    '#title' => t('State of friendship:') . ' ' . $friendship_text,
    '#options' => $options,
    '#required' => TRUE,
  );
  $form['send_now'] = array(
    '#type' => 'checkbox',
    '#default_value' => $has_public_key,
    '#title' => t('send (updated) friendship state immediatelly'),
    '#description' => t('If set, the new state will be sent to the other instance immediatelly, otherwise it might take some time until the request / update is propagated to the other instance. Note that this is only possible if a public key is associated with this instance.'),
    '#disabled' => !$has_public_key,
  );
  $form['submit'] = array(
    '#type' => 'submit', 
    '#value' => t('Send'),
  );
  return $form;
}

function d2d_form_change_friendship_submit($form, &$form_state) {
  $id = $form_state['values']['id'];
  $old_friendship_state = $form_state['input']['old_friendship_state'];
  $instance = d2d_api_instance_get_by_id($id);
  if ($instance === FALSE) {
    drupal_set_message(t('Instance does not exist anymore'), 'error');
  }
  elseif ($instance['friendship_state'] !== $old_friendship_state) {
    drupal_set_message(t('Friendship state has been changed. Ignoring request.'), 'warning');
  }
  else {
    $action = $form_state['values']['action'];
    $new_friendship_state = $action % 4;
    if ($new_friendship_state != $old_friendship_state) {
      d2d_api_friendship_update($id, $new_friendship_state, $old_friendship_state, d2d_api_own_instance_id_get());
    }
    d2d_api_friendship_send($id, $form_state['values']['send_now'], 1, TRUE);
  }
}

function d2d_form_force_friendship($form, &$form_state, $id, $old_friendship_state) {
  $form = array();
  $form['id'] = array(
    '#type' => 'hidden',
    '#value' => $id,
  );
  $options = array(
    0 => t('no friendship'),
    1 => t('friendship request sent but not accepted'),
    2 => t('friendship request received but not accepted'),
    3 => t('friendship established'),
  );
  unset($options[$old_friendship_state]);
  $form['new_friendship_state'] = array(
    '#type' => 'radios',
    '#default_value' => min(array_keys($options)),
    '#title' => t('Force friendship state'),
    '#options' => $options,
    '#required' => TRUE,
  );
  $form['submit'] = array(
    '#type' => 'submit', 
    '#value' => t('Force friendship state'),
  );
  return $form;
}

function d2d_edit_instance_friendship($instance) {
  $build = _d2d_edit_instance_build_header($instance);
  $build['friendship'] = array(
    '#title' => t('Friendship'),
    '#type' => 'fieldset',
  );
  // four different states of friendship
  // 0: no friendship
  // 1: friendship request sent but not accepted
  // 2: friendship request received but not accepted
  // 3: friendship established
  if (!$instance['is_me']) {
    $form_state = array();
    $form_state['build_info']['args'] = array($instance['id'], $instance['friendship_state'], !is_null($instance['public_key']));
    $build['friendship']['change_friendship'] = drupal_build_form('d2d_form_change_friendship', $form_state);
    $form_state = array();
    $form_state['build_info']['args'] = array($instance['id'], $instance['friendship_state']);
    $build['friendship']['force_friendship'] = drupal_build_form('d2d_form_force_friendship', $form_state);
  }
  else {
    $build['friendship']['#description'] = t('You cannot establish friendship with your own D2D instance.');
  }
  return $build;
}

function d2d_edit_instance_groups($instance) {
  $build = _d2d_edit_instance_build_header($instance);
  $build['groups'] = array(
    '#title' => t('Groups'),
    '#type' => 'fieldset',
    '#description' => t('Group memberships for this instance.'),
  );
  $form_state = array();
  $form_state['build_info']['args'] = array($instance['id']);
  $build['groups'][] = drupal_build_form('d2d_form_groups_by_instance', $form_state);
  return $build;
}

function d2d_edit_instance_permissions($instance) {
  $build = _d2d_edit_instance_build_header($instance);
  $rows = array();
  if (!$instance['is_me']) {
    $method_table = d2d_api_method_get($instance['id'], TRUE);
    $hks = module_invoke_all('d2d_secure_rpc');
    foreach ($hks as $key => $value) {
      if (array_key_exists($key, $method_table)) {
        $col1 = array(
          '#type' => 'item',  
          '#markup' => $key,
          '#description' => isset($value['description']) ? $value['description'] : '',
        );
        $col2 = '?';
        if (isset($value['arguments'])) {
          $arguments = $value['arguments'];
          if (is_array($arguments)) {
            $col2 = var_export(array_keys($arguments), TRUE);
          }
        }
        $rows[] = array(drupal_render($col1), $col2);
      }
    }
  }
  $build['permissions'] = array(
    '#title' => t('Permissions'),
    '#type' => 'fieldset',
    '#description' => t('These methods can be called by remote by this instance if friendship is valid.'),
  );
  $build['permissions'][] = array(
    '#theme' => 'table',
    '#header' => array(t('Method'), t('Arguments')),
    '#rows' => $rows,
    '#empty' => t('This instance is not allowed to call any method on your instance.'),
  );
  return $build;
}

function d2d_edit_instance_remote_permissions($instance) {
  $build = _d2d_edit_instance_build_header($instance);
  $build['remote_permissions'] = array(
    '#title' => t('Remote Permissions'),
    '#type' => 'fieldset',
    '#description' => t('These methods can be called by remote on this instance.'),
  );
  $rows = array();
  $description = t('no methods may be called');
  if ($instance['is_friend'] && $instance['public_key']) {
    $friend = array('d2d_id' => $instance['d2d_id'], 'url' => $instance['url'], 'public_key' => $instance['public_key']);
    $error_string = '';
    $res = d2d_call_secure_rpc($friend, 'd2d_list_permissions', array(), $error_string);
    if ($res === FALSE) {
      $description = t('error:') . ' ' . $error_string;
    }
    else {
      $exploded_res = d2d_explode($res);
      if ($exploded_res === FALSE) {
        $description = t('erroneous return:') . ' ' . $res;
      }
      else {
        foreach ($exploded_res as $record) {
          $exploded_record = d2d_explode($record);
          if ($exploded_record) {
            if (d2d_check_array($exploded_record, array('method' => 'is_string', 'description' => 'is_string', 'arguments' => 'is_string'))) {
              $col1 = array(
                '#type' => 'item',  
                '#markup' => check_plain($exploded_record['method']),
                '#description' => check_plain($exploded_record['description']),
              );
              $rows[] = array(drupal_render($col1), check_plain($exploded_record['arguments']));
            }
            else {
              // TODO
            }
          }
          else {
            // TODO
          }
        }
      }
    }
  }
  $build['remote_permissions'][] = array(
    '#theme' => 'table',
    '#header' => array(t('Method'), t('Arguments')),
    '#rows' => $rows,
    '#empty' => $description,
  );
  return $build;
}

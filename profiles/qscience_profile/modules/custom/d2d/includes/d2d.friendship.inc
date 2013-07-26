<?php
/**
 * @file
 * Callback functions for establishing friendship.
 */

function d2d_friendship_outgoing_describe($arguments, $time_insert, $time_next_send, $time_invalid, $max_send_trials, $instance, &$persistent_state) {
  $comment = '';
  if (is_null($persistent_state)) {
    $persistent_state = TRUE;
  }
  else {
    $comment = t('(duplicate)');
  }
  $return = '';
  switch ($instance['friendship_state']) {
    case 0: $return = t('delete friendship / undo friendship request'); break;
    case 1: $return = t('accept / ask friendship'); break;
    case 2: $return = t('this message will not be sent'); break;
    default: $return = t('accept / ask for friendship'); break;
  }
  return $return . ' ' . $comment;
}

function d2d_friendship_outgoing_process($arguments, $time_insert, $time_next_send, $time_invalid, $max_send_trials, $instance, &$persistent_state, &$arguments_send) {
  if (is_null($persistent_state)) {
    $persistent_state = TRUE;
  }
  else {
    return NULL; // do not send and delete
  }
  switch ($instance['friendship_state']) {
    case 0: $arguments_send = array('state' => 'FALSE'); return TRUE; // no friendship, send
    case 1: $arguments_send = array('state' => 'TRUE'); return TRUE; // friendship, send
    case 2: return NULL; // no friendship, do not send and delete
    default: $arguments_send = array('state' => 'TRUE'); return TRUE;
  }
}

function d2d_friendship_incoming_process($arguments, $rpc_info, $live, &$return) {
  $return = NULL;
  if ($arguments['state'] !== 'TRUE' && $arguments['state'] !== 'FALSE') {
    return FALSE;
  }
  if (!$rpc_info['signature_valid']) {
    if ($live) {
      if ($rpc_info['instance']) {
        $old_friendship_state = $rpc_info['instance']['friendship_state'];
        d2d_check_convert_int($old_friendship_state);
      }
      else {
        $old_friendship_state = NULL;
      }
      // ignore not validated request if it is already represented in the current friendship state
      if ($arguments['state'] === 'TRUE' && ($old_friendship_state === 2 || $old_friendship_state === 3)) {
        return FALSE;
      }
      if ($arguments['state'] === 'FALSE' && ($old_friendship_state === 0 || is_null($old_friendship_state))) {
        return FALSE;
      }
      
      $description = array('url' => $rpc_info['url'], 'ip' => $rpc_info['ip'], 'reason' => ($rpc_info['instance'] ? 'public_key' : 'instance'));
      if ($arguments['state'] === 'TRUE') {
        $type = ($old_friendship_state === 1 ? 'unchecked_accept' : 'unchecked_request');
      }
      else {
        if ($old_friendship_state === 1) {
          $type = 'unchecked_reject';
        }
        elseif ($old_friendship_state === 2) {
          $type = 'unchecked_revoke';
        }
        else { // $old_friendship_state === 3
          $type = 'unchecked_remove';
        }
      }
      _d2d_insert_friendship_notification($rpc_info['d2d_id'], $type, $description);
      return TRUE;
    }
    else {
      return FALSE;
    }
  }
  $old_friendship_state = $rpc_info['instance']['friendship_state'];
  // 0: no friendship
  // 1: friendship request sent but not accepted
  // 2: friendship request received but not accepted
  // 3: friendship established
  $friendship_map = array(
    'FALSE' => array(0 => 0, 1 => 0, 2 => 0, 3 => 0),
    'TRUE' => array(0 => 2, 1 => 3, 2 => 2, 3 => 3),
  );
  $types = array(
    'FALSE' => array(0 => '', 1 => 'reject', 2 => 'revoke', 3 => 'remove'),
    'TRUE' => array(0 => 'request', 1 => 'accept', 2 => '', 3 => '')
  );
  $new_friendship_state = $friendship_map[$arguments['state']][$old_friendship_state];
  if ($new_friendship_state != $old_friendship_state) {
    d2d_api_friendship_update($rpc_info['instance']['id'], $new_friendship_state, $old_friendship_state, $rpc_info['instance']['id']);
    _d2d_insert_friendship_notification($rpc_info['instance']['d2d_id'], $types[$arguments['state']][$old_friendship_state], array());
  }
  if ($live) {
    if ($new_friendship_state == 3) {
      $return = array('value' => 'auto_accept');
    }
    else {
      $return = array('value' => '');
    }
  }
  return FALSE;
}

function _d2d_insert_friendship_notification($d2d_id, $type, $description) {
  d2d_api_notification_insert('d2d_friendship', $d2d_id, 
    d2d_implode(
      array(
        'type' => $type,
        'description' => d2d_implode($description),
      )
    )
  );
}

function d2d_friendship_outgoing_return($return, $arguments, $arguments_send, $time_insert, $time_next_send, $time_invalid, $max_send_trials, $error_string, &$instance, &$persistent_state) {
  if ($return === FALSE) {
  }
  elseif ($return === NULL) {
  }
  else {
    $old_friendship_state = $instance['friendship_state'];
    if ($old_friendship_state == 1 && $return['value'] == 'auto_accept') {
      $new_friendship_state = 3;
      $instance['friendship_state'] = $new_friendship_state;
      $instance['is_friend'] = TRUE;
      d2d_api_friendship_update($instance['id'], $new_friendship_state, $old_friendship_state, $instance['id']);
      _d2d_insert_friendship_notification($instance['d2d_id'], 'accept', array());
    }
  }
}

function d2d_get_friendship_simple($arguments, $attributes) {
  $type = $arguments['type'];
  $instance = d2d_api_instance_get($attributes['d2d_id']);
  if ($instance) {
    $instance_name = check_plain($instance['name']);
    $instance_str = l(check_plain($instance_name), 'admin/d2d/instances/' . $instance['id'] . '/details');
  }
  else {
    $instance_str = '<i>' . $attributes['d2d_id'] . '</i>';
  }
  if ($type === 'accept') {
    return $instance_str . ' ' . t('has accepted your friendship request.');
  }
  if ($type === 'reject') {
    return $instance_str . ' ' . t('has rejected your friendship request.');
  }
  if ($type === 'request') {
    $return_str = $instance_str . ' ' . t('has sent you a friendship request.');
    if ($instance) {
      $return_str .= ' ' . t('Click !link to reply.', array('!link' => l(t('here'), 'admin/d2d/instances/' . $instance['id'] . '/friendship')));
    }
    return $return_str;
  }
  if ($type === 'revoke') {
    return $instance_str . ' ' . t('has revoked the friendship request.');
  }
  if ($type === 'remove') {
    return $instance_str . ' ' . t('has terminated the friendship.');
  }
  if ($type === 'unchecked_accept') {
    return $instance_str . ' ' . t('has accepted your friendship request (signature not valid!).');
  }
  if ($type === 'unchecked_reject') {
    return $instance_str . ' ' . t('has rejected your friendship request (signature not valid!).');
  }
  if ($type === 'unchecked_request') {
    return $instance_str . ' ' . t('has sent you a friendship request (signature not valid!).');
  }
  if ($type === 'unchecked_remove') {
    return $instance_str . ' ' . t('has terminated friendship (signature not valid!).');
  }
  if ($type === 'unchecked_revoke') {
    return $instance_str . ' ' . t('has revoked the friendship request (signature not valid!).');
  }
  return 'unknown request';
}

function d2d_get_friendship_detailed($arguments, $attributes) {
  $type = $arguments['type'];
  $instance = d2d_api_instance_get($attributes['d2d_id']);
  $instance_str = check_plain($instance['name']);
  $build['notifications'] = array(
    '#title' => t('Notification') . ' #' . $attributes['id'],
    '#type' => 'fieldset',
    '#description' => d2d_get_friendship_simple($arguments, $attributes),
  );
  //if ($type === 'unchecked_request' || $type === 'unchecked_remove') {
  if (!strncmp($type, 'unchecked_', strlen('unchecked_'))) { // $type starts with 'unchecked_'
    $description = d2d_explode($arguments['description']);
    $build['notifications']['#description'] .= '<br/>';
    if ($description['reason'] === 'public_key') {
      $build['notifications']['#description'] .= t('The reason why the signature could not be checked is that you don\'t have a public key associated with that instace.');
      $instance = d2d_api_instance_get($attributes['d2d_id']);
      if ($instance) {
        $build['notifications']['#description'] .= ' ' . t('Click !link to associate a public key with that instance.', array('!link' => l(t('here'), 'admin/d2d/instances/' . $instance['id'] . '/public_key')));
      }
    }
    else {
      $build['notifications']['#description'] .= t('The reason why the signature could not be checked is that this instance is not in your database and hence no public key is available. Click !link to proceed with adding this instance.', array('!link' => l(t('here'), 'admin/d2d/prefill')));
      $_SESSION['d2d_add_instance'] = array('url' => $description['url'], 'd2d_id' => $attributes['d2d_id'], 'prefill' => FALSE);
    }
  }
  return $build;
}

function d2d_add_instance_prefill() {
  if (isset($_SESSION['d2d_add_instance'])) {
    $_SESSION['d2d_add_instance']['prefill'] = TRUE;
  }
  drupal_goto('admin/d2d');
}

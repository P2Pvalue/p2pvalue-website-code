<?php
/**
 * @file
 * Miscellaneous helper functions.
 */

/**
 * Return the version number of the module as a string.
 */
function d2d_get_version() {
  return D2D_VERSION_STRING;
}

/**
 * Returns the options for an XML-RPC request, i.e. the third argument given
 * to xmlrpc(...)
 */
function d2d_xmlrpc_options() {
  return array(
    'timeout' => variable_get('d2d_timeout', D2D_TIMEOUT_DEFAULT),
  );
}

/**
 * Changes the online state of this D2D-instance.
 *
 * @param bool $online the desired new online state.
 * @return bool the new online state.
 */
function d2d_change_online_state($online) {
  return $online ? d2d_go_online() : d2d_go_offline();
}

/**
 * Changes the state of this D2D-instance to offline.
 *
 * @return bool the new online state.
 */
function d2d_go_offline() {
  variable_set('d2d_online', FALSE);
  return FALSE;
}

/**
 * Changes the state of this D2D-instance to online.
 *
 * @return bool the new online state.
 */
function d2d_go_online() {
  $public_key = d2d_api_own_public_key_get();
  $private_key = d2d_api_own_private_key_get();
  if (!d2d_check_key_pair($public_key, $private_key)) {
    variable_set('d2d_online', FALSE);
    return FALSE;
  }
  else {
    variable_set('d2d_online', TRUE);
    return TRUE;
  }
}
/**
 * Returns whether this D2D-instance is online.
 */
function d2d_is_online() {
  return variable_get('d2d_online', FALSE);
}

function d2d_get_time() {
  return time() + variable_get('d2d_utc_offset', D2D_UTC_OFFSET_DEFAULT);
}

// TODO: use t(.) for units
function d2d_format_unit($v, $unit, $skip=FALSE) {
  if (!$v && $skip) {
    return '';
  }
  else {
    return strval($v) . ' ' . $unit . ($v != 1 ? 's' : '');
  }
}

// TODO: localization
function d2d_get_absolute_time($t) {
  return date('F j, Y, g:i a', $t);
}

function d2d_check_url($url) {
  if (!is_string($url)) {
    return FALSE;
  }
  if (!preg_match('/^https?:\/\/.+$/', $url)) {
    return FALSE;
  }
  return strlen($url) <= D2D_INSTANCE_URL_MAX_LENGTH;
}

function d2d_get_relative_time($t) {
  $diff = $t - d2d_get_time();
  $prefix = '';
  $suffix = '';
  if ($diff > 0) {
    $absdiff = $diff;
    $prefix = 'in ';
  }
  else {
    $absdiff = -$diff;
    $suffix = ' ago';
  }
  $diffs = array(
    gmdate('Y', $absdiff) - 1970,
    gmdate('n', $absdiff) - 1,
    gmdate('j', $absdiff) - 1,
    gmdate('G', $absdiff),
    intval(gmdate('i', $absdiff)),
    intval(gmdate('s', $absdiff)),
    0,
  );
  $relative = '';
  $units = array('year', 'month', 'day', 'hour', 'minute', 'second', '');
  for ($i = 0; $i < 6; $i++) {
    if ($i == 5 || $diffs[$i] > 0) {
      $relative = d2d_format_unit($diffs[$i], $units[$i]);
      $tmp = d2d_format_unit($diffs[$i + 1], $units[$i + 1], TRUE);
      if ($tmp) {
        $relative = "$relative, $tmp";
      }
      break;
    }
  }
  return $prefix . $relative . $suffix;
}

function d2d_bool_to_str($b) {
  if (is_null($b)) {
    return t('may be');
  }
  else {
    return $b ? t('true') : t('false');
  }
}

/**
 * Return string stating friendship
 */
function d2d_get_friendship_string($signer, $friend, $until) {
  return d2d_implode(
    array(
      'type' => 'friendship',
      'signer' => $signer,
      'friend' => $friend,
      'until' => $until,
    )
  );
}

function d2d_get_request_type($i) {
  switch ($i) {
    case 0:
      return 'request';
    case 1:
      return 'accept';
    case 2:
      return 'renew';
    default:
      return 'N/A';
  }
}

/**
 * Adds convenient fields to an instance array
 * 
 * Modifies the instance array
 * 
 * @param array $instance
 */
function _d2d_decorate_instance_array(&$instance) {
  if (empty($instance)) return;
  $instance['is_friend'] = $instance['friendship_state'] == 3;
  $instance['is_me'] = d2d_api_own_d2d_id_get() == $instance['d2d_id'];
}

/**
 * Performs transformations on the list of instances as loaded from the database
 * 
 * @param array $result_set
 * @param boolean $d2d_id If TRUE, sets the associative keys of each instance to their d2d_id. 
 *   Defaults, FALSE (keys are set to their instance id)
 * @return array The transformed result set
 */
function _d2d_process_result_set_instance($result_set, $d2d_id = FALSE) {
  $instances = array();
  foreach ($result_set as $record) {
    $instances[$d2d_id ? $record->d2d_id : $record->id] = array(
        'id' => $record->id,
        'd2d_id' => $record->d2d_id,
        'name' => $record->name,
        'url' => $record->url,
        'description' => $record->description,
        'time_inserted' => $record->time_inserted,
        'last_alive' => $record->last_alive,
        'public_key' => $record->public_key,
        'friendship_state' => $record->friendship_state,
    );
    _d2d_decorate_instance_array($instances[$d2d_id ? $record->d2d_id : $record->id]);
  }

  return $instances;
}

/**
 * Include the instance with the provided ID in all groups that are marked as default.
 */
function d2d_set_default_group_memberships($id) {
  db_delete('d2d_group_memberships')
    ->condition('instance_id', $id)->execute();
  $groups = d2d_api_group_get(NULL, TRUE);
  if (!empty($groups)) {
    $query = db_insert('d2d_group_memberships')->fields(array('group_id', 'instance_id'));
    foreach ($groups as $group) {
      $query->values(array('group_id' => $group['id'], 'instance_id' => $id));
    }
    $query->execute();
  }
}
function d2d_verify_timestamp($current_time, $timestamp) {
  return $timestamp >= $current_time - variable_get('d2d_delta_0', D2D_DELTA_0_DEFAULT) && $timestamp <= $current_time + variable_get('d2d_delta_1', D2D_DELTA_1_DEFAULT);
}
function d2d_verify_nonce($timestamp, $nonce) {
  try {
    db_insert('d2d_nonces')->fields(array(
        'nonce' => $nonce,
        'timestamp' => $timestamp,
    ))->execute();
  }
  catch (Exception $e) {
    return FALSE;
  }
  return TRUE;
}

function d2d_convert_d2d_id_to_id($d2d_id = NULL) {
  if (empty($d2d_id)) return FALSE;
  $instance = d2d_api_instance_get($d2d_id);
  return $instance !== FALSE ? $instance['id'] : FALSE;
}

function d2d_convert_id_to_d2d_id($id = NULL) {
  if (empty($id)) return FALSE;
  $instance = d2d_api_instance_get_by_id($id);
  return $instance !== FALSE ? $instance['d2d_id'] : FALSE;
}

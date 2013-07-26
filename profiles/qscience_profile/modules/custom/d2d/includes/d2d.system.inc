<?php
/**
 * @file
 * Functions and forms for displaying outgoing requests.
 */

/**
 */
function d2d_form_send_requests() {
  $form = array();
  $form['submit'] = array(
    '#type' => 'submit', 
    '#value' => 'Send all open requests now',
  );
  return $form;
}

/**
 */
function d2d_form_send_requests_submit($form, &$form_state) {
  d2d_remove_outdated_outgoing_requests();
  d2d_send_outgoing_requests(NULL, TRUE);
}

/**
 * Builds a table with outgoing requests of type specified in $types.
 *
 * If $types is an array, the type has to be contained in the array, otherwise
 * the table contains notifications of any type.
 */
function d2d_built_outgoing_requests_table($types) {
  $instances = d2d_api_instance_get_by_id();
  $hooks = module_invoke_all('d2d_handle_outgoing_request');
  $rows = array();
  $result = d2d_api_outgoing_request_get($types);
  $persistent_state = array();
  foreach ($result as $record) {
    if (!array_key_exists($record->receiver_id, $instances)) {
      continue;
    }
    $description = t('N/A');
    if (array_key_exists($record->type, $hooks)) {
      $hook = $hooks[$record->type];
      $argument_checks = (array_key_exists('arguments_save', $hook) ? $hook['arguments_save'] : array());
      $arguments = d2d_explode($record->data);
      if (d2d_check_array($arguments, $argument_checks) !== FALSE) {
        d2d_friendship_outgoing_describe($arguments, $record->time_insert, $record->time_next_send, $record->time_invalid, $record->max_send_trials, $instances[$record->receiver_id], $persistent_state);
        if (!array_key_exists($record->type, $persistent_state)) {
          $persistent_state[$record->type] = array();
        }
        if (!array_key_exists($record->receiver_id, $persistent_state[$record->type])) {
          $persistent_state[$record->type][$record->receiver_id] = NULL;
        }
        $description = call_user_func_array($hook['callback_describe'], array($arguments, $record->time_insert, $record->time_next_send, $record->time_invalid, $record->max_send_trials, $instances[$record->receiver_id], &$persistent_state[$record->type][$record->receiver_id]));
      }
    }
    $receiver = l($instances[$record->receiver_id]['d2d_id'], 'admin/d2d/instances/' . $record->receiver_id . '/details');
    $row = array(
      $record->id,
      $receiver,
      $record->type,
      $description,
      d2d_get_relative_time($record->time_insert),
      d2d_get_relative_time($record->time_next_send),
      d2d_get_relative_time($record->time_invalid),
      $record->max_send_trials,
      is_null($instances[$record->receiver_id]['public_key']) ? t('no') : t('yes'),
      l(t('delete'), 'admin/d2d/system/out_requests/' . $record->id . '/delete'),
      l(t('send now'), 'admin/d2d/system/out_requests/' . $record->id . '/send'),
    );
    $rows['id' . $record->id] = $row;
  }
  $table = array(
    '#theme' => 'table',
    '#header' => array('ID', 'Receiver', 'Type', 'Description', 'Time Inserted', 'Time Next Send', 'Time Invalid', '#Resend', 'PK', 'Op', 'Op'),
    '#rows' => $rows,
    '#empty' => t('No outgoing requests found.'),
  );
  return $table;
}

/**
 */
function d2d_show_system_tab() {
  d2d_notify();
  $build = array();
  $build['system'] = array(
    '#title' => t('System and advanced functionality'),
    '#type' => 'fieldset',
  );
  $build['system'][] = drupal_get_form('d2d_form_send_requests');
  $build['system'][] = d2d_built_outgoing_requests_table(NULL);
  return $build;
}

/**
 */
function d2d_outgoing_requests_delete_submit($form, &$form_state, $id) {
  if (d2d_check_convert_int($id)) {
    d2d_api_outgoing_request_delete($id);
  }
  drupal_goto('admin/d2d/system');
}

/**
 */
function d2d_outgoing_requests_send_submit($form, &$form_state, $id) {
  if (d2d_check_convert_int($id)) {
    d2d_send_outgoing_requests(array($id), TRUE);
  }
  drupal_goto('admin/d2d/system');
}

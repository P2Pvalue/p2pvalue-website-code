<?php

/**
 * @file
 * Functions directly related to the manage groups tab.
 */
function d2d_form_list_groups() {
  $rows = array();
  $groups = d2d_api_group_get();
  foreach ($groups as $group) {
    $row = array(
      l($group['id'], 'admin/d2d/g_and_p/groups/edit/' . $group['id']),
      $group['name'],
      $group['description'],
      d2d_bool_to_str($group['auto_member']),
    );
    $rows['id' . $group['id']] = $row;
  }
  $form['groups_table'] = array(
    '#type' => 'tableselect',
    '#header' => array('ID', 'Name', 'Description', 'Default'),
    '#options' => $rows,
    '#empty' => t('No groups in database.'),
  );
  $form['submit'] = array(
    '#type' => 'submit', 
    '#value' => 'Delete',
  );
  return $form;
}
function _d2d_get_ids(&$form_state, $table, $prefix_length) {
  $ids = array();
  foreach ($form_state['values'][$table] as $key => $value) {
    if ($value && strlen($value) > $prefix_length) {
      $ids[] = substr($key, $prefix_length);
    }
  }
  return $ids;
}
function d2d_form_list_groups_validate($form, &$form_state) {
  $ids = _d2d_get_ids($form_state, 'groups_table', 2);
  if (empty($ids)) {
    form_set_error('groups_table', t('No group selected for deletion.'));
  }
}

function d2d_form_list_groups_submit($form, &$form_state) {
  $group_ids = _d2d_get_ids($form_state, 'groups_table', 2);
  d2d_api_group_delete($group_ids);
  d2d_api_permission_delete($group_ids);
  d2d_api_group_membership_delete($group_ids);
}

function theme_d2d_list_permissions($variables) {
  $form = $variables['form'];
  foreach (element_children($form['permission']) as $key) {
    $row = array();
    $row[] = array(
      'data' => drupal_render($form['permission'][$key]),
      'class' => array('permission'),
    );
    if (array_key_exists('checkboxes', $form)) {
      foreach (element_children($form['checkboxes']) as $gid) {
        $row[] = array('data' => drupal_render($form['checkboxes'][$gid][$key]), 'class' => array('checkbox'));
      }
    }
    $rows[] = $row;
  }
  $header[] = (t('Method'));
  if (array_key_exists('group', $form)) {
    foreach (element_children($form['group']) as $gid) {
      $header[] = array('data' => drupal_render($form['group'][$gid]), 'class' => array('checkbox'));
    }
  }
  $output = theme('table', array('header' => $header, 'rows' => $rows));
  $output .= drupal_render_children($form);
  return $output;
}

function d2d_list_permissions($form, &$form_state) {
  $hks = module_invoke_all('d2d_secure_rpc');
  if (empty($hks)) {
    drupal_set_message(t('No hooks found.'), 'warning');
    return array();
  }
  $groups = d2d_api_group_get();
  $permission_table = d2d_api_permission_get(TRUE);
  $form['groups'] = array(
    '#type' => 'value',
    '#value' => $groups,
  );
  $options = array();
  $status = array();
  foreach ($hks as $key => $value) {
    if (strlen($key) > D2D_METHOD_NAME_MAX_LENGTH) {
      drupal_set_message(t('Ignoring hook for method @method (corresponding key may be no longer than @length characters).', array('@method' => $key, '@length' => D2D_METHOD_NAME_MAX_LENGTH)), 'warning');
      continue;
    }
    $options[$key] = '';
    $form['permission'][$key] = array(
      '#type' => 'item',
      '#markup' => $key,
      '#description' => isset($value['description']) ? $value['description'] : '',
    );
    foreach ($groups as $group_id => $group) {
      if (isset($permission_table[$group_id][$key])) {
        $status[$group_id][] = $key;
      }
    }
  }
  foreach ($groups as $group_id => $group) {
    $form['checkboxes'][$group_id] = array(
      '#type' => 'checkboxes',
      '#options' => $options,
      '#default_value' => isset($status[$group_id]) ? $status[$group_id] : array(),
    );
    $form['group'][$group_id] = array('#markup' => $group['name'], '#tree' => TRUE);
  }
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Save permissions'));
  return $form;
}
function d2d_list_permissions_submit($form, &$form_state) {
  d2d_api_permission_clear();
  $methods = array();
  $permissions = array();
  foreach ($form_state['values']['groups'] as $gid => $group) {
    $permissions_group = array();
    foreach ($form_state['values'][$gid] as $method => $flag) {
      if ($flag) {
        $permissions_group[] = $method;
        $methods[$method] = 1;
        $permissions[] = array('group_id' => $gid, 'method' => $method);
      }
    }
  }
  d2d_api_permission_add($permissions);
  drupal_set_message(t('The changes have been saved.'));
}
function d2d_form_add_group() {
  $form = array();
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#description' => t('Name of the group'),
    '#default_value' => '',
    '#size' => 40,
    '#maxlength' => D2D_GROUP_NAME_MAX_LENGTH,
    '#required' => TRUE,
  );
  $form['default'] = array(
    '#type' => 'checkbox',
    '#default_value' => FALSE,
    '#title' => t('Automatically add new instances to this group.'),
    '#description' => t('This has no effect on existing instances.'),
  );
  $form['description'] = array(
    '#type' => 'textarea',
    '#title' => t('Description of the group'),
    '#description' => t('An optional desciption of the newly added group, no longer than 1024 characters.'),
    '#rows' => 4,
    '#cols' => 60,
    '#default_value' => 'N/A',
    '#required' => FALSE,
  );
  $form['submit'] = array(
    '#type' => 'submit', 
    '#value' => 'Create group',
  );
  return $form;
}
function d2d_form_add_group_validate($form, &$form_state) {
  $group = d2d_api_group_get_by_name($form_state['values']['name']);
  if (!is_null($group)) {
    form_set_error('name', t('Provided name already exists in database'));
  }
  if (strlen($form_state['values']['description']) > D2D_GROUP_DESCRIPTION_MAX_LENGTH) {
    form_set_error('description', t('Provided description must be no longer than @max_length characters'), array('@max_length' => D2D_GROUP_DESCRIPTION_MAX_LENGTH));
  }
}
function d2d_form_add_group_submit($form, &$form_state) {
  $name = $form_state['values']['name'];
  $description = $form_state['values']['description'];
  $auto_member = $form_state['values']['default'];
  $id = d2d_api_group_add($name, $description, $auto_member);
  if (is_null($id)) {
    drupal_set_message(t('A group with that name already exists in the databaset.'), 'error');
  }
  else {
    drupal_set_message(t('Successfully created a new group.'));
  }
}
function d2d_manage_groups() {
  d2d_notify();
  $build = array();
  $build['groups'] = array(
    '#title' => t('Groups in the database'),
    '#type' => 'fieldset',
  );
  $build['groups'][] = drupal_get_form('d2d_form_list_groups');

  $build['add_group'] = array(
    '#title' => t('Create group'),
    '#type' => 'fieldset',
  );
  $form_state = array();
  $build['add_group'][] = drupal_get_form('d2d_form_add_group');
  return $build;
}
function d2d_manage_permissions() {
  d2d_notify();
  $build = array();
  $build['group_rights'] = array(
    '#title' => t('Permissions'),
    '#type' => 'fieldset',
  );
  $build['group_rights'][] = drupal_get_form('d2d_list_permissions');
  return $build;
}

<?php
/*
 * TODO: Use rolename instead of rid in the case of permissions
 * TODO: Add more descriptive messages after merging with the new core branch (custom function cases)
 * 
 * @file
 * Patterns component for users.
 */

/**
 * Implements hook_patterns() for the user module.
 */
function user_patterns($data) {

  $files = array('modules/user/user.admin.inc', 'modules/user/user.pages.inc');

  $actions['role'] = array(
    PATTERNS_INFO => t('Create/Modify/Delete Roles'),
    PATTERNS_CREATE => array('user_admin_roles'),
    PATTERNS_MODIFY => array('user_admin_role'),
    PATTERNS_DELETE => array('user_admin_role_delete_confirm'),
    PATTERNS_FILES => $files,
    PATTERNS_EXPORT => array(PATTERNS_EXPORT_ALL => 'user_patterns_export_all_roles', ),
  );

  $actions['permission'] = array(
    PATTERNS_INFO => t('Create/Modify/Delete Permissions'),
    PATTERNS_MODIFY => array('set_user_permissions'), // Run through custom function. It was previously run through user_admin_permissions.
    PATTERNS_FILES => $files,
    PATTERNS_EXPORT => array(PATTERNS_EXPORT_ALL => 'user_patterns_export_all_permissions', ),
  );
  
  $actions['user'] = array(
      PATTERNS_INFO => t('Create/Modify/Delete Users'),
      PATTERNS_CREATE => array('user_register_form'),
      PATTERNS_MODIFY => array('set_user_values'), // Run through custom function. It was previously run through user_profile_form
      PATTERNS_DELETE => array('user_cancel_confirm_form'),
      PATTERNS_FILES => $files,
      PATTERNS_EXPORT => array(PATTERNS_EXPORT_ALL => 'user_patterns_export_all_users', ),
  );
  
  $actions['role_users'] = array(
      PATTERNS_INFO => t('Add/Remove roles to users'),
      PATTERNS_CREATE => array('set_role_users'), // Run through custom function.
      PATTERNS_DELETE => array('unset_role_users'), // Run through custom function.
      PATTERNS_FILES => $files,
      PATTERNS_EXPORT => array(PATTERNS_EXPORT_ALL => 'user_patterns_export_all_role_users', ),
  );
  return $actions;

}

/**
 *
 * Returns a set of PATTERNS_CREATE or PATTERNS_MODIFY actions 
 * with the whole set of roles currently stored in the system.
 *
 * @param array $args  Set of arguments received from the form.
 * @param string $result  Current set of actions for the whole process.
 * 
 * @return array $result  Set of actions after performing the changes.
 */
function user_patterns_export_all_roles($args = NULL, &$result = NULL) {
  $roles = user_roles(TRUE);
  $result = array();
  
  //Got through all the roles and prepare set of actions according to the type of export process
  switch ($args['type']) {
    case PATTERNS_CREATE :
      foreach ($roles as $rid => $role) {
        //Create operations just require the name
        $data = array('tag' => 'role');
        $data['name'] = $role;
        $action = array(PATTERNS_CREATE => $data);
        array_push($result, $action);
      }      
      break;
    case PATTERNS_MODIFY :
      foreach ($roles as $rid => $role) {
        //Modify operations require the name and the rid
        $data = array('tag' => 'role');
        $data['name'] = $role;
        $data['rid'] = $rid;
        $action = array(PATTERNS_MODIFY => $data);
        array_push($result, $action);
      }
      break;
  }
    
  return $result;
}

/**
 *
 * Returns a set of PATTERNS_MODIFY actions ONLY with the whole 
 * set of roles currently stored in the system.
 *
 * @param array $args  Set of arguments received from the form.
 * @param string $result  Current set of actions for the whole process.
 * 
 * @return array $result  Set of actions after performing the changes.
 */
function user_patterns_export_all_permissions($args = NULL, &$result = NULL) {

  $roles = user_roles();
  $permissions = user_role_permissions($roles);
  $result = array();

  foreach ($permissions as $rid => $perms) {
    $permissions = array_keys($perms);

    $action = array(
      PATTERNS_MODIFY => array(
        'tag' => 'permission',
        'role' => $roles[$rid],
        'value' => $permissions,
      )
    );
    array_push($result, $action);
  }

  return $result;
}


/**
 *
 * Returns a set of PATTERNS_CREATE or PATTERNS_MODIFY actions
 * with the whole set of users currently stored in the system.
 *
 * @param array $args  Set of arguments received from the form.
 * @param string $result  Current set of actions for the whole process.
 *
 * @return array $result  Set of actions
 */
function user_patterns_export_all_users($args = NULL, &$result = NULL) {
  //Fetch all user objects except the anonymous
  $users = _user_patterns_get_all_users();
  $result = array();

  switch ($args['type']) {
    case PATTERNS_CREATE :
      foreach ($users as $user) {
        //Create operation generate a new random password
        $data = array('tag' => 'user');
        $data['name'] = $user->name;
        $data['mail'] = $user->mail;
        $data['password'] = user_password();
        $data['timezone'] = $user->timezone;
        $data['status'] = $user->status;
        $action = array(PATTERNS_CREATE => $data);
        array_push($result, $action);
      }
      break;
    case PATTERNS_MODIFY :
      foreach ($users as $uid => $user) {
        //Modify operations override all the available fields but password.
        $data = array('tag' => 'user');
        $data['uid'] = $uid;
        $data['name'] = $user->name;
        $data['mail'] = $user->mail;
        $data['timezone'] = $user->timezone;
        $data['status'] = $user->status;
        $action = array(PATTERNS_MODIFY => $data);
        array_push($result, $action);
      }
      break;
  }

  return $result;
}


/**
 *
 * Returns a set of PATTERNS_CREATE actionswith the whole set of 
 * relationsps between users and roles currently stored in the system.
 *
 * @param array $args  Set of arguments received from the form.
 * @param string $result  Current set of actions for the whole process.
 *
 * @return array $result  Set of actions after performing the changes.
 */
function user_patterns_export_all_role_users($args = NULL, &$result = NULL) {
  $roles = user_roles(TRUE);
  $result = array();
  
  //A PATTERNS_CREATE action is generated for each role
  foreach ($roles as $rid => $role) {
    $data = array('tag' => 'role_users');
    $data['role'] = $role;
    $data['users'] = _user_patterns_users_with_role($role);
    $action = array(PATTERNS_CREATE => $data);
    array_push($result, $action);
  }

  return $result;
}


// returns arguments needed for the forms when processing automatically
// $loop is set if we want to extract all values
function user_patterns_get_arguments($action, $tag, $form_id, &$loop = FALSE) {
  $return = array();
  $loop = TRUE;
  if ($form_id == 'user_profile_form') {
    $query = db_select('users', 'u');
    $query->condition('u.uid', 0, '<>');
    $query = $query->extend('PagerDefault')->extend('TableSort');
    $query->fields('u', array('uid'));
    $result = $query->execute()->fetchAll();
    foreach ($result as $account) {
      $user = user_load($account->uid);
      array_push($return, array($user));
    }
  }
  elseif ($form_id == 'user_admin_role') {
    $roles = user_roles();
    foreach ($roles as $rid => $name) {
      if (!($rid == DRUPAL_ANONYMOUS_RID || $rid == DRUPAL_AUTHENTICATED_RID)) {
        array_push($return, array((object) array('rid' => $rid, 'name' => $name, 'weight' => 0)));
      }
    }
  }
  elseif ($form_id == 'user_admin_permissions') {
    $roles = user_roles();
    foreach ($roles as $rid => $name) {
      if (!($rid == DRUPAL_ANONYMOUS_RID || $rid == DRUPAL_AUTHENTICATED_RID)) {
        array_push($return, array($rid));
      }
    }
  }
  return $return;
}

/**
 * Implements hook_patterns_prepare() for the user module.
 */
function user_patterns_prepare($action, $tag, &$data) {
  $status = PATTERNS_SUCCESS;
  $msg = t('Execution successful');
  if ($tag == 'user') {
    if (!isset($data['name'])) {
      if (!isset($data['uid'])) {
        $status = PATTERNS_ERR;
        $msg = t('Cannot create/modify user. Missing name and uid.');
      }
      // Retrieve the username name from the db
      $data['name'] = db_select('users', 'u')
                        ->fields('u', array('name'))
                        ->condition('u.uid', $data['uid'])
                        ->execute()
                        ->fetchField();
    }
    else {
      if (!isset($data['uid'])) {
        $data['uid'] = db_select('users', 'u')
                        ->fields('u', array('uid'))
                        ->condition('u.name', $data['name'])
                        ->execute()
                        ->fetchField();
      }
    }

    // @TODO: Check if we are overriding the password
    if (isset($data['password'])) {
      $data['pass'] = array(
        'pass1' => $data['password'],
        'pass2' => $data['password'],
      );
    }else{
      //Load the fake att but with empty values, so it can be executed with no changes
      $data['pass'] = '';
    }

  }
  elseif ($tag == 'role') {
    /* if (isset($data['value'])) {
      $data['name'] = $data['value'];
      unset($data['value']);
    } */
  }

  return patterns_results($status, $msg);
}

/**
 * 
 * @param string $action Type of action being executed
 * @param string $tag Type of tag to be validated
 * @param array $data Associative array containing the data action processed from the pattern
 * 
 * @return mixed through patterns_results($status, $msg, $result) function. Status of the operation,
 * error messages and semantic warnings through $result
 */
function user_patterns_validate($action, $tag, &$data) {
  $result = array();
  $status = PATTERNS_SUCCESS;
  $msg = '';
  
/*
 * Syntactic validation:
 * - Syntactic warnings: They refer to wrong grammar statements that will not 
 * provoke any execution error. Ex.: non-interpretable attributes.
 * - Syntactic errors: They refer to wrong grammar statements that will provoke
 * an execution error. Ex.: misspelling in required attribute.
 * 
 */
  switch ($tag) {
    case 'role':
      switch ($action) {
        case PATTERNS_CREATE:
          //Check mandatory fields, in this case is only one.
          $mandatory_attributes = array('name');
          if (!_patterns_has_all_mandatory_attributes($data, $mandatory_attributes, $msg)) {
            return patterns_results(PATTERNS_ERR, $msg);
          }
          //In this case there are not optional attributes, so we can also use this set as interpretable attributes
          if(_patterns_has_uninterpretable_attributes($data, $mandatory_attributes, $msg)) {
            $status = PATTERNS_WARN;
          }
          break;
        case PATTERNS_MODIFY:
          //Check mandatory fields
          $mandatory_attributes = array('name', 'rid');
          if (!_patterns_has_all_mandatory_attributes($data, $mandatory_attributes, $msg)) {
            return patterns_results(PATTERNS_ERR, $msg);
          }
          //In this case there are not optional attributes, so we can also use this set as interpretable attributes
          if(_patterns_has_uninterpretable_attributes($data, $mandatory_attributes, $msg)) {
            $status = PATTERNS_WARN;
          }
          break;
        case PATTERNS_DELETE:
          //Check mandatory fields, in this case is only one.
          $mandatory_attributes = array('rid');
          if (!_patterns_has_all_mandatory_attributes($data, $mandatory_attributes, $msg)) {
            return patterns_results(PATTERNS_ERR, $msg);
          }
          //In this case there are not optional attributes, so we can also use this set as interpretable attributes
          if(_patterns_has_uninterpretable_attributes($data, $mandatory_attributes, $msg)) {
            $status = PATTERNS_WARN;
          }
          break;
      }
      break;
    case 'permission':
      //Create and delete actions for the permission case are considered as syntactical errors
      if ($action == PATTERNS_CREATE || $action == PATTERNS_DELETE) {
        $msg = t('Action %action is uncompatible for tag %tag.', array('%action' =>$action, '%tag' =>$tag,));
        return patterns_results(PATTERNS_ERR, $msg);
      }else if($action == PATTERNS_MODIFY) {
        $mandatory_attributes = array('role', 'value');
        if (!_patterns_has_all_mandatory_attributes($data, $mandatory_attributes, $msg)) {
          return patterns_results(PATTERNS_ERR, $msg);
        }
        //In this case there are not optional attributes, so we can also use this set as interpretable attributes
        if(_patterns_has_uninterpretable_attributes($data, $mandatory_attributes, $msg)) {
          $status = PATTERNS_WARN;
        }
        
      }
      break;
    case 'user':
      switch ($action) {
        case PATTERNS_CREATE:
          //Check mandatory fields
          $mandatory_attributes = array('name', 'mail', 'password', 'timezone', 'status');
          if (!_patterns_has_all_mandatory_attributes($data, $mandatory_attributes, $msg)) {
            return patterns_results(PATTERNS_ERR, $msg);
          }
          //In this case we will need to define as well the attributes generated by the hook_prepare(): uid and pass
          $interpretable_attributes = array('name', 'mail', 'password', 'timezone', 'status', 'uid', 'pass');
          if(_patterns_has_uninterpretable_attributes($data, $interpretable_attributes, $msg)) {
            $status = PATTERNS_WARN;
          }
          break;
        case PATTERNS_MODIFY:
          //Check mandatory fields
          $mandatory_attributes = array('uid');
          if (!_patterns_has_all_mandatory_attributes($data, $mandatory_attributes, $msg)) {
            return patterns_results(PATTERNS_ERR, $msg);
          }
          //All the fields but uid are in this case optional. Pass is generated by hook_prepare()
          $interpretable_attributes = array('uid','name', 'mail', 'timezone', 'status', 'pass');
          if(_patterns_has_uninterpretable_attributes($data, $interpretable_attributes, $msg)) {
            $status = PATTERNS_WARN;
          }
          break;
        case PATTERNS_DELETE:
          //Check mandatory fields, in this case is only one.
          $mandatory_attributes = array('uid');
          if (!_patterns_has_all_mandatory_attributes($data, $mandatory_attributes, $msg)) {
            return patterns_results(PATTERNS_ERR, $msg);
          }
          //In this case there are not optional attributes, so we can also use this set as interpretable attributes
          if(_patterns_has_uninterpretable_attributes($data, $mandatory_attributes, $msg)) {
            $status = PATTERNS_WARN;
          }
          break;
      }
      break;
      case 'role_users':
        switch ($action) {
          case PATTERNS_CREATE:
          case PATTERNS_DELETE:
            //Check mandatory fields, in this case the syntax is the same for DELETE and CREATE
            $mandatory_attributes = array('role', 'users');
            if (!_patterns_has_all_mandatory_attributes($data, $mandatory_attributes, $msg)) {
              return patterns_results(PATTERNS_ERR, $msg);
            }
            //In this case there are not optional attributes, so we can also use this set as interpretable attributes
            if(_patterns_has_uninterpretable_attributes($data, $mandatory_attributes, $msg)) {
              $status = PATTERNS_WARN;
            }
            break;
        default:
          $msg = t('Action %action is uncompatible for tag %tag.', array('%action' =>$action, '%tag' =>$tag,));
          return patterns_results(PATTERNS_ERR, $msg);
        }
        break;
  }
  
/*
 * Semantic validation: 
 * - Semantic warnings: They refer to the meaning of the pattern itself, and they 
 * might provoke execution errors if they are not solved.
 * 
 */
  switch ($tag) {
    case 'role':
      //Prepare common role values
      $current_roles = user_roles();
      switch ($action) {
        case PATTERNS_CREATE:
          $role_name = $data['name'];
          //Create semantic warning if the role name has already been defined
          if (in_array($role_name, $current_roles)) {
            $result[] = array(PATTERNS_WARNING_ALREADY_DEFINED_ELEMENT => 
              t('The role name %role_name already exists in the system.', array('%role_name' =>$role_name,)));
          }
          break;
        case PATTERNS_MODIFY:
          $role_name = $data['name'];
          $role_rid = $data['rid'];
          //Create semantic warning if the rid does not exist
          if (!array_key_exists($role_rid, $current_roles)) {
            $result[] = array(PATTERNS_WARNING_ELEMENT_UNDEFINED => 
              t('The role id %rid does not exist in the system.', array('%rid' =>$role_rid,)));
          }
          //Create semantic warning if the role name has already been defined
          if (in_array($role_name, $current_roles)) {
            $result[] = array(PATTERNS_WARNING_ALREADY_DEFINED_ELEMENT => 
              t('The role name %role_name already exists in the system.', array('%role_name' =>$role_name,)));
          }
          break;
        case PATTERNS_DELETE:
          $role_rid = $data['rid'];
          if (!array_key_exists($role_rid, $current_roles) && !in_array($role_rid, $current_roles)) {
            $result[] = array(PATTERNS_WARNING_ELEMENT_UNDEFINED => 
              t('The value %rid does not exist in the system as role name neither as a numeric rid.', array('%rid' =>$role_rid,)));
          }
          break;
      }
      break;
    case 'permission':
      //This is the only case available, the lack of the rest are checking during the syntactic validation
      if($action == PATTERNS_MODIFY) {
        $permissions = $data['value'];
        $role = $data['role'];
        //Check if all permissions exist, if any of them don't raise a syntactic warning
        $available_permissions = user_permission_get_modules();
        $msg = "";
        foreach ($permissions as $permission) {
          if (!array_key_exists($permission, $available_permissions)) {
            $msg .= t('Permission %permission is not available in the system.<br>', array('%permission' =>$permission,));
          }
        }
        if ($msg !="") {
          $result[] = array(PATTERNS_WARNING_ELEMENT_UNDEFINED => $msg);
        }
        //Raise semantic warning if the role name does not exist
        if (!user_role_load_by_name($role)) {
          $result[] = array(PATTERNS_WARNING_UNMET_DEPENDENCY =>
              t('The role %role does not exist in the system.', array('%role' =>$role,)));
        }
      }
            
      break;
    case 'user':
      $users = _user_patterns_get_all_users();
      switch ($action) {
        case PATTERNS_CREATE:
          //Create semantic warning if username or mail has already been defined
          if (user_load_by_name($data['name'])) {
            $result[] = array(PATTERNS_WARNING_ALREADY_DEFINED_ELEMENT =>
                t('The username %username already exists in the system.', array('%username' =>$data['name'],)));
          }
          if (user_load_by_mail($data['mail'])) {
            $result[] = array(PATTERNS_WARNING_ALREADY_DEFINED_ELEMENT =>
                t('The username %mail already exists in the system.', array('%mail' =>$data['mail'],)));
          }
          break;
        case PATTERNS_MODIFY:
          //Create semantic warning if the uid does not exist
          if (!user_load($data['uid'])) {
            $result[] = array(PATTERNS_WARNING_ELEMENT_UNDEFINED =>
              t('The User ID %uid does not exist in the system.', array('%uid' =>$data['uid'],)));
          }
          break;
        case PATTERNS_DELETE:
          //Create semantic warning if the uid does not exist
          if (!user_load($data['uid'])) {
            $result[] = array(PATTERNS_WARNING_ELEMENT_UNDEFINED =>
              t('The User ID %uid does not exist in the system.', array('%uid' =>$data['uid'],)));
          }
          break;
      }
      break;
      case 'role_users':
        switch ($action) {
          case PATTERNS_CREATE:
          case PATTERNS_DELETE:
            //Create semantic warning if the role does not exist
            if (!user_role_load_by_name($data['role'])) {
              $result[] = array(PATTERNS_WARNING_UNMET_DEPENDENCY =>
                  t('The role %role does not exist in the system.', array('%role' =>$data['role'],)));
            }
            //We raised a semantic error for each username that does not exist. 
            foreach ($data['users'] as $username) {
              if (!user_load_by_name($username)) {
                $result[] = array(PATTERNS_WARNING_UNMET_DEPENDENCY =>
                    t('The username %username does not exist in the system.', array('%username' =>$username,)));
              }
            }
            break;
        }
      break;
  }
  return patterns_results($status, $msg, $result);
}


// Prepare for valid processing of this type of component
function user_patterns_build($action, $form_id, &$data = NULL, &$a) {
  $status = PATTERNS_SUCCESS;
  $msg = '';
  $result = NULL;

  if ($form_id == 'user_register_form') {
    if (isset($data['uid'])) {
      unset($data['uid']);
    }

    // Ste: added
//        if (!isset($data['user'])) {
//          $data['user'] = (object) $data;
//        }

  }
  elseif ($form_id == 'user_profile_form') {
    // Make sure that required fields are not left empty.
    if (empty($data['mail'])) {
      $data['mail'] = db_query("SELECT mail FROM {users} WHERE uid = :uid", array('uid' => $data['uid']))->fetchField();
    }
    if (empty($data['name'])) {
      $data['name'] = db_query("SELECT name FROM {users} WHERE uid = :uid", array('uid' => $data['uid']))->fetchField();
    }

    static $old_q; // TODO: What is this?
    $old_q = $_GET['q'];
    $_GET['q'] = 'user/' . $data['uid'] . '/edit';
  }
  elseif ($id == 'user_cancel_confirm_form') {
    $data['confirm'] = 1;
    switch ($data['method']) {
      case '1':
        $data['user_cancel_method'] = 'user_cancel_block';
        break;
      case '2':
        $data['user_cancel_method'] = 'user_cancel_block_unpublish';
        break;
      case '3':
        $data['user_cancel_method'] = 'user_cancel_reassign';
        break;
      case '4':
        $data['user_cancel_method'] = 'user_cancel_delete';
        break;
    }
  }
  elseif ($form_id == 'user_admin_roles' || $form_id == 'user_admin_role_delete_confirm' || $form_id == 'user_admin_role') {
    if ($action === PATTERNS_DELETE) {
      $data['op'] = t('Delete');
    }
    elseif ($data['rid']) {
      $data['op'] = t('Save role');
    }
    else {
      $data['op'] = t('Add role');
    }
  }
  $result = $data;

  return patterns_results($status, $msg, $result);

}

// Build a patterns actions and parameters
function user_patterns_params($action, $form_id, &$data = NULL, &$a) {
  if ($form_id == 'user_cancel_confirm_form' && $form_id == 'user_profile_form') {
    $result = array(user_load($data['uid']));
  }
  elseif ($form_id == 'user_admin_role_delete_confirm' || $form_id == 'user_admin_role') {
    $result = array((object) $data);
  }
  return patterns_results(PATTERNS_SUCCESS, t('Execution successful'), $result);
}

// Cleanup any global settings after the action runs
function user_patterns_cleanup($action, $tag, &$data) {
  return patterns_results();
}



/**
 *
 * Wraps the call to drupal_form_submit for 'user_profile_form'.
 * Gathers all the expected values from the form and prepare them for the query.
 *
 * @param string $form_id  String containing the form ID. In the case of custom functions the value is empty.
 * @param array $form_state  Set of values after parsing the action.
 */
function set_user_permissions($form_id, &$form_state) {
  //Transform the rolename into a role object, we don't need to check existance since we did it previously
  $role = user_role_load_by_name($form_state['values']['role']);
  //Prepare all the attributes for the query, but uid
  $permissions = $form_state['values']['value'];
  //Remove all permissions not associated to any module (due to http://drupal.org/node/737816)
  $modules = user_permission_get_modules();
  foreach ($permissions as $key => $name) {
    if (!isset($modules[$name])) {
      unset($permissions[$key]);
    }
  }
  patterns_set_user_permissions($role->rid, $permissions);
  
  $msg = t('Permissions for role %role successfully updated.', array('%role' => $role->name));
  return patterns_results(PATTERNS_SUCCESS, $msg);
}

/**
 *
 * Run Drupal API function to gran permissions
 *
 * @param string $uid  User ID
 * @param array $attributes  Set of values that will be updated. All are optional.
 */
function patterns_set_user_permissions($rid, $permissions) {
  user_role_grant_permissions($rid, $permissions);
}

/**
 * 
 * Wraps the call to drupal_form_submit for 'user_profile_form'.
 * Gathers all the expected values from the form and prepare them for the query.
 * 
 * @param string $form_id  String containing the form ID. In the case of custom functions the value is empty.
 * @param array $form_state  Set of values after parsing the action.
 */
function set_user_values($form_id, &$form_state) {
  // The only mandatory attribute is uid, but it should not be re-validate it (we are imitating the behaviour of the form execution)
  $uid = $form_state['values']['uid'];
  //Prepare all the attributes for the query, but uid
  $attributes = array_diff_key($form_state['values'], array('uid' => $uid));
  patterns_set_user_values($uid, $attributes);
  
  $msg = t('Information for user %uid successfully updated.', array('%uid' => $uid));
  return patterns_results(PATTERNS_SUCCESS, $msg);
}

/**
 * 
 * Run a query to update the fields of the user
 * 
 * @param string $uid  User ID
 * @param array $attributes  Set of values that will be updated. All are optional.
 */
function patterns_set_user_values($uid, $attributes) {
  //Run a query with all the optional sent values that are set.
  $num_updated = db_update('users')
  ->fields($attributes)
  ->condition('uid', $uid, '=')
  ->execute();
}


/**
 * Wraps the call to drupal_form_submit() to assign roles.
 *
 * @param string $form_id  String containing the form ID. In the case of custom functions the value is empty.
 * @param array $form_state  Set of values after parsing the action.
 */
function set_role_users($form_id, &$form_state) {
  //Transform names in identifiers for role and users
  $role = user_role_load_by_name($form_state['values']['role']);
  $uids = array();
  foreach ($form_state['values']['users'] as $username) {
    $u = user_load_by_name($username);
    $uids[] = $u->uid;
  }
  //Add the role to the list of uids
  user_multiple_role_edit($uids, 'add_role', $role->rid);
  
  $msg = ((count($uids) > 0) ? t('Role %role was assigned to the following users: %users.',
      array('%role' => $form_state['values']['role'],'%users' => implode(', ', $uids))) : t('Role %role was applied to no user.', array('%role' => $form_state['values']['role'])));
  return patterns_results(PATTERNS_SUCCESS, $msg);
}

/**
 * Wraps the call to drupal_form_submit() to remove roles.
 *
 * @param string $form_id  String containing the form ID. In the case of custom functions the value is empty.
 * @param array $form_state  Set of values after parsing the action.
 */
function unset_role_users($form_id, &$form_state) {
  //Transform names in identifiers for role and users
  $role = user_role_load_by_name($form_state['values']['role']);
  $uids = array();
  foreach ($form_state['values']['users'] as $username) {
    $u = user_load_by_name($username);
    $uids[] = $u->uid;
  }
  
  //Remove the role to the list of urls
  user_multiple_role_edit($uids, 'remove_role', $role->rid);
  
  $msg = ((count($uids) > 0) ? t('Role %role was removed to the following users: %users.',
      array('%role' => $form_state['values']['role'],'%users' => implode(', ', $uids))) : t('Role %role was removed to no user.', array('%role' => $form_state['values']['role'])));
  return patterns_results(PATTERNS_SUCCESS, $msg);
}


/**
 * Returns all the users currently stored in the system, excluding the anonymous.
 */
function _user_patterns_get_all_users() {
  $query = db_select('users', 'u');
  $query->fields('u', array('uid'));
  $query->condition('uid', '0', '<>');
  $uids = $query->execute()->fetchCol();
  return user_load_multiple($uids);
}

/**
 * 
 * Returns a list of usernames that belong to the given Role ID
 * 
 * @param string $role  Role ID to filter by
 * @return array List of usernames that currently hold that role
 */
function _user_patterns_users_with_role($role) {
  $users_with_role = array();
  //Get rid and load users
  $role_info = user_role_load_by_name($role);
  $users = entity_load('user');

  //Filter all users that do not have the given role
  foreach($users as $user) {
    if (array_key_exists($role_info->rid, $user->roles)) {
      array_push($users_with_role, $user->name);
    }
  }
  return $users_with_role;
}

<?php
/*
 * @file
 * Patterns component for toolbar.
 */

/**
 * hook_patterns()
 */
function toolbar_patterns($data) {
  $files = array('modules/user/user.admin.inc');
  $actions['toolbarrole'] = array(
    PATTERNS_INFO => t('Decide who can see the toolbar'),
    PATTERNS_MODIFY => array('user_admin_permissions'),
    PATTERNS_FILES => $files,
    PATTERNS_EXPORT => array(PATTERNS_EXPORT_ALL => 'toolbar_patterns_export_all_roles',),
  );
  return $actions;
}

/**
 * 
 * Returns a PATTERNS_MODIFY action with the set of all the roles of the
 * system groupped in 'addroles' or 'delroles' according to if they have
 * permission to access or not the toolbar. The 'administrator' role is excluded.
 * 
 * Since this property depends on a specific permission, we cannot make
 * user of an auxiliary function such as patterns_export_actions_from_form().
 * 
 * @param string $args
 * @param string $result
 * @return array $actions
 */
function toolbar_patterns_export_all_roles($args = NULL, $result = NULL) {

  switch ($args['type']) {
    case PATTERNS_MODIFY:
      //Prepare auxiliary variables
      $actions = array(PATTERNS_MODIFY => array('tag' => 'toolbarrole'));
      $roles_in = array();
      $roles_out = array();
      
      //Go through all the roles to classify them in the proper group excluding the admin
      foreach (user_roles() as $role) {
        if ($role!='administrator') {
          if (in_array($role, user_roles(FALSE, 'access toolbar'))) {
            array_push($roles_in, $role);
          }else{
            array_push($roles_out, $role);
          }      
        }
      }
      
      //Both sections need to be added in any case (even if the array is empty)
      $actions[PATTERNS_MODIFY]['addroles'] = $roles_in;
      $actions[PATTERNS_MODIFY]['delroles'] = $roles_out;    
      
      $result = array($actions);
      break;
  }

  return $result;
}

/**
 * hook_patterns_prepare()
 */
function toolbar_patterns_prepare($action, $tag, &$data) {

  module_load_include('module', 'user', 'user');
  $data['role_names'] = user_roles();
  $data['role_permissions'] = user_role_permissions($data['role_names']);

  //get all the module's name which inplement the hook_permission function.
  $module_info = system_get_info('module');
  $modules = array();
  foreach (module_implements('permission') as $module) {
    $modules[$module] = $module_info[$module]['name'];
  }
  asort($modules);

  //three foreach,every module implementing the hook_permission,every permission from module's hook_permission,every role for every permission.
  //$data['status'] get every rid's permission. asort by module's name.
  foreach ($modules as $module => $display_name) {
    if ($permissions = module_invoke($module, 'permission')) {
      foreach ($permissions as $perm => $perm_item) {
        foreach ($data['role_names'] as $rid => $name) {
          // Builds arrays for checked boxes for each role
          if (isset($data['role_permissions'][$rid][$perm])) {
            $data['status'][$rid][] = $perm;
          }
        }
      }
    }
  }

  return patterns_results();
}

/**
 * hook_patterns_validate()
 */
function toolbar_patterns_validate($action, $tag, &$data) {
  $result = array();
  $status = PATTERNS_SUCCESS;
  $msg = '';


  switch ($tag) {
    case 'toolbarrole' :
      //if both addroles and delroles is not given, there is a error.
      if (!(isset($data['addroles'])) && !(isset($data['delroles']))) {
        $status = PATTERNS_ERR;
        $msg = t('there is nothing to do.');
        return patterns_results($status, $msg, $result);
      }

     //if there is a same name in add and delroles, there is a error.
      if (is_array($data['addroles']) && is_array($data['delroles'])) {
        $a = array_intersect($data['addroles'], $data['delroles']);
        if (count($a) != 0) {
          $status = PATTERNS_ERR;
          $msg = t('same name in both addroles and delroles');
          return patterns_results($status, $msg, $result);
        }
      }

      //the process of addroles.
      //role can not be administrator
      //the name should exist in the role table.
      //if the role already have the perm, there is a error.
      if (is_array($data['addroles'])) {
        foreach ($data['addroles'] as $key => $value) {
          $rid = db_select('role', 'r')
                  ->fields('r', array('rid'))
                  ->condition('r.name', $value)
                  ->execute()
                  ->fetchField();
          if ($rid == 3) {
            $status = PATTERNS_ERR;
            $msg = t('administrator should keep static.');
            return patterns_results($status, $msg, $result);
          }
          if ($rid == FALSE) {
            $result[] = array(PATTERNS_WARNING_UNMET_DEPENDENCY => 
              t('The role %role does not exist in the system.', array('%role' =>$value,)));
          }
          if (isset($data['status'][$rid])) {
            if (in_array("access toolbar", $data['status'][$rid])) {
              $result[] = array(PATTERNS_WARNING_ALREADY_DEFINED_ELEMENT => 
                t('The role %role already have toolbar permission in the system', array('%role' => $value,)));
            }
          }

          //add the "access toolbar" perm to the $data['status']
          $data['status'][$rid][] = "access toolbar";
        }
      }

      //the process of delroles.
      //role can not be administrator
      //the name should not exist in the role table.
      //if the role don't have the perm yet, there is a error.
      if (is_array($data['delroles'])) {
        foreach ($data['delroles'] as $key => $value) {
          $rid = db_select('role', 'r')
                  ->fields('r', array('rid'))
                  ->condition('r.name', $value)
                  ->execute()
                  ->fetchField();
          if ($rid == 3) {
            $status = PATTERNS_ERR;
            $msg = t('administrator should keep static.');
            return patterns_results($status, $msg, $result);
          }
          if ($rid == FALSE) {
            $result[] = array(PATTERNS_WARNING_UNMET_DEPENDENCY => 
              t('The role %role does not exist in the system.', array('%role' =>$value,)));
          }
          //ensure the role have some perms.
          if (isset($data['status'][$rid])) {
            if (!(in_array("access toolbar", $data['status'][$rid]))) {
              $result[] = array(PATTERNS_WARNING_UNMET_DEPENDENCY => 
                t('The role %role does not have toolbar permission yet in the system.', array('%role' =>$value,)));
            }
            //delete the "access toolbar" from $data['status']
            $data['status'][$rid] = array_diff($data['status'][$rid], array("access toolbar"));
          }
          else {
            $result[] = array(PATTERNS_WARNING_UNMET_DEPENDENCY => 
              t('The role %role does not have any permission yet in the system.', array('%role' =>$value,)));
          }
        }
      }

      //the $data[$rid] is the form's element.
      foreach ($data['status'] as $rid => $value) {
        $data[$rid] = $value;
      }
  }

  return patterns_results($status, $msg, $result);
}

/**
 * hook_patterns_callbacks()
 */
function toolbar_patterns_callbacks($action, $tag, &$data) {
  $result = array('user_admin_permissions');
  return patterns_results(PATTERNS_SUCCESS, t('Execution successful'), $result);
}

/**
 * hook_patterns_build()
 */
function toolbar_patterns_build($action, $form_id, &$data = NULL, &$a) {
  module_load_include('inc', 'user', 'user.admin');
  return patterns_results(PATTERNS_SUCCESS, t('Execution successful(in build function)'), $data);
}


/**
 * hook_patterns_params()
 */
function toolbar_patterns_params($action, $form_id, &$data = NULL, &$a) {
  return patterns_results();
}

/**
 * hook_patterns_cleanup()
 */
function toolbar_patterns_cleanup($action, $tag, &$data) {
  return patterns_results();
}

<?php
/**
 * @file
 * Patterns component for shortcut.
 *
 * There are seven forms in the shortcut module. They can be separated into three kinds:
 * set, link, switch for user.
 *
 * set(three forms):
 *     add a shortcut set: just give a name for the new shortcut set.
 *     delete a shortcut set: just supply a shortcut set's name.
 *     rename a shortcut set: need two name, one is used to define tht shortcut set which will be modified, the other is the new name.
 *
 * link(three forms):
 *     add a shortcut link: supply the name of a shortcut set which the new link will be in,supply the link's title and path.
 *     delete a shortcut link: just give the link's id, the id is in the menu_links table of the database.
 *     edit a shortcut link: need the link's id, need the link's new title or/and new path.
 *
 * switch for user(one forms);
 *     need a user's name
 *     need a shortcut set'name
 *     the user will use the shortcut set.
 */

/**
 * Implements hook_patterns().
 */
function shortcut_patterns($data = NULL) {
  $files = array('modules/shortcut/shortcut.admin.inc');

  $actions['shortcut_set'] = array(
    PATTERNS_INFO => t('create/delete/modify shortcut set'),
    PATTERNS_CREATE => array('shortcut_set_add_form'),
    PATTERNS_MODIFY => array('shortcut_set_edit_form'),
    PATTERNS_DELETE => array('shortcut_set_delete_form'),
    PATTERNS_FILES => $files,
    PATTERNS_EXPORT => array(PATTERNS_EXPORT_ALL => 'shortcut_patterns_export_all_set',),
    );
  $actions['shortcut_link'] = array(
    PATTERNS_INFO => t('add a new link to one shortcut_set'),
    PATTERNS_CREATE => array('shortcut_link_add'),
    PATTERNS_MODIFY => array('shortcut_link_edit'),
    PATTERNS_DELETE => array('shortcut_link_delete'),
    PATTERNS_FILES => $files,
    PATTERNS_EXPORT => array(PATTERNS_EXPORT_ALL => 'shortcut_patterns_export_all_link',),
    );
  $actions['shortcut_set_user'] = array(
    PATTERNS_INFO => t('one user get one shortcut_set'),
    PATTERNS_MODIFY => array('shortcut_set_switch'),
    PATTERNS_FILES => $files,
    PATTERNS_EXPORT => array(
    PATTERNS_EXPORT_ALL => 'shortcut_patterns_export_all_user_set',),
    );

  return $actions;
}


/**
 *
 * Returns a set of PATTERNS_CREATE actions with the whole set of shortcuts
 * currently stored in the system.
 *
 * @param string $args
 * @param string $result
 * @return array $actions
 */
function shortcut_patterns_export_all_set($args = NULL, $result = NULL) {
  //Loop through the elements in the set to prepare the actions
  $sets = shortcut_sets();
  $actions = array();

  switch ($args['type']) {
    case PATTERNS_CREATE :
      foreach ($sets as $set_name) {
        //Create action only require a name given to the new shortcut_set.
        $action = array(PATTERNS_CREATE => array('tag' => 'shortcut_set', 'shortcut_set_name' => $set_name->title));
        array_push($actions, $action);
      }
      break;
    case PATTERNS_MODIFY :
      foreach ($sets as $set_name) {
        //Modify shortcut_set need a newname which shold be assigned by someone(Administrator)
        $action = array(PATTERNS_MODIFY => array('tag' => 'shortcut_set', 'oldname' => $set_name->title), 'newname' => 'PleaseGiveNewName');
        array_push($actions, $action);
      }
      break;
  }

  return $actions;
}

/**
 *
 * Returns a set of PATTERNS_CREATE actions with the whole set of links
 * currently stored in the system.
 *
 * @param string $args
 * @param string $result
 * @return array $actions
 */
function shortcut_patterns_export_all_link($args = NULL, $result = NULL) {
  //Loop through the elements in the set to prepare the actions
  $sets = shortcut_sets();
  $actions = array();


  switch ($args['type']) {
    case PATTERNS_CREATE :
      foreach ($sets as $set_name) {
        //Retrieve all the elements, that are stored as normal menu entries. Shortcuts always have a depth level of 1
        $shortcut_links = menu_tree_all_data($set_name->set_name, NULL, 1);
        foreach ($shortcut_links as $shortcut_link) {
          //Go through elements extracting link_path and link_title fields
          $action = array(PATTERNS_CREATE => array('tag' => 'shortcut_link', 'shortcut_set_name' => $set_name->title,
            'shortcut_link_title' => $shortcut_link['link']['link_title'], 'shortcut_link_path' => $shortcut_link['link']['link_path']));
          array_push($actions, $action);
        }
      }
      break;
    case PATTERNS_MODIFY:
      foreach ($sets as $set_name) {
        //Retrieve all the elements, that are stored as normal menu entries. Shortcuts always have a depth level of 1
        $shortcut_links = menu_tree_all_data($set_name->set_name, NULL, 1);
        foreach ($shortcut_links as $shortcut_link) {
          //Modify action require new link title and new link path which should be given by someone(website administrator,i.e.)
          $action = array(PATTERNS_MODIFY => array('tag' => 'shortcut_link', 'set_name' => $set_name->title,
            'link_title' => $shortcut_link['link']['link_title'], 'new_link_title' => 'PleaseGiveNewLinkName', 'new_link_path' => 'PleaseGiveNewLinkPath'));
          array_push($actions, $action);
        }
      }
      break;
  }

  return $actions;
}

/**
 *
 * Returns a set of PATTERNS_MODIFY actions with the whole set of relationships
 * between shortcut set and related user currently stored in the system.
 *
 * @param string $args
 * @param string $result
 * @return array $actions
 */
function shortcut_patterns_export_all_user_set($args = NULL, $result = NULL) {
  $actions = array();
  switch ($args['type']) {
    case PATTERNS_MODIFY:
      //There is currently no function provided by Drupal to get the relationship
      //between users and selected shortcut sets, we need to query the DB directly
      $query = db_select('shortcut_set_users', 'ssu');
      $query->join('shortcut_set', 'ss', 'ssu.set_name = ss.set_name');
      $query
        ->fields('ssu', array('uid', 'set_name'))
        ->fields('ss', array('set_name', 'title'));
      $result = $query->execute();
      
      //Loop through the elements in the $result set to prepare the actions using set title and username
      foreach ($result as $record) {
        $action = array(PATTERNS_MODIFY => 
          array(
            'tag' => 'shortcut_set_user',
            'name' => $record->title, 
            'user' => user_load($record->uid)->name));
        array_push($actions, $action);
      }
      break;
  }

  return $actions;
}




/**
 * hook_patterns_prepare()
 */
function shortcut_patterns_prepare($action, $tag, &$data) {
  // for set.
  if ($tag == 'shortcut_set') {
    if ($action == PATTERNS_CREATE ) {
      $data['new'] = $data['shortcut_set_name'];
    }
    if ($action == PATTERNS_MODIFY ) {
      $data['title'] = $data['newname'];
      $data['searchtitle'] = $data['oldname'];
    }
    if ($action == PATTERNS_DELETE) {
      $data['searchtitle'] = $data['name'];
    }
  }

  // for switch user
  if ($tag == 'shortcut_set_user') {
    $data['searchtitle'] = $data['name'];
    $data['account'] = NULL;
  }

  // for link
  if ($tag == 'shortcut_link' ) {
    if ($action == PATTERNS_CREATE) {
      $data['searchtitle'] = $data['shortcut_set_name'];
      $data['shortcut_link']['link_title'] = $data['shortcut_link_title'];
      $data['shortcut_link']['link_path'] = $data['shortcut_link_path'];
    }
    if ($action == PATTERNS_MODIFY) {
      $data['shortcut_link']['link_title'] = $data['new_link_title'];
      $data['shortcut_link']['link_path'] = $data['new_link_path'];
    }
  }

  return patterns_results();
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
function shortcut_patterns_validate($action, $tag, &$data) {
  /* @TO-DO: The checking will be performed as follows:
   * - Check type of tag in a switch
   * -- Check type of $action in an internal switch for each tag
   * --- Raise errors if any through msg and status
   * --- Raise warning if any thrugh $result
   */
  $result = array();
  $status = PATTERNS_SUCCESS;
  $msg = '';
  switch ($tag) {
    // shortcut_set tag.
    case 'shortcut_set' :
      $sets = shortcut_sets();
      switch ($action) {
        case PATTERNS_CREATE :
          $set = db_select('shortcut_set', 'ss')
                  ->fields('ss', array('set_name'))
                  ->condition('title', $data['new'])
                  ->execute()
                  ->fetchField();
          if ($set) {
            $result[] = array(PATTERNS_WARNING_ALREADY_DEFINED_ELEMENT => 
                t('The shortcut set %set_name already exists in the system', array('%set_name' => $data['new'],)));
          }
          break;
        case PATTERNS_MODIFY :
          $set_for_title = db_select('shortcut_set', 'ss')
                  ->fields('ss', array('set_name'))
                  ->condition('title', $data['title'])
                  ->execute()
                  ->fetchField();
          if ($set_for_title) {
            $result[] = array(PATTERNS_WARNING_ALREADY_DEFINED_ELEMENT => 
              t('The shortcut set %set_name already exists in the system', array('%set_name' => $data['title'],)));
          }
          $set_for_searchtitle = db_select('shortcut_set', 'ss')
                  ->fields('ss', array('set_name'))
                  ->condition('title', $data['searchtitle'])
                  ->execute()
                  ->fetchField();
          if (!($set_for_searchtitle)) {
            $result[] = array(PATTERNS_WARNING_ELEMENT_UNDEFINED => 
              t('The shortcut set %oldname does not exist in the system.', array('%oldname' =>$data['searchtitle'],)));
          }
          if (!($set_for_title)&& $set_for_searchtitle) {
            module_load_include('module', 'shortcut');
            $shortcutsetobj = shortcut_set_load($set_for_searchtitle);
            $data['shortcut_set_obj'] = $shortcutsetobj;
          }
          break;
        case PATTERNS_DELETE :
          $set_for_searchtitle = db_select('shortcut_set', 'ss')
                  ->fields('ss', array('set_name'))
                  ->condition('title', $data['searchtitle'])
                  ->execute()
                  ->fetchField();
          if (!($set_for_searchtitle)) {
            $result[] = array(PATTERNS_WARNING_ELEMENT_UNDEFINED => 
              t('The shortcut set %oldname does not exist in the system.', array('%oldname' =>$data['searchtitle'],)));
          }
          else {
            module_load_include('module', 'shortcut');
            $shortcutsetobj = shortcut_set_load($set_for_searchtitle);
            $data['shortcut_set_obj'] = $shortcutsetobj;
          }
          break;

      }
      break;

    //shortcut_link tag.
    case 'shortcut_link' :
      switch ($action) {
        case PATTERNS_CREATE :
          $set_for_searchtitle = db_select('shortcut_set', 'ss')
                  ->fields('ss', array('set_name'))
                  ->condition('title', $data['searchtitle'])
                  ->execute()
                  ->fetchField();
          if (!($set_for_searchtitle)) {
            $result[] = array(PATTERNS_WARNING_ELEMENT_UNDEFINED => 
              t('The shortcut set %oldname does not exist in the system.', array('%oldname' =>$data['searchtitle'],)));
          }
          else {
            $shortcutlink = db_select('menu_links', 'ml')
                              ->fields('ml')
                              ->condition('menu_name', $set_for_searchtitle)
                              ->condition('link_title', $data['shortcut_link_title'])
                              ->execute()
                              ->fetchAssoc();
            if ($shortcutlink) {
              $result[] = array(PATTERNS_WARNING_ALREADY_DEFINED_ELEMENT => 
                t('The shortcut link %shortcut_link_title already exists in the system', array('%shortcut_link_title' => $data['shortcut_link_title'],)));
            }
            else {
              module_load_include('module', 'shortcut');
              $shortcutsetobj = shortcut_set_load($set_for_searchtitle);
              $data['shortcut_set_obj'] = $shortcutsetobj;
            }
          }
          break;
        case PATTERNS_MODIFY :
        case PATTERNS_DELETE :
          $set_name = db_select('shortcut_set', 'ss')
                          ->fields('ss', array('set_name'))
                          ->condition('title', $data['set_name'])
                          ->execute()
                          ->fetchField();
          if (!($set_name)) {
            $result[] = array(PATTERNS_WARNING_ELEMENT_UNDEFINED => 
              t('The shortcut set %oldname does not exist in the system.', array('%oldname' =>$data['set_name'],)));
          }
          else {
            $shortcutlink = db_select('menu_links', 'ml')
                              ->fields('ml')
                              ->condition('menu_name', $set_name)
                              ->condition('link_title', $data['link_title'])
                              ->execute()
                              ->fetchAssoc();
            if (!($shortcutlink)) {
              $result[] = array(PATTERNS_WARNING_ELEMENT_UNDEFINED => 
                t('The shortcut link %oldname does not exist in the system.', array('%oldname' =>$data['link_title'],)));
            }
            $data['shortcut_link_parm'] = $shortcutlink;
          }
          break;
      }
      break;

    //shortcut_set_user tag.
    case 'shortcut_set_user' :
       switch ($action) {
        case PATTERNS_MODIFY :
          $set_for_searchtitle = db_select('shortcut_set', 'ss')
                  ->fields('ss', array('set_name'))
                  ->condition('title', $data['searchtitle'])
                  ->execute()
                  ->fetchField();
          if (!($set_for_searchtitle)) {
            $result[] = array(PATTERNS_WARNING_ELEMENT_UNDEFINED => 
              t('The shortcut set %oldname does not exist in the system.', array('%oldname' =>$data['searchtitle'],)));
          }
          else {
            // This form has the third parm: $account, the following code provides it.
            if (isset($data['user'])) {
              module_load_include('module', 'user');
              if (!($data['account'] = user_load_by_name( $data['user']))) {
                $result[] = array(PATTERNS_WARNING_ELEMENT_UNDEFINED => 
                  t('The user %username does not exist in the system.', array('%username' =>$data['user'],)));
              }
            }

            module_load_include('module', 'shortcut');
            $shortcutsetobj = shortcut_set_load($set_for_searchtitle);
            $data['shortcut_set_obj'] = $shortcutsetobj;
            $data['set'] = $set_for_searchtitle;
            $data['new'] = $data['name'];
          }
          break;
      }
      break;
  }
  return patterns_results($status, $msg, $result);
}
 
/**
 * hook_patterns_validate()
 */
 /*
function shortcut_patterns_validate($action, $tag, &$data) {

  $status = PATTERNS_SUCCESS;
  $msg = '';

  //for link's edit and delete.
  //these two form have the third parm:$shortcut_link, follow code provide the parm.
  if ($tag == 'shortcut_link') {
    $set_name = '';
    if ($action == PATTERNS_MODIFY || $action == PATTERNS_DELETE) {
      $set_name = db_select('shortcut_set', 'ss')
        ->fields('ss', array('set_name'))
        ->condition('title', $data['set_name'])
        ->execute()
        ->fetchField();
      if ($set_name == FALSE) {
        $status = PATTERNS_ERR;
        $msg = 'Shortcut Error:there is no such name shortcut set!';
        return patterns_results($status, $msg);
      }
      else {
        $shortcutlink = db_select('menu_links', 'ml')
          //->fields('ml', array('menu_name','link_title', 'link_path', 'mlid'))
          ->fields('ml')
          ->condition('menu_name', $set_name)
          ->condition('link_title', $data['link_title'])
          ->execute()
          ->fetchAssoc();
          //follow line is for test
          //drupal_set_message(t('the %acc is %mlid  dfldjdkfdld',array('%acc' =>$shortcutlink['link_title'],'%mlid' => $shortcutlink['mlid'])));
        if ($shortcutlink == FALSE) {
          $status = PATTERNS_ERR;
          $msg = t('Shortcut Error: there is no such name shortcut link');
          return patterns_results($status, $msg);
        }
        $data['shortcut_link_parm'] = $shortcutlink;
      }
    }
  }

  //for set's edit&delete and link's add and switch user.
  //these three form(switch user just use the $set) have the third parm:$shortcut_set, follow code provide the parm.
  if (($tag == 'shortcut_set' && $action != PATTERNS_CREATE) || ($tag == 'shortcut_link' && $action == PATTERNS_CREATE) || ($tag == 'shortcut_set_user')) {
    $set = db_select('shortcut_set', 'ss')
      ->fields('ss', array('set_name'))
      ->condition('title', $data['searchtitle'])
      ->execute()
      ->fetchField();
    if ($set == FALSE) {
      $status = PATTERNS_ERR;
      $msg = t('Shortcut Error: there is no such name shortcut set');
      return patterns_results($status, $msg);
    }
    else {
      module_load_include('module', 'shortcut');
      $shortcutsetobj = shortcut_set_load($set);
      $data['shortcut_set_obj'] = $shortcutsetobj;
      if ($tag == 'shortcut_set_user') {
        $data['set'] = $set;
        $data['new'] = $data['name'];
      }
    }
    if ($tag =='shortcut_set_user') {
      // This form has the third parm: $account, the following code provides it.
      if (isset($data['user'])) {
        module_load_include('module', 'user');
        if (($data['account'] = user_load_by_name( $data['user'])) == FALSE) {
          $status = PATTERNS_ERR;
          $msg = t('Shortcut Error:there is no such a user');
          return patterns_results($status, $msg);
        }
      }
    }

  }
   
  return patterns_results($status, $msg);
}
*/

/**
 * hook_patterns_callbacks()
 */
function shortcut_patterns_callbacks($action, $tag, &$data) {
  $allform = shortcut_patterns();
  $result = $allform[$tag][$action];
  return patterns_results(PATTERNS_SUCCESS, t('Execution successful(in callbacks function)'), $result);
}


/**
 * hook_patterns_build()
 */
function shortcut_patterns_build($action, $form_id, &$data = NULL, &$a) {
  module_load_include('inc', 'shortcut', 'shortcut.admin');
  return patterns_results(PATTERNS_SUCCESS, t('Execution successful(in build function)'), $data);
}

/**
 * hook_patterns_params()
 */
function shortcut_patterns_params($action, $form_id, &$data = NULL, &$a) {
  //provide the third parm.
  switch ($form_id) {
    case 'shortcut_set_edit_form':
    case 'shortcut_set_delete_form':
    case 'shortcut_link_add':
      $result = array( $data['shortcut_set_obj'] );
      break;
    case 'shortcut_set_switch':
      $result = array( $data['account'] );
      break;
    case 'shortcut_link_delete':
    case 'shortcut_link_edit':
      $result = array( $data['shortcut_link_parm']);
      break;
  }
  return patterns_results(PATTERNS_SUCCESS, t('Execution successful'), $result);
}

/**
 * hook_patterns_get_arguments()
 */
function shortcut_patterns_get_arguments($action, $tag, $form_id, &$loop = FALSE) {
  if ($tag == "shortcut_set") {
    $loop = TRUE;
    $return = array();
    $sets = shortcut_sets();
    foreach ($sets as $set_name => $value) {
      array_merge($return, array('oldname' => $value['title']));
      return $return;
    }
    return $return;
  }
}

/**
 * hook_patterns_cleanup()
 */
function shortcut_patterns_cleanup($action, $tag, &$data) {
  return patterns_results();
}

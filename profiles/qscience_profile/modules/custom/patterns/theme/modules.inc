<?php
/**
 * @file
 * Patterns modules page theme functions.
 */

/**
 * Lists the modules used by a particular pattern.
 *
 * @TODO Doc.
 */
function patterns_modules_page($pid) {
  $pattern = patterns_utils_if_invalid_go_back($pid);

  drupal_set_title(t('Check modules required for pattern: ' . $pattern->title));
  
  $modules = isset($pattern->pattern['modules']) ? $pattern->pattern['modules'] : array();
  $modules = is_array($modules) ? $modules : array($modules);
  $modules_info = system_rebuild_module_data();
  $modules_list = module_list();

  $not_available = FALSE;
  
  
  $header = array(t('Name'), t('Available'), t('Enabled'), t('Actions'));
  
//  $header = array(
//    'name' => array('data' => t('Name'), 'field' => 'n.name', 'sort' => 'asc'),
//    'available' => array('data' => t('Available'), 'field' => 'n.available'),
//    'enabled' => array('data' => t('Enabled'), 'field' => 'n.enabled'),
//    'actions' => array('data' => t('Actions'), 'field' => 'n.actions'),
//  );
  
  $rows = array();
  // Get module name, whether its to be disabled or enabled,
  // whether the module is available or not, and whether it is
  // currently enabled or not

  foreach ($modules as $module) {
    $module = is_array($module) ? $module['value'] : $module;
    $available = array_key_exists($module, $modules_info);
    $enabled = array_key_exists($module, $modules_list);
    
    $row = array();
    $row['name'] = $module;
    $row['available'] = $available ? t('Yes') : '<span class="alert">' . t('No') . '</span>';
    $row['enabled'] = $enabled ? t('Yes') : '<span class="alert">' . t('No') . '</span>';

//    if ($available && ! $enabled) {
//      $row['actions'] = l(t('Enable'), 'admin/patterns/drush/enable/' . $module);
//
//    }
//    elseif (!$available) {
//      $row['actions'] = l(t('Download and Enable'), 'admin/patterns/drush/dlenable/' . $module);
//    }
//    else {
//      $row['actions'] = '-';
//    }

    $row['actions'] = t('No available actions.');
    //$row[] = '<span id="action_results"></span>';
    
    //$row['#attributes'] = array('class' => array('blue-row'));

    $rows[$module] = $row;

    if (!$available) {
      $not_available = TRUE;
    }
  }

  if ($not_available) {
    drupal_set_message(t('Some modules are not available, please download them before running this pattern.'), 'error');
  }
  else {
    drupal_set_message(t('All modules required by this module are available. Click !here to run this pattern.', array('!here' => l(t('here'), 'admin/patterns/enable/' . $pid))));
  }

  if (empty($rows)) {
    $rows = array(
      array(
        'data' => array(
          array(
            'data' => t('No extra modules are required to run this pattern.'),
            'colspan' => '5',
          ),
        ),
      ),
    );

  }

  $out = theme('table', array('header' => $header, 'rows' => $rows));
  $out .= t('Click !here to view or edit the pattern.', array('!here' => l(t('here'), 'admin/patterns/edit/' . $pid)));
  return $out;
}
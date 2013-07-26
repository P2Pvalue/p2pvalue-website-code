<?php

/**
 * //Seb:
 * Check for the functions about how he gets the fields for the table.(visualscience_get_user_data())
 * @file
 * This file contains several helping functions that are used for the module 
 */

/**
 * 
 * Helper function, returns which user data to search 
 * @return       data names wich are used for the search
 */
function visualscience_get_user_data() {
  return array('name', 'mail', 'role', 'status', 'created');
}

/**
 * 
 * Helper function, returns which node data to search 
 * @return       data names wich are used for the search
 */
function visualscience_get_node_data() {
  return array('type', 'language', 'title', 'created');
}

/**
 * 
 * Returns the appropriate search data
 * @param string $table	the DB table wich data is searched. Possible values are "users" and "node"
 */
function visualscience_get_data($table) {
  return $table=="users"?visualscience_get_user_data():($table=="node"?visualscience_get_node_data():FALSE);
}

/**
 * 
 * Redirects to visualscience page
 */
function visualscience_redirect_to_visualscience() {
  drupal_goto('visualscience');
}

/**
 * returns an array containing all fields
 */
function visualscience_get_all_fields() {
  $fields = field_info_instances('user', 'user');
  $field_names = array();
  foreach ($fields as $name => $value) {
    if (strpos($name, "field_") !== 0)
      continue;
    array_push($field_names, $name);
  }
  return $field_names;
}

/**
 *
 * Case insensitive in_array
 * @param $needle
 * @param $haystack
 */
function in_arrayi($needle, $haystack) {
 return in_array(strtolower($needle), array_map('strtolower', $haystack));
}
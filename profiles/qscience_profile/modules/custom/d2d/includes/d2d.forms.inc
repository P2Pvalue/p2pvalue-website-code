<?php
/**
 * @file
 * Functions to create D2D forms or components of them
 */

/**
 * Decorates an array with a select element containing the current D2D friends 
 * 
 * The array is passed as referenced, and the result is ready to be 
 * rendered by the Drupal Form API
 * 
 * @param Array $form Optional. The reference to the array. Defaults, array().
 * @param mixed $value Optional. The key of the D2D friend object that will be 
 *  used the 'value' attribute of each option element. Defaults, 'd2d_id'. 
 * @param mixed $display Optional. The key of the D2D friend object that will be
 *  used as the content of each option element. Defaults, 'url'.
 */
function d2d_forms_select_friends(&$form = array(), $value = 'd2d_id', $display = 'url') {
  $friends = d2d_api_friend_get();
  
  $options = array();
  foreach($friends as $f) {
    $options[$f[$value]] = $f[$display];
  }
  
  $form += array(
    '#type' => 'select',
    '#options' => $options, 
  );
  
}

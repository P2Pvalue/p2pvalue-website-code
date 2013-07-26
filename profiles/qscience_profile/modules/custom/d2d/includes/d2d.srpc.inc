<?php

/**
 * Implements hook_d2d_secure_rpc().
 *
 * Some methods are associated with a default group during the installation.
 */
function d2d_d2d_secure_rpc() {
  $methods = array();
  $methods['d2d_get_public_key'] = array(
    'arguments' => array('d2d_id' => 'd2d_check_d2d_id'),
    'callback' => 'd2d_srpc_get_public_key',
    'description' => 'returns the public key of the provided instance',
  );
  $methods['d2d_list_permissions'] = array(
    'arguments' => array(),
    'callback' => 'd2d_srpc_list_permissions',
    'description' => 'returns the methods the invocing friend may call',
  );
  return $methods;
}
function d2d_srpc_get_public_key($arguments, $rpc_info) {
  $d2d_id = $arguments['d2d_id'];
  $instance = d2d_api_instance_get($d2d_id);
  return $instance && $instance['public_key'] ? $instance['public_key'] : '';
}
function d2d_srpc_list_permissions($arguments, $rpc_info) {
  $id = $rpc_info['instance']['id'];
  $method_table = d2d_api_method_get($id, TRUE);
  $return_array = array();
  $hks = module_invoke_all('d2d_secure_rpc');
  foreach ($hks as $key => $value) {
    if (array_key_exists($key, $method_table)) {
      $args = '?';
      if (isset($value['arguments'])) {
        $arguments = $value['arguments'];
        if (is_array($arguments)) {
          $args = var_export(array_keys($arguments), TRUE);
        }
      }
      $imploded_array = d2d_implode(
        array(
          'method' => $key,
          'description' => isset($value['description']) ? $value['description'] : '',
          'arguments' => $args,
        )
      );
      if ($imploded_array) {
        $return_array[] = $imploded_array;
      }
    }
  }
  $imploded_return_array = d2d_implode($return_array);
  if ($imploded_return_array === FALSE) {
    return '';
  }
  return $imploded_return_array;
}

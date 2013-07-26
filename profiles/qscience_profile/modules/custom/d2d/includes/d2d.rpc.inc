<?php
/**
 * @file
 * XML RPC functions to answer remote queries.
 */

/**
 * Implements hook_xmlrpc().
 */
function d2d_xmlrpc() {
  $methods = array();
  $methods[] = array(
    'd2d.getInstanceInformation',
    'd2d_get_instance_information',
    array('string'),
    t('Get Information about this instance.'),
  );
  $methods[] = array(
    'd2d.getPublicKey',
    'd2d_get_public_key',
    array('string', 'string'),
    t('Get public key.'),
  );
  $methods[] = array(
    'd2d.secureRPC',
    'd2d_secure_rpc',
    array('string', 'string', 'string'),
    t('General secure RPC')
  );
  $methods[] = array(
    'd2d.secureRequest',
    'd2d_secure_request',
    array('string', 'string', 'string'),
    t('General secure request.'),
  );
  return $methods;
}
/**
 * Provide public key.
 */
function d2d_get_public_key($d2d_id) {
  if (!d2d_is_online()) {
    return FALSE;
  }
  $my_instance = d2d_api_own_instance_get();
  if ($my_instance['d2d_id'] !== $d2d_id) {
    return xmlrpc_error(1, 'Provided d2d-id does not match.');
  }
  return $my_instance['public_key'];
}
/**
 * Get information about this instance.
 * (in particular this information is used for remote auto-completion
 *  of the add instance form)
 */
function d2d_get_instance_information() {
  if (!d2d_is_online()) {
    return FALSE;
  }
  $instance = d2d_api_own_instance_get();
  $res = array(
    'd2d_id' => $instance['d2d_id'],
    'name' => $instance['name'],
    'description' => $instance['description'],
  );
  return d2d_implode($res);
}

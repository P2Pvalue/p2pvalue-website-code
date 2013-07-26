<?php
/**
 * @file
 * Cryptographic functions.
 */

/**
 * Generate public / private key pair.
 */ 
function d2d_create_keys(&$public_key, &$private_key) {
  $res = @openssl_pkey_new();
  if ($res === FALSE) {
    return FALSE;
  }
  if (!@openssl_pkey_export($res, $private_key)) {
    return FALSE;
  }
  $public_key = @openssl_pkey_get_details($res);
  $public_key = $public_key['key'];
  return TRUE;
}

/**
 * Check if a public / private key pair match.
 */
function d2d_check_key_pair($public_key, $private_key) {
  $data = 'just a simple encryption / decryption test.';
  if (!@openssl_public_encrypt($data, $crypted, $public_key)) {
    return FALSE;
  }
  if (!@openssl_private_decrypt($crypted, $decrypted, $private_key)) {
    return FALSE;
  }
  return $decrypted === $data;
}

/**
 * Sign data.
 */
function d2d_sign($data, $private_key, $signer) {
  if (@openssl_sign($data, $signature, $private_key) === FALSE) {
    return FALSE;
  }
  else {
    $array = array(
      'data' => $data,
      'signature' => $signature,
      'signer' => $signer,
    );
    return d2d_implode($array);
  }
}

/**
 * Sign data but do not include data in return.
 */
function d2d_sign2($data, $private_key, $signer) {
  if (@openssl_sign($data, $signature, $private_key) === FALSE) {
    return FALSE;
  }
  else {
    $array = array(
      'signature' => $signature,
      'signer' => $signer,
    );
    return d2d_implode($array);
  }
}

/**
 * Check signature.
 */
function d2d_verify($signature_string, $public_key, &$signature_array) {
  if (($array = d2d_explode($signature_string)) === FALSE || !d2d_check_array($array, array('data' => 'is_string', 'signature' => 'is_string', 'signer' => 'is_string')) || 
    (@openssl_verify($array['data'], $array['signature'], $public_key) !== 1)) {
    return FALSE;
  }
  $signature_array = $array;
  return TRUE;
}

/**
 * Enrypt data.
 */
function d2d_encrypt($data, &$encrypted_data, $public_key) {
  if (@openssl_seal($data, $sealed_data, $env_keys, array($public_key)) === FALSE) {
    return FALSE;
  }
  $env_key = $env_keys[0];
  $tmp = d2d_implode(array('cipher' => $sealed_data, 'env_key' => $env_key));
  if ($tmp === FALSE) {
    return FALSE;
  }
  else {
    $encrypted_data = $tmp;
    return TRUE;
  }
}

/**
 * Decrypt data.
 */
function d2d_decrypt($encrypted_data, &$data, $private_key) {
  // explode $encrypted_data
  $cipher_envkey = d2d_explode($encrypted_data);
  if ($cipher_envkey === FALSE) {
    return FALSE;
  }
  // check if exploded $encrypted_data is of desired structure
  if (!d2d_check_array($cipher_envkey, array('cipher' => 'is_string', 'env_key' => 'is_string'))) {
    return FALSE;
  }
  if (!openssl_open($cipher_envkey['cipher'], $data, $cipher_envkey['env_key'], $private_key)) {
    return FALSE;
  }
  return TRUE;
}

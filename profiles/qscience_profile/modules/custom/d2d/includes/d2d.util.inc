<?php
/**
 * @file
 * Utility functions used by the D2D module.
 */

/**
 * Replaces all occurrences of the search string with the replacement string.
 *
 * This function is just a wrapper for str_replace that also checks if the
 * given string is really is a string. If $string is an integer, it will be
 * converted to a string before replacing. If it's neither a string nor an
 * integer, FALSE is returned.
 *
 * @param mixed $search the value(s) being searched for 
 * @param mixed $replace the replacement value(s)
 * @param string|int $string the value being searched and replaced on
 * @return string|bool the replaced value or FALSE, if replacement failed
 */
function d2d_replace($search, $replace, $string) {
  if (is_int($string)) {
    $string = strval($string);
  }
  if (is_string($string)) {
    $string = str_replace($search, $replace, $string); 
  }
  else {
    return FALSE;
  }
  return $string;
}

/**
 * Encodes both the type and the value of the argument.
 *
 * If $value === NULL it is encoded as "n$",
 * if $value is a boolean it is encoded as either
 * "b$0" or "b$1". Otherwise, $value is encoded as
 * "s$" . $string, where $string = strval($value).
 *
 * @param mixed $value the value to be encoded
 * @return string the encoded value or FALSE; if encoding failed
 */
function d2d_value_encode($value) {
  if (is_null($value)) {
    $type = 'n';
    $string = '';
  }
  elseif (is_bool($value)) {
    $type = 'b';
    $string = $value ? '1' : '0';
  }
  else {
    $type = 's';
    $string = strval($value);
  }
  return $type . '$' . $string;
}

/**
 * Decodes a string that has been encoded by d2d_value_encode.
 *
 * @see d2d_value_encode
 *
 * @param string $string the string to be decoded
 * @return array an array of two elements; the first value in that array
 *   contains a flag which is TRUE if and only if the decoind succeeded.
 *   On success, the second is the decoded version of $string.
 */
function d2d_value_decode($string) {
  $array = preg_split('/\$/', $string, 2);
  if (sizeof($array) != 2) {
    return array(FALSE, '');
  }
  switch ($array[0]) {
    case 'n': return array(TRUE, NULL);
    case 'b': return array(TRUE, $array[1] ? TRUE : FALSE);
    case 's': return array(TRUE, $array[1]);
  }
  return array(FALSE, '');
}

/**
 * Implodes an array using its key and value pair.
 *
 * Returns FALSE if there is a key or value not of type integer or string. The
 * key-value-pair is separated using a '='-character as a separator, several
 * key-value pairs are separated using ','. Moreover, before concatenating the
 * keys and values, the characters '=', ',' and '\\' (single slash) are
 * prepended with an addtional '\\', i.e. they are escaped to allow unique
 * reconstruction of the provided array from the returned string.
 * Note that a unique reconstruction is only possible for array values being of
 * type string, e.g. d2d_implode('a=-1') will result in array('a'
 * => '-1') and not in array('a' => -1), that is one might have to convert /
 * cast the key to obtain the desired type.
 *
 * @param array $array the array to be imploded
 * @return string|bool the imploded array or FALSE, if implosion failed
 */
function d2d_implode($array) {
  if (!is_array($array)) {
    return FALSE;
  }
  $escaped_array = array();
  foreach ($array as $key => $value) {
    // escape separators and escape-character in $key
    $key = d2d_replace(array('\\', ',', '='), array('\\\\', '\\,', '\\='), $key);
    if ($key === FALSE) { // replacement failed
      return FALSE;
    }
    // encode $value
    $value = d2d_value_encode($value);
    // escape separators and escape-character in $value
    $value = d2d_replace(array('\\', ',', '='), array('\\\\', '\\,', '\\='), $value);
    if ($value === FALSE) { // replacement failed
      return FALSE;
    }
    $escaped_array[] = "{$key}={$value}";
  }
  return implode(',', $escaped_array);
}

/**
 * Explodes a string into an array of key and value pairs.
 *
 * Returns FALSE if the explosion failed, otherwise the array resulting from
 * exploding the given string into key-value-pairs is returned. Note that
 * corresponding keys and values must be separated using '=', while key-value-
 * pairs must be separated using ','. Furthermore, '=' and ',' appearing in
 * keys resp. values must be escaped, see d2d_implode for further details.
 *
 * @param string $string the input string
 * @return array|bool the array resulting form splitting the input, FALSE if explosion failed
 */
function d2d_explode($string) {
  if (!is_string($string)) {
    return FALSE;
  }
  if (empty($string)) {
    return array();
  }
  // prepend two consecutive slashes by an '@' s.t. every slash followed
  // directly by a ',' resp. '=' really corresponds to an escaped character
  $string = str_replace('\\\\', '\\\\@', $string);
  // split array at ','-characters that are not preceded by a slash
  $array = preg_split('/(?<=[^\\\\]),|^,/', $string);
  $return_array = array();
  foreach ($array as $key => $value) {
    // split array at '='-characters that are not preceded by a slash
    $tokens = preg_split('/(?<=[^\\\\])=|^=/', $value);
    // not escaped '=' must occur exactly once, splitting the string into
    // exactly two strings
    if (sizeof($tokens) != 2) {
      return FALSE;
    }
    // undo first replacement for both tokens
    $parsed_key = str_replace('\\\\@', '\\\\', $tokens[0]);
    $parsed_value = str_replace('\\\\@', '\\\\', $tokens[1]);
    // unescape, i.e. remove slashes followed by a single character
    $parsed_key = preg_replace('/\\\\(.)/', '\\1', $parsed_key);
    $parsed_value = preg_replace('/\\\\(.)/', '\\1', $parsed_value);
    list($success, $decoded_parsed_value) = d2d_value_decode($parsed_value);
    if (!$success) {
      return FALSE;
    }
    $return_array[$parsed_key] = $decoded_parsed_value;
  }
  return $return_array;
}

/**
 * Checks values of an array using user defined functions.
 *
 * The check fails if the check for one value of the provided array fails. Note
 * that the check only succeeds if each keys of both provided arrays are the
 * same and appear in the same order. Moreover, the checks may have side-effects
 * on the input array. The input array is first copied before performing the
 * checks and copied back only if returning TRUE, that is the function is free
 * of side-effects when the check fails.
 *
 * @param array &$array key-value-pairs to be checked
 * @param array $checks mapping of keys to user defined functions
 * @return bool TRUE if the check succeeds, FALSE otherwise
 */
function d2d_check_array(&$array, $checks) {
  if (!is_array($array)) {
    return FALSE;
  }
  if (array_keys($array) !== array_keys($checks)) {
    return FALSE;
  }
  $tmp_array = $array;
  foreach ($tmp_array as $key => &$value) {
    if (!call_user_func_array($checks[$key], array(&$value))) {
      return FALSE;
    }
  }
  $array = $tmp_array;
  return TRUE;
}

/**
 * Shortens a public key string to N characters.
 * 
 * Useful to display part of a key in status messages.
 * 
 * Comments are automatically excluded from the N chars.
 * Negative N will start counting from the ending of the key.
 * 
 * @param string $key The public key to shorten for preview
 * @param integer $N Optional. The number of characters to keep. Defaults, -10
 * @return string The shortened key
 * 
 * @see d2d_clean_public_key
 */
function d2d_substr_public_key($key, $N = -10) {
  $key = d2d_clean_public_key($key);

  if ($N < 0) {
    return mb_substr($key, mb_strlen($key) + $N);
  }
  else {
    return mb_substr($key, 0, $N);
  }
  
}

// function d2d_strip_comments_from_key($key, $public = TRUE) {
//   $begin = $public ? '-----BEGIN PUBLIC KEY-----' : '-----BEGIN PRIVATE KEY-----';
//   $end = $public ? '-----END PUBLIC KEY-----' : '-----END PRIVATE KEY-----';
//   $limit = 0;
//   $comment_begin = strpos($key, $begin);
//   if ($comment_begin !== FALSE) {
//     $limit = $keep_comment ? $comment_begin : $comment_begin + 26; //
//     $key = substr($key, $limit);
//   }
  
//   $comment_end = strpos($key, $end);
//   if ($comment_end !== FALSE) {
//     $limit = $keep_comment ? $comment_end + 25 : $comment_end; //
//     $key = substr($key, 0, $limit);
//   }
//   return $key;
// }

function d2d_strip_comments_from_key($key) {
  return preg_replace('!\-\-\-\-\-[^\-]+\-\-\-\-\-!s', '', $key);
//   return preg_replace('!/-/-/-/-/-[^/-]+/-/-/-/-/-/!s', '', $key);
  //return preg_replace('!/\*.*?\*/!s', '', $text);
}

/**
 * Cleans the public key string.
 * 
 * Trim, removes comments and other characters. 
 * 
 * @param string $key The public key to clean
 * @param boolean $keep_comment Optional. Keeps comment about begin/end 
 *  public key. Defaults, FALSE
 * @return string The cleaned key
 */
function d2d_clean_public_key($key, $keep_comment = FALSE) {
  if (!$keep_comment) {
    $key = d2d_strip_comments_from_key($key, TRUE);
  }
  return trim(str_replace(array(" ", "  ", "\n", "\t", "\r"), '', $key));
}

/**
 * Compares two strings containing public keys.
 * 
 * Both keys are cleaned, and then compared
 * 
 * @param string $key1 The first public key to compare
 * @param string $key2 The second public key to compare
 * @return boolean TRUE if the public keys are equal
 * 
 * @see d2d_clean_public_key
 */
function d2d_public_key_equal($key1, $key2) {
  $key1 = d2d_clean_public_key($key1);
  $key2 = d2d_clean_public_key($key2);
  return $key1 === $key2;
}

//////////////////////////////////////////////////////////
// The following tiny functions should be self-describing.
//////////////////////////////////////////////////////////
function d2d_check_convert_int(&$x) {
  $y = intval($x);
  if ($x != strval($y)) {
    return FALSE;
  }
  else {
    $x = $y;
    return TRUE;
  }
}
function d2d_bool_to_int($b) {
  if ($b) {
    return 1;
  }
  return $b === NULL ? NULL : 0;
}
function d2d_random_hex_string($len) {
  $ret = '';
  for ($i = 0; $i < $len; $i++) {
    $n = mt_rand(0, 15);
    if ($n < 10) {
      $ret .= chr($n + ord('0'));
    }
    else {
      $ret .= chr($n - 10 + ord('A'));
    }
  }
  return $ret;
}
function d2d_random_d2d_id() {
  return d2d_random_hex_string(D2D_INSTANCE_IDENTIFIER_LENGTH);
}
function d2d_is_hex_string($string) {
  return preg_match('/^[A-F0-9]*$/', $string) ? TRUE : FALSE;
}
function d2d_check_d2d_id_length($id) {
  return strlen($id) == D2D_INSTANCE_IDENTIFIER_LENGTH;
}
function d2d_check_d2d_id($id) {
  return is_string($id) && d2d_check_d2d_id_length($id) && d2d_is_hex_string($id);
}
function d2d_check_description($str) {
  return is_string($str) && strlen($str) <= D2D_INSTANCE_DESCRIPTION_MAX_LENGTH;
}
function d2d_check_name($str) {
  return is_string($str) && strlen($str) <= D2D_INSTANCE_NAME_MAX_LENGTH;
}
function d2d_check_nonce($nonce) {
  return is_string($nonce) && (strlen($nonce) == D2D_NONCE_LENGTH) && d2d_is_hex_string($nonce);
}
function d2d_random_nonce() {
  return d2d_random_hex_string(D2D_NONCE_LENGTH);
}

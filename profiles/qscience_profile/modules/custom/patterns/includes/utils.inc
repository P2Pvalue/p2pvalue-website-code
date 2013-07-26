<?php
/**
 * @file
 * Collectiion of general purpose functions.
 */


function patterns_utils_is_macro_enabled() {
  $list = module_list();
  return (isset($list["macro"])) ? TRUE : FALSE;
}

/**
 * Checks a pattern identifier (numeric or alphanumeric) and if it not
 * valid displays an error message and redirect the user to the specified page.
 * If the pattern is valid, returns it.
 *
 * If no page is specified redirects to '/admin/patterns/'.
 *
 * @param mixed $pattern the pattern identifier
 * @param mixed $back the page to redirect to (default '/admin/patterns/')
 * @param array $options an associative array of extra options. E.g.
 *  $options('numeric' => TRUE) enforces that the pattern id must be numeric.
 *
 *
 */
function patterns_utils_if_invalid_go_back( $pattern = NULL,
                                            $back = '/admin/patterns/',
                                            $options = array()) {

  $id = $pattern; // store a reference to the original parameter
  if (isset($options['numeric'])) {
    if (!is_numeric($pattern)) {
      drupal_set_message(t('You must specify a valid id for the pattern.'));
      return FALSE;
    }
  }

  $pattern = patterns_get_pattern($pattern);

  if (!$pattern) {
    drupal_set_message(t('No pattern was found with the given id: %id', array('%id' => $id)), 'error');
    drupal_goto($back);
  }

  return $pattern;
}

/**
 * Helper function to get PHPDoc @version tag from a file
 */
function patterns_utils_get_phpdoc_version($path) {
  $version = 'unknown';
  $needle  = '@version ';

  if (file_exists($path)) {
    $fp = @fopen($path, 'r');
    if ($fp) {
      while (!feof($fp)) {
        $occurence = stristr(fgets($fp), $needle);
        if ($occurence) { // FALSE if stristr found nothing
          return rtrim(substr($occurence, strlen($needle)));
        }
      }
    }
  }
  return $version;
}



/**
 * Returns a string representation of a pattern object or array.
 * If an id or the name of the pattern is passed, it tries
 * to load it, and then constructs the name of the pattern.
 *
 * @param mixed $pattern
 */
function patterns_utils_toString($pattern = NULL) {
  $pattern = _patterns_db_get_pattern($pattern);
  if (!$pattern) return FALSE;
  return $pattern->name . ' (' . $pattern->title . ')';
}


/**
 * Generate an array representing a standard info section of a pattern.
 * Default values for empty parameters are passed, e.g. the current user
 * for the author, and author's email and the url of the Drupal web site
 * for the author's web site.
 *
 * @param mixed $category
 * @param mixed $descr
 * @param mixed $v
 * @param mixed $core
 * @param mixed $user
 * @param mixed $base_root
 *
 */
// TODO: t()
function patterns_utils_generate_info_section($title = 'Untitled Pattern',
                                              $category = 'General',
                                              $descr = 'No description',
                                              $v = '1.0',
                                              $core = 'x.y',
                                              $user = NULL,
                                              $base_root = NULL ) {
  if (is_null($user)) global $user;
  if (is_null($base_root)) global $base_root;

  $info = array('info' => array());

  $info['info']['title']          = $title;
  $info['info']['author']         = $user->name;
  $info['info']['category']       = $category;
  $info['info']['description']    = $descr;
  $info['info']['core']           = $core;
  $info['info']['author']         = $user->name;
  $info['info']['author_email']   = $user->mail;
  $info['info']['author_website'] = $base_root;

  return $info;
}

/**
 * Checks if current configuration of php enables open remote files
 * for reading.
 *
 */
function patterns_utils_is_fopenurl_enabled() {
  return ini_get('allow_url_fopen') ? TRUE : FALSE;
}

/**
 * Take as input an array representing the pattern code or the pattern as
 * extracted from the database and looks for the specified key.
 *
 * In the following order returns:
 *
 *  - the $value parameter, if not NULL
 *  - the value of the key if found in the INFO section
 *  - the value of the key if found in the first level of the array
 *  - NULL or $fallback if the key is not found
 *
 *
 * @param mixed $key the key to look for
 * @param array $patterm the array to inspect
 * @param mixed $value a default value for the key
 * @param mixes $fallback to return instead of NULL, if no key is found
 *
 */
function patterns_utils_init_from_pattern($key, $pattern, $value = NULL, $fallback = NULL) {
  if (!is_null($value)) return $value;
  if (isset($pattern['info'])) {
    if (isset($pattern['info'][$key])) {
      return $pattern['info'][$key];
    }
  }
  if (isset($pattern[$key])) {
    return $pattern[$key];
  }
  return $fallback;
}

/**
 * Helper function to find out whether a module is currently installed
 *
 * @param mixed $module String representing the name of the module
 *
 * @return Bool TRUE If the module is currently installed
 */
function patterns_utils_is_module_available($module = NULL) {
  if (is_null($module)) return FALSE;
  $modules = system_rebuild_module_data();
  return (isset($modules[$module])) ? TRUE : FALSE;
}

/**
 * Helper function to find out whether a module is currently enabled
 *
 * @param mixed $module String representing the name of the module
 *
 * @return Bool TRUE If the module is currently enabled
 */
function patterns_utils_is_module_enabled($module = NULL) {
  if (is_null($module)) return FALSE;
  $modules = module_list();
  return isset($modules[$module]) ? TRUE : FALSE;
}

/**
 * Checks whether the public patterns page has been enabled.
 *
 * @return Bool TRUE, if a valid url was specified for the
 *   public patterns page.
 */
function patterns_utils_is_public_page_enabled() {
  return (variable_get('patterns_public_url', '') != '') ? TRUE : FALSE;
}

/**
 * Checks whether the current page is the public patterns page
 *
 * @return Bool TRUE, if the current page is the public patterns page
 */
function patterns_utils_is_public_page() {
  $f = variable_get('site_frontpage', '');
  $p = variable_get(PATTERNS_PUBLIC_URL, '');
  if ($f == $p) {
    return drupal_is_front_page();
  }
  return (request_path() == $p) ? TRUE : FALSE;
}

/**
 * Enhanced version of array_key_exists with support for array of keys
 * 
 * Returns an associative array where to each key is associated the 
 * logical value TRUE if it was found in the test array, FALSE otherwise.
 * 
 * @param mixed|Array $key
 * 	The key or set of keys to test
 * @return Array $out
 * 	Associative array of boolean values for each key
 * 
 * @see array_key_exists
 * @see patterns_utils_check_keys
 */
function patterns_utils_key_exists($key = NULL, $array = array()) {
	$out = array();
	if (!isset($key)) {
		return $out;
	}
  if (!is_array($key)) {
  	$key = array($key);
  }
	if (count($key) === 0) {
		return $out;
	}
	foreach ($key as $k) {
		$out[$k] = array_key_exists($k, $array);
	}
	return $out;
}

/**
 * Powerful tool to validate component input parameters
 * 
 * User can add sets of conditions, that will be evaluated together.
 * A detailed error message is created with all failing conditions.
 * 
 * Supported conditions: 'mandatory', 'at_least_one', 'one_and_only_one', 
 * 'exclusive'. 
 *
 * Conditions must be inserted as a new element of the checkings array,
 * in the form: array('type' => params). E.g.
 *
 * $kchecks = array();
 * $kchecks[] = array('mandatory', array('key1', 'key2', 'key3');
 * $kchecks[] = array('at_least_one', array('key4', 'key5', 'key6');
 * 
 * @param array $data The array to validate
 * @param array $kchecks The array of conditions  
 *
 * @see patterns_utils_key_exists
 */
function patterns_utils_check_keys($data, $kchecks = array(), &$msg, $BR = '<br/>') {

  $msg_mandatory = NULL;
  $msg_init_mandatory = t("The following mandatory keys are missing: ");

  $msg_exclusive = NULL;
  $msg_init_exclusive = t("The following keys are mutually exclusive: ");
  $count_exclusive = NULL;

  $msg_at_least_one = NULL;
  $msg_init_at_least_one = t("At least one key of the following key must be present: ");
  $found_at_least_one = NULL;

  $msg_one_and_only_one = NULL;
  $msg_init_one_and_only_one = t("Exactly one item of each group must be present. Zero, or more than one found: ");
  $count_one_and_only_one = NULL;

  $status = PATTERNS_SUCCESS;

  foreach($kchecks as $check) {
    $type = key($check);
    $set = current($check);
     
    if ($type == 'exclusive') {
       
      $found_exclusive = 0;
      foreach ($set as $k) {
        if (isset($data[$k])) {
          $found_exclusive++;
          if ($found_exclusive > 1) {
            break;
          }
        }
      }
      if ($found_exclusive > 1) {
        $group_exclusive = '(' . implode(', ', $set) . ')';
        if (is_null($msg_exclusive)) {
          $msg_exclusive = $msg_init_exclusive . $group_exclusive;
          $status = PATTERNS_ERR;
        }
        else {
          $msg_exclusive .= ', ' . $group_exclusive;
        }
      }
    }


    if ($type == 'at_least_one') {
       
      $found_at_least_one = FALSE;
      foreach ($set as $k) {
        if (isset($data[$k])) {
          $found_at_least_one = TRUE;
          break;
        }
      }
      if (!$found_at_least_one) {
        $group_at_least_one = '(' . implode(', ', $set) . ')';

        if (is_null($msg_at_least_one)) {
          $msg_at_least_one = $msg_init_at_least_one . $group_at_least_one;
          $status = PATTERNS_ERR;
        }
        else {
          $msg_at_least_one .= ', ' . $group_at_least_one;
        }
      }
    }


    if ($type == 'one_and_only_one') {
       
      $count_one_and_only_one = 0;
      foreach ($set as $k) {
        if (isset($data[$k])) {
          $count_one_and_only_one++;
          if ($count_one_and_only_one > 1) {
            break;
          }
           
        }
      }
      if ($count_one_and_only_one != 1) {
        $group_one_and_only_one = '(' . implode(', ', $set) . ')';

        if (is_null($msg_at_least_one)) {
          $msg_one_and_only_one = $msg_init_one_and_only_one . $group_one_and_only_one;
          $status = PATTERNS_ERR;
        }
        else {
          $msg_one_and_only_one .= ', ' . $group_one_and_only_one;
        }
      }
    }


    if ($type == 'mandatory') {

      foreach ($set as $k) {
        if (!isset($data[$k])) {
          if (is_null($msg_mandatory)) {
            $msg_mandatory = $msg_init_mandatory . $k;
            $status = PATTERNS_ERR;
          }
          else {
            $msg_mandatory .= ', ' . $k;
          }
        }
      }
    }
     
  } // end kcheck

  // update error msg
  if ($status == PATTERNS_ERR) {
    $msg = is_null($msg_mandatory) ? $msg : $msg_mandatory;
    $msg = is_null($msg_one_and_only_one) ? $msg : $msg . (is_null($msg) ? $msg_one_and_only_one : $BR . $msg_one_and_only_one);
    $msg = is_null($msg_at_least_one) ? $msg : $msg . (is_null($msg) ? $msg_at_least_one : $BR . $msg_at_least_one);
    $msg = is_null($msg_exclusive) ? $msg : $msg . (is_null($msg) ? $msg_exclusive : $BR . $msg_exclusive);
  }

  return $status;
}

/**
 * Returns TRUE if patterns is executed via cli
 * 
 * Useful for detecting if patterns is running via drush
 * 
 * @return Boolean TRUE, if the script is executed in cli
 */
function patterns_utils_is_cli() {
	return php_sapi_name() == "cli";
}


/**
 * Generates a Universally Unique IDentifier, version 4.
 *
 * RFC 4122 (http://www.ietf.org/rfc/rfc4122.txt) defines a special type of Globally
 * Unique IDentifiers (GUID), as well as several methods for producing them. One
 * such method, described in section 4.4, is based on truly random or pseudo-random
 * number generators, and is therefore implementable in a language like PHP.
 *
 * We choose to produce pseudo-random numbers with the Mersenne Twister, and to always
 * limit single generated numbers to 16 bits (ie. the decimal value 65535). That is
 * because, even on 32-bit systems, PHP's RAND_MAX will often be the maximum *signed*
 * value, with only the equivalent of 31 significant bits. Producing two 16-bit random
 * numbers to make up a 32-bit one is less efficient, but guarantees that all 32 bits
 * are random.
 *
 * The algorithm for version 4 UUIDs (ie. those based on random number generators)
 * states that all 128 bits separated into the various fields (32 bits, 16 bits, 16 bits,
 * 8 bits and 8 bits, 48 bits) should be random, except : (a) the version number should
 * be the last 4 bits in the 3rd field, and (b) bits 6 and 7 of the 4th field should
 * be 01. We try to conform to that definition as efficiently as possible, generating
 * smaller values where possible, and minimizing the number of base conversions.
 *
 * @copyright   Copyright (c) CFD Labs, 2006. This function may be used freely for
 *              any purpose ; it is distributed without any form of warranty whatsoever.
 * @author      David Holmes <dholmes@cfdsoftware.net>
 *
 * @return  string  A UUID, made up of 32 hex digits and 4 hyphens.
 */
function patterns_utils_get_uuuid() {

    // The field names refer to RFC 4122 section 4.1.2

    return sprintf('%04x%04x-%04x-%03x4-%04x-%04x%04x%04x',
        mt_rand(0, 65535), mt_rand(0, 65535), // 32 bits for "time_low"
        mt_rand(0, 65535), // 16 bits for "time_mid"
        mt_rand(0, 4095),  // 12 bits before the 0100 of (version) 4 for "time_hi_and_version"
        bindec(substr_replace(sprintf('%016b', mt_rand(0, 65535)), '01', 6, 2)),
            // 8 bits, the last two of which (positions 6 and 7) are 01, for "clk_seq_hi_res"
            // (hence, the 2nd hex digit after the 3rd hyphen can only be 1, 5, 9 or d)
            // 8 bits for "clk_seq_low"
        mt_rand(0, 65535), mt_rand(0, 65535), mt_rand(0, 65535) // 48 bits for "node"
    );
}

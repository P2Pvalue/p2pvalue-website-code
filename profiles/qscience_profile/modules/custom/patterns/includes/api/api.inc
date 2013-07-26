<?php

/**
 * @file
 *
 * API for writing pattern files.
 *
 */

/**
 * Returns an array with the reserved words for Patterns files
 *
 * Those words are not allowed to be used as section headings.
 *
 * @return Array
 *   The array of reserved words.
 */
function patterns_api_reserved_words() {
  return array(
    PATTERNS_TAG,
    PATTERNS_SECTION_INFO,
    PATTERNS_SECTION_MODULES,
    PATTERNS_CREATE,
    PATTERNS_MODIFY,
    PATTERNS_DELETE,
    PATTERNS_INCLUDE,
  );
}

/**
 * Returns TRUE if a string is a Patterns reserved word
 *
 * @param mixed $word
 *   The string to check.
 *
 * @return Boolean,
 *   TRUE, if it is a reserved word
 */
function patterns_api_is_reserved_word($word = NULL) {
  if (is_null($word)) return FALSE;
  return (in_array($word, patterns_api_reserved_words())) ? TRUE
                                                          : FALSE;
}

/**
 * Checks whether a string is an existing component name.
 *
 * @param mixed $component
 *   The string with the name of the component
 *
 * @return Boolean
 *   TRUE, if the string is a valid component name
 *
 * @see patterns_api_build_moduletags_index()
 */
function patterns_api_is_valid_component_name($component = NULL) {
  if (is_null($component)) return FALSE;
  $idx = patterns_moduletags_get_index();
  return (isset($idx[$component])) ? TRUE : FALSE;
}


/**
 * Adds / Replaces the 'modules' section in a pattern.
 *
 * Removes duplicates from the list of modules.
 *
 * @param array $modules
 *   Array containing the name of the modules
 * @param array $pattern (optional)
 *   The associative array representing a pattern
 * @param Bool $ow (optional)
 *   If TRUE, a pre-existing modules section will be
 *   overwritten. Defaults FALSE.
 *
 * @return Bool|Array Return the array containing the
 *   the modules section, or FALSE if an error occurred
 */
function patterns_api_add_modules_section($modules = NULL, &$pattern = array(), $ow = FALSE) {
  if (empty($modules)) return FALSE;
  if (isset($pattern['modules']) && !$ow) return FALSE;
  $modules = (is_array($modules)) ? $modules : array($modules);
  $pattern['modules'] = array_unique($modules);
  return $pattern;
}


/**
 * Initializes an empty section in a pattern file.
 *
 * Checks whether the section name is valid before adding it.
 *
 * @param mixed $section_name
 *   A string representing the pattern name
 * @param array $pattern (optional)
 *   The associative array representing a pattern
 * @param Bool $ow (optional)
 *   If TRUE, an existing section with the same name will be
 *   overwritten. Defaults FALSE.
 */
function patterns_api_add_section($section_name = NULL, &$pattern = array(), $ow = FALSE) {
  if (empty($section_name)) return FALSE;
  if (patterns_api_is_reserved_word($section_name)) return FALSE;
  if (isset($pattern[$section_name]) && !$ow) return FALSE;

  $pattern[$section_name] = array();
  return $pattern;
}


/**
 * Adds / Replaces the 'info' section in a pattern
 *
 * Adds automatically the standard values to the missing properties
 *
 * @param array $info (optional)
 *   An associative array of properties for the info section.
 * @param array $pattern (optional)
 *   An array representing a pattern file.
 * @param Bool $ow (optional)
 *   If TRUE, overwrites existing Info sections.
 */
function patterns_api_add_info_section($info = array(), &$pattern = array(), $ow = FALSE) {
  if (isset($pattern[PATTERNS_SECTION_INFO]) && !$ow) return FALSE;

  global $user;
  global $base_root;

  $pattern[PATTERNS_SECTION_INFO] = array();

  $pattern[PATTERNS_SECTION_INFO]['title']          = (isset($info['title'])) ? $info['title'] : 'Untitled Pattern';
  $pattern[PATTERNS_SECTION_INFO]['category']       = (isset($info['category'])) ? $info['category'] : 'General';
  $pattern[PATTERNS_SECTION_INFO]['description']    = (isset($info['description'])) ? $info['description'] : 'No description';
  $pattern[PATTERNS_SECTION_INFO]['version']        = (isset($info['version'])) ? $info['version'] : '1.0';
  $pattern[PATTERNS_SECTION_INFO]['core']           = (isset($info['core'])) ? $info['core'] : 'x.y';
  $pattern[PATTERNS_SECTION_INFO]['author']         = (isset($info['author'])) ? $info['author'] : $user->name;
  $pattern[PATTERNS_SECTION_INFO]['author_email']   = (isset($info['author_email'])) ? $info['author_email'] : $user->mail;
  $pattern[PATTERNS_SECTION_INFO]['author_website'] = (isset($info['author_website'])) ? $info['author_website'] : $base_root;

  return $pattern;
}

/**
 * Adds an action to a section
 * 
 * Checks that the action has the 'tag' key
 * 
 * @param mixed $action
 * 		A valid Patterns action, e.g. PATTERNS_CREATE, PATTERNS_MODIFY, etc.
 * @param Array $data
 * 		The tag of the action and the additional properties
 * @param Array $section Optional
 * 		The containing section
 */
function patterns_api_add_action($action = PATTERNS_CREATE, $data = array(), &$section = array()) {
  if (!isset($data['tag'])) {
  	return FALSE;
  }
	$section[] = array($action => $data);
  return TRUE;
}


/**
 * Helper function to create the associative array of returned values from
 * an invoked function.
 *
 * @param mixed $status One of [PATTERNS_SUCCESS, PATTERNS_WARN, PATTERNS_ERR]. Required.
 * @param array $msg A message.
 * @param mixed $result Any kind of additional data.
 *
 * @return array An associative array of the input values with keys 'status', 'msg', 'result'.
 */
function patterns_results($status = PATTERNS_SUCCESS, $msg = NULL, $result = NULL) {
  return array(
    'status' => $status,
    'msg' => $msg,
    'result' => $result,
  );
}

/**
 * Checks if a (supposed) array is actually correctly formatted as a patterns_results().
 *
 * @param mixed $results The array to check.
 *
 * @return bool TRUE if correct, FALSE otherwise.
 * @see patterns_results()
 */
function _patterns_is_patterns_results($results) {
  // 'status' is the only mandatory field.
  if (!isset($results['status'])) {
    return FALSE;
  }

  return TRUE;
}


/**
 * Returns an array of allowed actions, as defined in patterns.module.
 *
 * @return array Allowed actions.
 */
function patterns_actions() {
  return array( PATTERNS_CREATE => PATTERNS_CREATE,
                PATTERNS_MODIFY => PATTERNS_MODIFY,
                PATTERNS_DELETE => PATTERNS_DELETE,
                PATTERNS_INCLUDE => PATTERNS_INCLUDE,
                );
}

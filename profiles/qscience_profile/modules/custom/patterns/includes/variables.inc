<?php
/**
 * @file
 * A file containing a set of constants and variables.
 *
 */

static $patterns_formats = array();

define('PATTERNS_PUBLIC_URL', 'patterns_public_url');

define('PATTERNS_UPDATES', 'patterns_updates');

// Notice: if you change this value you must update also patterns.test
define('PATTERNS_FIRST_INSTALL', 'patterns_first_install');

define('PATTERNS_PARSER_PARSE', 'parse');
define('PATTERNS_PARSER_DUMP', 'dump');
define('PATTERNS_PARSER_DUMP_COMMENT', 'dump_comment');
define('PATTERNS_PARSER_LOAD', 'load');


define('PATTERNS_EXEC_PHP', 'php');
define('PATTERNS_EXEC_BATCH', 'batch');

define('PATTERNS_FORMAT_UNKNOWN', 'unknown');

define('PATTERNS_SUCCESS', 1);
define('PATTERNS_WARN', 0);
define('PATTERNS_ERR', -1);

define('PATTERNS_TAG', 'tag');
define('PATTERNS_SECTION_INFO', 'info');
define('PATTERNS_SECTION_MODULES', 'modules');

define('PATTERNS_CREATE', 'create');
define('PATTERNS_MODIFY', 'modify');
define('PATTERNS_DELETE', 'delete');
define('PATTERNS_INCLUDE', 'include');

define('PATTERNS_EXPORT', 'export');
define('PATTERNS_EXPORT_ALL', 'All');
define('PATTERNS_EXPORT_CURRENT_THEMES', 'current_theme(s)'); //Export settings related to current theme(s)
define('PATTERNS_EXTRACT_ALL', 'extract_all');

define('PATTERNS_FILES', 'files');
define('PATTERNS_INFO', 'descr');


define('PATTERNS_STATUS_OK', 0);        // installed, not enabled
define('PATTERNS_STATUS_ENABLED', 1);   // installed, enabled
define('PATTERNS_STATUS_INVALID', -1);  // invalid
define('PATTERNS_STATUS_TRASHED', -5);  // removed

define('PATTERNS_INCLUDE_NEVER', 'never');
define('PATTERNS_INCLUDE_FIRSTRUN', 'firstrun');
define('PATTERNS_INCLUDE_UPDATE', 'onupdate');
define('PATTERNS_INCLUDE_FIRSTRUN_OR_UPDATE', 'first_or_up');
define('PATTERNS_INCLUDE_ATTACHED', 'attached');
define('PATTERNS_INCLUDE_ALWAYS', 'always');

define('PATTERNS_EXPORT_TO_FILE', 'file');
define('PATTERNS_EXPORT_TO_DB', 'db');
define('PATTERNS_EXPORT_TO_ZIP', 'zip');

define('PATTERNS_EXPORT_MODE_FULL', 'full');
define('PATTERNS_EXPORT_MODE_ACTIONS', 'actions');
define('PATTERNS_EXPORT_MODE_ID', 'id');
define('PATTERNS_EXPORT_MODE_NAME', 'name');



define('PATTERNS_VALIDATE_ALL', 10);        // Pattern has valid content. Currently this is the same level as PATTERNS_VALIDATE_SEMANTIC
define('PATTERNS_VALIDATE_SEMANTIC', 8);    // This level includes the semantic errors/warnings raised by the components
define('PATTERNS_VALIDATE_TAG_SYNTAX', 7);  //This level includes the syntactic errors/warnings raised by the components.
define('PATTERNS_VALIDATE_TAG_EXISTS', 5);	// tags exist
define('PATTERNS_VALIDATE_INCLUDE', 3);		// syntax check for included patterns
define('PATTERNS_VALIDATE_SYNTAX', 2);		// basic syntax is correct (info, modules, and 1 section)
define('PATTERNS_VALIDATE_FORMAT', 1);		// can be parsed
define('PATTERNS_VALIDATE_SKIP', 0);        // skip validation

//Semantic warnings
define('PATTERNS_WARNING_ALREADY_DEFINED_ELEMENT', -1); //This refers to the problem of creating an element that it is already present in the system.
define('PATTERNS_WARNING_ELEMENT_UNDEFINED', -2); //This refers to the problem of trying to perform changes in an element that is not present in the system yet.
define('PATTERNS_WARNING_UNMET_DEPENDENCY', -3); //This refers to the problem of creating an element that depends on the creation of another element whose creation belongs to another component (or another tag of the same component)
define('PATTERNS_WARNING_REMAINING_DEPENDENCY', -4); //This refers to the problem of deleting a part of the configuration that affects elements defined by other components.
define('PATTERNS_WARNING_NOT_UNIQUE_ALIAS', -5); //This refers to the problem of modifying or delete an element that there are more than one meeting the requirement existing on the system (not unique).
define('PATTERNS_WARNING_INCONSISTENT_OPERATION', -6); //This refers to the problem of trying to apply an action to an element not supported by Drupal core

// Type checking semantic warnings
define('PATTERNS_WARNING_TYPE_ERROR', -7); // This refers to an element being of an unexpected type (e.g. string rather than int)
define('PATTERNS_WARNING_OUT_OF_RANGE_ELEMENT', -8); // This refers to an element of the expected type, but out of range (e.g. 100, but max is 10)
define('PATTERNS_WARNING_EMPTY_ELEMENT', -9); // This refers to the value of the key being empty, when that is not allowed
define('PATTERNS_WARNING_GENERIC_VALIDATION_ERROR', -100); // All that is not coverered by previous cases


define('PATTERNS_ALLOW_PUBLISH_DEFAULT', TRUE);

define('PATTERNS_FILES_DIR_DEFAULT','sites/default/files/patterns');


<?php
/**
 * @file
 * Patterns forms.
 */

module_load_include('inc', 'patterns', 'includes/forms/lab');
module_load_include('inc', 'patterns', 'includes/forms/quickrun');
module_load_include('inc', 'patterns', 'includes/forms/editor');
module_load_include('inc', 'patterns', 'includes/forms/import');
module_load_include('inc', 'patterns', 'includes/forms/helper');
module_load_include('inc', 'patterns', 'includes/forms/info');


function patterns_forms_add_page_header(&$form, $title, $text = '') {
    $form['header'] = array(
    '#markup' => t($text),
    '#prefix' => '<div id="intro_export"><strong>' . t($title) . ': </strong>',
    '#suffix' => '<hr><br/></div>',
  );
}

/**
 * Builds up a fieldset containing the options for executing a
 * pattern.
 *
 * @param array $form An array element representing a form. If specified,
 *   the execution options are appended here.
 *
 * @return array $form The form containing the pattern execution options.
 *
 */
function patterns_forms_get_execution_options($form = array()) {

  $form['options'] = array(
      '#type' => 'fieldset',
      '#title' => t('Options'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
  );

  $options_mode = array(
    'extend' => t('Extend'),
    'runover' => t('Run-Over'),
  );

  $form['options']['execution'] = array(
    '#type' => 'radios',
    '#title' => t('Execution mode:'),
    '#description' => t('Extend mode can only add new feature to your web-site, Run-over can also delete. At the moment \'Extend\' is the only mode permitted.'),
    '#options' => $options_mode,
    '#default_value' => 'extend',
    '#disabled' => 'true',
  );

  patterns_forms_get_exec_mode_selector($form['options']);

  $options = array( PATTERNS_INCLUDE_NEVER => t('Never run'),
                    PATTERNS_INCLUDE_FIRSTRUN => t('Only during the first run'),
                    PATTERNS_INCLUDE_UPDATE => t('Only if pattern was updated'),
                    PATTERNS_INCLUDE_FIRSTRUN_OR_UPDATE => t('If it is the first run or if the pattern was updated'),
                    PATTERNS_INCLUDE_ATTACHED => t('Only if the pattern code is attached (no lookup to database)'),
                    PATTERNS_INCLUDE_ALWAYS => t('Always'),
                  );

  $form['options']['run-subpatterns'] = array(
    '#type' => 'radios',
    '#title' => t('Run sub-patterns:'),
    '#description' => t("Decide when to run sub-patterns that are called by the currently run pattern. If unsure, stick to recommended setting. Note that your choice won't have any effect if your pattern doesn't contain sub-patterns or if this setting has been defined within the pattern file itself."),
    '#options' => $options,
    '#default_value' => patterns_get_include_mode(),
  );

  return $form;
}

/**
 * Creates the radio buttons to select the pattern exec mode
 *
 * @param array $form An array element representing a form. If specified,
 *   the execution options are appended here.
 *   
 * @TODO: doc
 *
 * @return array $form The form containing the pattern execution options.
 *
 */
function patterns_forms_get_exec_mode_selector(&$form = array(), $default = NULL, $title = NULL, $descr = NULL) {
	$options_run_mode = array(
    PATTERNS_EXEC_BATCH => t('Batch'),
    PATTERNS_EXEC_PHP => t('PHP'),
  );
	
  $default = empty($default) ? variable_get('patterns_default_running_mode', PATTERNS_EXEC_PHP) : $default; 
  $title = empty($title) ? t('Running mode:') : $title;
  $descr = empty($descr) ? t('Batch mode uses the Batch API provided by Drupal thus providing intermediate feedback about the progress. PHP will only return when every action has been processed (or in case of error).')
  											 : $decr;
  
  $form['mode'] = array(
    '#type' => 'radios',
    '#title' => $title,
    '#description' => $descr,
    '#options' => $options_run_mode,
    '#default_value' => $default,
  );
  
  return $form;
}

/**
 * Builds a HTML select element with all the currently available
 * patterns formats.
 *
 * @param array $form (optional) If specified, the select form is appended
 *   here. Default array().
 * @param mixed $default (optional) The default value for the selector.
 *   Default PATTERNS_FORMAT_UNKNOWN.
 * @param bool $unknown (optional) If TRUE PATTERNS_FORMAT_UNKNOWN is added
 *   to the 'available' formats. Default TRUE.
 *
 * @return array $form The form containing the select element.
 *
 */
function patterns_forms_get_formats_selector(&$form = array(), $default = PATTERNS_FORMAT_UNKNOWN, $descr = NULL, $unknown = TRUE) {

  $descr = (is_null($descr)) ? t('Pattern will be validated and run against this format.') : t($descr);
  $formats = ($unknown) ? array(PATTERNS_FORMAT_UNKNOWN => PATTERNS_FORMAT_UNKNOWN)
                        : array();
  $available_formats = patterns_parser_get_formats();
  if (!empty($available_formats)) {
    $formats = array_merge($formats, array_combine($available_formats, $available_formats));
  }

  $form['format'] = array(
    '#type' => 'select',
    '#title' => t('Pattern syntax'),
    '#options' => $formats,
    '#default_value' => $default,
    '#description' => $descr,
  );

  return $form;
}

/**
 * Builds a HTML select element with all the currently available
 * patterns validation levels.
 *
 * @param array $form (optional) If specified, the select form is appended here.
 * @param mixed $default (optional) The default value for the selector.
 *
 * @return array $form The form containing the select element.
 *
 */
function patterns_forms_get_validation_level_selector(&$form = array(), $default = PATTERNS_VALIDATE_SYNTAX) {

  $available_formats = array(
    PATTERNS_VALIDATE_TAG_SYNTAX => t('Syntax only'),
    PATTERNS_VALIDATE_SEMANTIC => t('Semantic and Syntax'),
  );
  
  $form['validation'] = array(
    '#type' => 'select',
    '#title' => t('Validation Level'),
    '#options' => $available_formats,
    '#description' => t('Choose the level of validation. Patterns validated semantically will run without errors,
      but they only need to be syntactically valid in order to be run.'),
  );
  return $form;
}

/**
 * Returns a default format, according to the
 * available parsers. In the following order returns:
 * YAML, XML, PHP, ''.
 *
 * @return mixed String A default value for a form selector
 */
function patterns_forms_get_default_format() {

  if (defined('PATTERNS_FORMAT_YAML')) {
    return PATTERNS_FORMAT_YAML;
  }
  if (defined('PATTERNS_FORMAT_XML')) {
    return PATTERNS_FORMAT_XML;
  }
  if (defined('PATTERNS_FORMAT_PHP')) {
    return PATTERNS_FORMAT_PHP;
  }

  return '';
}

function patterns_forms_parser_ready($form = array(), $parser = NULL) {
  if (!patterns_parser_ready()) {
    $forms['noparser'] = array(
      '#markup' => t('Warning: no valid parser found. Please enable some of the from the interface'),
    );
  }
}


function patterns_forms_get_info_section(&$form = array(), $info = array(), $options = array()) {
  $info = array_merge($info, patterns_api_add_info_section($info));
  $collapsed = (isset($options['collapsed']) && $options['collapsed'] == TRUE) ? TRUE : FALSE;
  $form['info'] = array(
    '#type' => 'fieldset',
    '#title' => 'Info Section',
    '#collapsible' => TRUE,
    '#collapsed' => $collapsed,
    '#tree' => TRUE,
  );

  foreach ($info['info'] as $i => $value) {
    $form['info'][$i] = array(
      '#type' => 'textfield',
      '#title' => t($i),
      '#required' => TRUE,
      '#default_value' => $value,
    );
  }

  return $form;
}


function patterns_forms_get_name_selector(&$form = array(), $default = NULL, $options = array()) {

  $form['pattern_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name of the pattern file'),
    '#default_value' => t('pattern') . '_' . strval(time()),
  );

  return $form;
}


function patterns_forms_get_export_to_selector(&$form = array(), $options = array()) {
  $title = (isset($options['title'])) ? $options['title'] : 'Export to';
  $default = (isset($options['default'])) ? $options['default'] : PATTERNS_EXPORT_TO_DB;
  //$collapsed = (isset($options['collapsed'])) ? $options['collapsed'] : TRUE;
  $form['export'] = array(
    '#type' => 'fieldset',
    '#title' => $title,
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#tree' => TRUE,
  );

  $form['export']['type'] = array(
      '#type' => 'radios',
      '#options' => _patterns_lab_get_exp_type_options(),
      '#title' => t('Type of export'),
      '#default_value' => PATTERNS_MODIFY,
  );
  
  $form['export']['to'] = array(
    '#type' => 'radios',
    '#options' => _patterns_lab_get_exp_to_options(),
    '#title' => t('Action'),
    '#default_value' => $default,
  );

  return $form;
}

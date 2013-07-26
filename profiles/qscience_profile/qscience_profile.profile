<?php
/**
 * @file
 * Provides an option to select patterns to be executed during the site installation.
 *
 * Issues:
 *  - Prepare request to have codemirror in the whitelist: http://drupal.org/node/1945806
 * 
 * @TO-DO
 *  - Disable reporting of projects that are in the distribution, but only
 *    if they have not been updated manually.
 *    See http://drupalcode.org/project/commerce_kickstart.git/blob/HEAD:/commerce_kickstart.profile (l.171)
 *    
 */

/**
 * Implements hook_install_tasks().
 *
 */
function qscience_profile_install_tasks() {
  $tasks = array();

  // Add a page to send the public key to the PDF server
  $tasks['qscience_profile_pdfparser_settings_form'] = array(
      'display_name' => st('PDF Parser'),
      'type' => 'form',
  );
  
  // Add a page to allow the user to configure the QTR algorithm
  $tasks['qscience_profile_qtr_settings_form'] = array(
      'display_name' => st('Quality, Trust and Reputation'),
      'type' => 'form',
  );
  
  // Add a page to configure D2D
  $tasks['qscience_profile_d2d_settings_form'] = array(
      'display_name' => st('Drupal to Drupal'),
      'type' => 'form',
  );
  
  // Add a page to share the configuration in a Pattern
  $tasks['qscience_profile_patterns_share_form'] = array(
      'display_name' => st('Share my configuration'),
      'type' => 'form',
  );
  
  return $tasks;
}

/**
 * Implements hook_install_tasks_alter().
 */
function qscience_profile_install_tasks_alter(&$tasks, $install_state) {
  $tasks['install_finished']['function'] = 'qscience_profile_install_finished';
  $tasks['install_select_profile']['display'] = FALSE;

  // The "Welcome" screen needs to come after the first two steps
  // (profile and language selection), despite the fact that they are disabled.
  $new_task['install_welcome'] = array(
      'display' => TRUE,
      'display_name' => st('Welcome'),
      'type' => 'form',
      'run' => isset($install_state['parameters']['welcome']) ? INSTALL_TASK_SKIP : INSTALL_TASK_RUN_IF_REACHED,
  );
  $old_tasks = $tasks;
  $tasks = array_slice($old_tasks, 0, 1) + $new_task + array_slice($old_tasks, 1);

  // Set the installation theme.
  _commerce_kickstart_set_theme('commerce_kickstart_admin');
  // Test if the current task is a batch task.
  if (isset($tasks[$install_state['active_task']]['type']) && $tasks[$install_state['active_task']]['type'] == 'batch') {
    if (!isset($tasks[$install_state['active_task']]['dfp_settings'])) {
      $tasks[$install_state['active_task']]['dfp_settings'] = array();
    }
    // Default to Kickstart_Install dfp unit.
    $tasks[$install_state['active_task']]['dfp_settings'] += array(
        'dfp_unit' => 'Kickstart_Install',
    );
    drupal_add_js('//www.googletagservices.com/tag/js/gpt.js');
    drupal_add_js(array(
    'dfp' => $tasks[$install_state['active_task']]['dfp_settings'],
    ), 'setting');
    drupal_add_js(drupal_get_path('profile', 'commerce_kickstart') . '/js/commerce_kickstart.js');
  }
}

/**
 * Force-set a theme at any point during the execution of the request.
 *
 * Drupal doesn't give us the option to set the theme during the installation
 * process and forces enable the maintenance theme too early in the request
 * for us to modify it in a clean way.
 */
function _commerce_kickstart_set_theme($target_theme) {
  if ($GLOBALS['theme'] != $target_theme) {
    unset($GLOBALS['theme']);

    drupal_static_reset();
    $GLOBALS['conf']['maintenance_theme'] = $target_theme;
    _drupal_maintenance_theme();
  }
}

/**
 * Task callback: shows the welcome screen.
 */
function install_welcome($form, &$form_state, &$install_state) {
  drupal_set_title(st('Welcome to QScience'));
  $qlectives_link = l("QLectives", "http://www.qlectives.eu", array('attributes' => array('target' => '_blank')));
  $qscience_link = l("QScience", "http://qscience.inn.ac", array('attributes' => array('target' => '_blank')));
  
  $message = '<p>' . st('Thank you for choosing !qscience, a distribution developed as part of the project !qlectives.', array('!qlectives' => $qlectives_link, '!qscience' => $qscience_link)) . '</p>';
  $eula = '<p>' . st('QScience is a free, open source, distributed platform tailored to support the needs of modern scholarly communities. QScience offers a free, open source, web 2.0 venue for scientists to meet and discuss about science. Display of ratings of articles, users reputation and indexes of scholarly productivity are among the supported features, however what really distinguish QScience from other analogous software and web site are the following principles:') . '</p>';
  $items = array();
  $items[] = st('Reclaim your data: No sign-up, no central authority. Get QScience, install it, be online. Share what you want with whom you want.');
  $items[] = st('Networked Mind: A QScience instance is never isolated, but rather belongs to a network of relevant communities, among which data can be searched for, shared, and exchanged in a totally secure way with strong cryptography techniques.');
  $items[] = st('Customization in Evolution: QScience is fully customizable to fit different purposes, e.g. a scholarly discussion forum, a research group home page or a research project web site. When you personalize your own QScience instance, you generate a modified version of QScience, that automatically becomes available to the world for download. In this way QScience keeps updated to the needs of the users.');
  $eula .= theme('item_list', array('items' => $items));
  $eula .= '<p>' . st('!qlectives is supported by the European Commission 7th Framework Programme (FP7) for Research and Technological Development under the Information and Communication Technologies Theme, Future and Emerging Technologies (FET) Proactive, Call 3: ICT-2007.8.4 Science of Complex Systems for socially intelligent ICT (COSI-ICT).', array('!qlectives' => $qlectives_link)) . '</p>';
  $form = array();
  $form['welcome_message'] = array(
      '#markup' => $message,
  );
  $form['eula'] = array(
      '#prefix' => '<div id="eula-installation-welcome">',
      '#markup' => $eula,
  );
  $form['eula-accept'] = array(
      '#title' => st('I would like to install a QScience instance'),
      '#type' => 'checkbox',
      '#suffix' => '</div>',
  );
  $form['actions'] = array(
      '#type' => 'actions',
  );
  $form['actions']['submit'] = array(
      '#type' => 'submit',
      '#value' => st("Let's Get Started!"),
      '#states' => array(
          'disabled' => array(
              ':input[name="eula-accept"]' => array('checked' => FALSE),
          ),
      ),
      '#weight' => 10,
  );
  return $form;
}

function install_welcome_submit($form, &$form_state) {
  global $install_state;

  $install_state['parameters']['welcome'] = 'done';
}

/**
 * Implements hook_form_alter().
 *
 * Allows the profile to alter the site configuration form.
 */
function qscience_profile_form_install_configure_form_alter(&$form, $form_state) {
  // Set a default name for the site
  $form['site_information']['site_name']['#default_value'] = st('My QScience web site');

  // Get public IP and query geoplugin to obtain the country code of the server 
  $external_ip = file_get_contents('http://phihag.de/ip/');
  $ip_data = @json_decode(file_get_contents("http://www.geoplugin.net/json.gp?ip=" . $external_ip));  
  if($ip_data && $ip_data->geoplugin_countryCode != null)
  {
    $form['server_settings']['site_default_country']['#default_value'] = $ip_data->geoplugin_countryCode;
  }

  // Hide Update Notifications.
  $form['update_notifications']['#access'] = FALSE;
}

/**
 * Implements qscience_profile_pdfparser_settings_form().
 */
function qscience_profile_pdfparser_settings_form($form, &$form_state, &$install_state) {
  drupal_set_title(st('PDF Parser'));
  $pdfparser_szeged_link = l("University of Szeged", "http://www.inf.u-szeged.hu/rgai/?lang=en", array('attributes' => array('target' => '_blank')));
  $pdfparser_instructions_link = l("instructions", "https://github.com/QScience/pdfparser-server/blob/master/README", array('attributes' => array('target' => '_blank')));

  $form['pdfparser_enable'] = array(
      '#title' => st('Enable PDF Parser module'),
      '#type' => 'checkbox',
      '#default_value' => TRUE,
      '#description' => st('PDF Parser allows you to upload and parse automatically papers\' authors and references.'),
  );
  $form['server_url'] = array(
      '#type' => 'textfield',
      '#title' => 'Server URL',
      '#value' => variable_get('pdfparser_server_url', 'http://www.inf.u-szeged.hu/rgai/~kojak/parser_server/'),
      '#description' => st('You can use our PDF Parser server (hosted at the !szeged) or follow these !instructions to set up your own server. Your public key will be sent to the server.', 
        array('!szeged' => $pdfparser_szeged_link, '!instructions' => $pdfparser_instructions_link)),
      '#states' => array(
          'disabled' => array(
              ':input[name="pdfparser_enable"]' => array('checked' => FALSE),
          ),
      ),
  );
  
  $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Save and continue'),
  );
  return $form;
}

/**
 * Implements qscience_profile_pdfparser_settings_form_validate().
 * Checks that the URL is a valid one
 */
function qscience_profile_pdfparser_settings_form_validate($form, &$form_state) {
  // Check URL only if the module is going to be enabled
  if ($form_state['values']['pdfparser_enable']) {
    $file_headers = @get_headers($form_state['values']['server_url']);
    if ($file_headers === FALSE || $file_headers[0] == 'HTTP/1.1 404 Not Found') {
      form_set_error('server_url', t('Invalid url, please check again.'));
    }
  }
}

/**
 * Implements qscience_profile_pdfparser_settings_form_submit().
 * Merges and executes the selected Patterns.
 */
function qscience_profile_pdfparser_settings_form_submit($form, &$form_state) {
  // Enable PDF module if selected in the task
  if ($form_state['values']['pdfparser_enable']) {
    if (module_enable(array('pdfparser'), TRUE)) {
      $public = _pdfparser_get_public_path();
      $url = $form_state['values']['server_url'];
      variable_set('pdfparser_server_url', $url);
      $file = $public . variable_get('pdfparser_public_key_path') . 'public_key';
      $post = new stdClass();
      $post->func = 'set_public_key';
      $post->ip = $_SERVER['SERVER_ADDR'];
      $post->public_key = base64_encode(file_get_contents($file));
      $ret_pure = do_post_request2($url, json_encode($post));
    }
  }
}

/**
 * Implements qscience_profile_qtr_settings_form().
 */
function qscience_profile_qtr_settings_form($form, &$form_state, &$install_state) {
  drupal_set_title(st('Configure QTR algorithm'));
  $qlectives_link = l("Measuring quality, reputation and trust in online communities", "http://arxiv.org/abs/1208.4042", array('attributes' => array('target' => '_blank')));
  
  $posttype = array ();
  foreach (node_type_get_types() as $type => $type_obj) {
    $posttype[$type] = $type_obj->name;
  }
  //Set content types to be used by QTR by default
  $posttype_default = array('paper');
  
  $form['qtr_message'] = array(
      '#markup' => st('Actions performed on those nodes will be included in the calculations of the QTR index. 
          E.g.: an article with many views will be considered of a high quality, and will generate a high 
          reputation for its author. Further information about the QTR algorith can be found in the following paper: !qtr_paper', 
          array('!qtr_paper' => $qlectives_link)),
  );
  $form['basic'] = array(
      '#type' => 'fieldset',
      '#title' => st('Basic configuration'),
      '#collapsible' => FALSE,
      '#collapsed' => FALSE,
  );
  $form['params'] = array(
      '#type' => 'fieldset',
      '#title' => st('Advanced configuration'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
  );
  $form['basic']['content_type'] = array(
      '#type' => 'checkboxes',
      '#title' => st('Content Types '),
      '#description' => st('These will be the nodes that will be considered for QTR'),
      '#options' => $posttype,
      '#default_value' => $posttype_default,
  );
  $form['params']['qtr_delta'] = array(
      '#type' => 'textfield',
      '#title' => t('Delta'),
      '#description' => st('N.B.: if the effective number of AGENTS/ITEMS is lower (i.e. if there are gaps in the input file), the renormalization of the algorithm has to change. Better to have no gaps!'),
      '#default_value' => variable_get('qtr_delta', 0.00000000001),
  );
  
  $actions = qtr_get_actiontype();
  if ($actions) {
    foreach ($actions as $action) {
      $form['params']['qtr_w_' . $action->action] = array(
          '#type' => 'textfield',
          '#title' => st('Weight of %action action', array('%action' => $action->action)),
          '#default_value' => $action->weight,
      );
    }
  }
  $form['params']['qtr_decay'] = array(
      '#type' => 'select',
      '#title' => st('Time-decay of scores'),
      '#options' => array(0 => 'no decay', 1 => 'power-decay', 2 => 'exponential decay', 3 => 'theta-decay'),
      '#default_value' => variable_get('qtr_decay', 0),
  );
  
  $form['params']['qtr_tau0'] = array(
      '#type' => 'textfield',
      '#title' => st('Time scale of the decay (day)'),
      '#default_value' => variable_get('qtr_tau0', 1),
  );
  
  $form['params']['qtr_renorm_q'] = array(
      '#type' => 'textfield',
      '#title' => st('Renormalization of quality'),
      '#default_value' => variable_get('qtr_renorm_q', 0),
  );
  
  $form['params']['qtr_renorm_r'] = array(
      '#type' => 'textfield',
      '#title' => st('Renormalization of reputation'),
      '#default_value' => variable_get('qtr_renorm_r', 1),
  );
  
  $form['params']['qtr_renorm_t'] = array(
      '#type' => 'textfield',
      '#title' => st('Renormalization of trust'),
      '#default_value' => variable_get('qtr_renorm_t', 0),
  );
  
  $form['params']['qtr_resc_q'] = array(
      '#type' => 'textfield',
      '#title' => st('Rescaled quality'),
      '#default_value' => variable_get('qtr_resc_q', 1),
  );
  
  $form['params']['qtr_resc_r'] = array(
      '#type' => 'textfield',
      '#title' => st('Rescaled reputation'),
      '#default_value' => variable_get('qtr_resc_r', 0),
  );
  
  $form['params']['qtr_resc_t'] = array(
      '#type' => 'textfield',
      '#title' => st('Rescaled trust'),
      '#default_value' => variable_get('qtr_resc_t', 0),
  );
  
  $form['submit'] = array(
      '#type' => 'submit',
      '#value' => st('Save and continue'),
  );
  return $form;
}

/**
 * Implements qscience_profile_qtr_settings_form_validate().
 * Checks that a successful response from the PDF Parser server is received.
 * 
 * @TO-DO: Ask for the ranges of the expected values
 */
function qscience_profile_qtr_settings_form_validate($form, &$form_state) {

}

/**
 * Implements qscience_profile_qtr_settings_form_submit().
 * Merges and executes the selected Patterns.
 */
function qscience_profile_qtr_settings_form_submit($form, &$form_state) {
  //Process the basic settings
  $result = array();
  $types = $form_state['values']['content_type'];
  foreach ($types as $type) {
    if ($type)
      $result[] = array('item_type' => $type);
  }
  qtr_update_itemtype($result);

  //Process the advanced settings
  $params = $form_state['values'];
  variable_set('qtr_delta', $params['qtr_delta']);
  variable_set('qtr_decay', $params['qtr_decay']);
  variable_set('qtr_tau0', $params['qtr_tau0']);
  variable_set('qtr_renorm_q', $params['qtr_renorm_q']);
  variable_set('qtr_renorm_r', $params['qtr_renorm_r']);
  variable_set('qtr_renorm_t', $params['qtr_renorm_t']);
  variable_set('qtr_resc_q', $params['qtr_resc_q']);
  variable_set('qtr_resc_r', $params['qtr_resc_t']);
  variable_set('qtr_resc_t', $params['qtr_resc_t']);
  $actions = qtr_get_actiontype();
  if ($actions) {
    foreach ($actions as $action) {
      qtr_update_actionweight($action->action, $params['qtr_w_' . $action->action]);
    }
  }
}


/**
 * Implements qscience_profile_d2d_settings_form().
 *
 *  @TODO: Ask if we need to simplify this form somehow.
 */
function qscience_profile_d2d_settings_form($form, &$form_state, &$install_state) {
  drupal_set_title(st('Drupal To Drupal settings'));
  
  $form = array();

  $form['introduction'] = array(
      '#markup' => t('D2D ("Drupal-to-Drupal") is a module to built-up a network among Drupal instances.
          Using public key cryptography, an instance can implement secure XML-RPC methods that can be called by friend instances.
          Friends are organized in groups to allow privileged access to particular methods.', array('@length' => D2D_INSTANCE_IDENTIFIER_LENGTH)),
  );
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#description' => t('A short name describing your instance.'),
    '#default_value' => _d2d_suggest_instance_name($GLOBALS['base_url']),
    '#size' => D2D_INSTANCE_NAME_MAX_LENGTH,
    '#maxlength' => D2D_INSTANCE_NAME_MAX_LENGTH,
    '#required' => TRUE,
  );
  $form['id'] = array(
    '#type' => 'textfield',
    '#title' => t('D2D Identifier'),
    '#description' => t('This identifier should be unique among all installations of D2D. 
        It is recommended to generate that identifier randomly (e.g. by using the button below). If you installed D2D before, you can reuse the 
        identifier of your old installation.', array('@length' => D2D_INSTANCE_IDENTIFIER_LENGTH)),
    '#default_value' => d2d_random_d2d_id(),
    '#size' => D2D_INSTANCE_IDENTIFIER_LENGTH,
    '#maxlength' => D2D_INSTANCE_IDENTIFIER_LENGTH,
    '#required' => TRUE, // length is checked anyway, no error if 'generate'-button is pressed
  );
  $form['address'] = array(
    '#type' => 'textfield',
    '#title' => t('Address'),
    '#description' => t('The address under which this instance is reachable.'),
    '#default_value' => $GLOBALS['base_url'] . '/xmlrpc.php',
    '#size' => 40,
    '#maxlength' => D2D_INSTANCE_URL_MAX_LENGTH,
    '#required' => TRUE,
  );
  $form['auto_keys_and_online'] = array(
    '#type' => 'checkbox',
    '#default_value' => TRUE,
    '#title' => t('Automatically select public / private key pair and go online.'),
    '#description' => t('If selected, a random public / private key pair is automatically chosen and the instance will be marked as online, i.e. other instances will be able to see this instance and to communicate with this instance. Do not select this option if you want to manually set your public / private key pair, e.g. to reuse keys you have used with an old installation or if you do not want your instance to be online immediatelly.'),
  );
  $form['submit'] = array(
    '#type' => 'submit', 
    '#value' => t('Save and continue'),
  );
  return $form;
}

/**
 * Implements qscience_profile_d2d_settings_form_validate().
 *
 * @TO-DO: Ask if the validation should be simplified
 */
function qscience_profile_d2d_settings_form_validate($form, &$form_state) {
  $id = $form_state['values']['id'];
  if (!d2d_check_d2d_id_length($id)) {
    form_set_error('id', t('Identifier must constist of exactly @length characters.', array('@length' => D2D_INSTANCE_IDENTIFIER_LENGTH)));
  }
  if (!d2d_is_hex_string($id)) {
    form_set_error('id', t('Identifier must consists only of hexadecimal characters (A-F, 0-9).'));
  }
  if (!d2d_check_url($form_state['values']['address'])) {
    form_set_error('address', t('Address must start with \'http://\' or \'https://\'.'));
  }
}

/**
 * Implements qscience_profile_d2d_settings_form_submit().
 */
function qscience_profile_d2d_settings_form_submit($form, &$form_state) {
 // we create the key pair first to return on error without mofifying
  // the database
  if ($form_state['values']['auto_keys_and_online']) {
    if (!d2d_create_keys($my_public_key, $my_private_key)) {
      drupal_set_message(t('Failed creating public / private key pair.'), 'error');
      return;
    }
  }
  $my_d2d_id = $form_state['values']['id'];
  // add own instance to database
  $instance = array(
    'name' => $form_state['values']['name'],
    'url' => $GLOBALS['base_url'] . '/xmlrpc.php',
    'd2d_id' => $my_d2d_id,
    'description' => 'this instance.',
  );
  $my_id = d2d_api_instance_add($instance, FALSE);
  d2d_api_own_instance_id_set($my_id);
  d2d_api_own_d2d_id_set($my_d2d_id);
  if ($form_state['values']['auto_keys_and_online']) {
    $my_public_key_id = d2d_api_own_public_key_set($my_public_key);
    d2d_api_own_private_key_set($my_private_key);
    variable_set('d2d_online', TRUE);
  }
  $content = d2d_implode(array(
    'subject' => t('Welcome to D2D!'),
    'text' => t('As a first step you might want to add other Drupal installations running D2D to your list of instances and send them a friendship request. Have fun!'),
  ));
  d2d_api_notification_insert('d2d_message', $my_d2d_id, $content);
}

/**
 * Implements qscience_profile_patterns_settings_form().
 */
function qscience_profile_patterns_settings_form($form, &$form_state, &$install_state) {
  // Set the install_profile variable employed by the function manually
  variable_set('install_profile', 'qscience_profile');
  $patterns = _patterns_io_get_patterns();
  // Set the status manually
  $patterns = $patterns[PATTERNS_STATUS_OK];
  // Display some example patterns to run
  $options = array();
  foreach($patterns as $pattern) {
    $options[$pattern->name] = $pattern->title .'<div class="description">'. $pattern->description .'</div>';
  }
  $form['patterns'] = array(
    '#type' => 'checkboxes',
    '#title' => st('Please select the Content Types you would like to install in your site'),
    '#description' => st('They will be set up via Patterns'),
    '#options' => $options,
  );
  $form['submit'] = array(
      '#type' => 'submit',
      '#value' => st('Continue'),
  );
  return $form;
}

/**
 * Implements qscience_profile_patterns_settings_form_submit().
 * Merges and executes the selected Patterns.
 */
function qscience_profile_patterns_settings_form_submit($form, &$form_state) {
  // Retrieve selected values and prepare execution
  $patterns_files = array_filter($form_state['values']['patterns']);
  if (count($patterns_files)>0) {
    // Retrieve the object of the first pattern file
    $pattern = _patterns_db_get_pattern(array_shift($patterns_files));
    // Merge actions of rest of patterns in the first one if any
    $pids = array();
    foreach($patterns_files as $pattern_file) {
      $subpattern = _patterns_db_get_pattern($pattern_file);
      foreach ($subpattern->pattern['actions'] as $action) {
        $pattern->pattern['actions'][] = $action;
      }
      $pids[] = $subpattern->pid;
    }
    // Execute merged pattern
    patterns_start_engine($pattern, array('run-subpatterns' => TRUE));
    // If all the subpatterns were successfully executed, marked the original ones as run
    foreach($pids as $pid) {
      $query_params = array(
        ':time' => time(),
        ':pid' => $pid,
        ':en' => PATTERNS_STATUS_ENABLED,
      );
      db_query("UPDATE {patterns} SET status = :en, enabled = :time WHERE pid = :pid", $query_params);
    }
  }
}

/**
 * Implements qscience_profile_patterns_share_form().
 */
function qscience_profile_patterns_share_form($form, &$form_state, &$install_state) {
  drupal_set_title(st('Share my QScience configuration'));
  
  $patterns_server_link = l("Drupal-patterns.org", "http://www.drupal-patterns.org", array('attributes' => array('target' => '_blank')));
  $message = '<p>' . st('Thanks for configuring your QScience instance. You can now share your configuration on !patterns_server_link. 
    By doing this your configuration can be installed by future QScience users. 
    At this stage no content of your web site will be transmitted during this process.', array('!patterns_server_link' => $patterns_server_link)) .'</p>';
  $message .= '<p>' . st('What\'s more, you will be contributing to the evolution of optimal QScience configurations!') .'</p>';
  $message .= '<p>' . st('Thanks for participating in the QScience project.') .'</p>';
  $form['share_message'] = array(
      '#markup' => $message,
  );
  $form['share'] = array(
      '#title' => st('I agree to share my configuration'),
      '#type' => 'checkbox',
      '#default_value' => TRUE,
  );
  $form['submit'] = array(
      '#type' => 'submit',
      '#value' => st('Save and continue'),
  );
  return $form;
}

/**
 * Implements qscience_profile_patterns_shares_form_submit().
 * Generates and send to the server the merged pattern.
 */
function qscience_profile_patterns_share_form_submit($form, &$form_state) {
  // TO-DO
}

/**
 * Custom installation task; perform final steps and display success message
 *
 * @param $install_state
 *   An array of information about the current installation state.
 *
 * @return
 *   A message informing the user about errors if there was some.
 */
function qscience_profile_install_finished(&$install_state) {
  drupal_set_title(st('QScience installation complete'), PASS_THROUGH);
  $messages = drupal_set_message();
  $output = '<p>' . st('Congratulations, you installed a QScience instance!') . '</p>';
  $output .= '<p>' . (isset($messages['error']) ? st('Review the messages above before visiting <a href="@url">your new site</a>', array('@url' => url(''))) : st('<a href="@url">Visit your new site</a>', array('@url' => url('')))) . '</p>';

  // Flush all caches to ensure that any full bootstraps during the installer
  // do not leave stale cached data, and that any content types or other items
  // registered by the installation profile are registered correctly.
  drupal_flush_all_caches();

  // Remember the profile which was used.
  variable_set('install_profile', drupal_get_profile());

  // Installation profiles are always loaded last
  db_update('system')->fields(array('weight' => 1000))->condition('type', 'module')->condition('name', drupal_get_profile())->execute();

  // Cache a fully-built schema.
  drupal_get_schema(NULL, TRUE);

  // Run cron to populate update status tables (if available) so that users
  // will be warned if they've installed an out of date Drupal version.
  // Will also trigger indexing of profile-supplied content or feeds.
  drupal_cron_run();

  return $output;
}
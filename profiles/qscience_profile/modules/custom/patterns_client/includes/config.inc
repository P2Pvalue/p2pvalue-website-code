<?php

/**
 * @file
 * Forms and settings for Patterns Client
 */

module_load_include('inc', 'd2d', 'includes/d2d.forms');

function patterns_client_form_outgoing_configure() {
  $form = array();
  
  $form['address'] = array(
  	'#title' => 'Patterns server address',
  	'#required' => TRUE,
  	'#default_value' => variable_get('patterns_client_server', PATTERNS_CLIENT_SERVER),
  	'#description' => t('Visit the <a href="@url">D2D admin page</a> to add more servers to the list.', array('@url' => url('admin/d2d'))),
  );
  
  d2d_forms_select_friends($form['address'], 'url');
  
  $form['auto'] = array(
    '#type' => 'checkbox',
    '#default_value' => variable_get('patterns_client_auto_publish', FALSE),
    '#title' => t('Automatically publish patterns'),
    '#description' => t('If checked, patterns are automatically published whenever is marked as \'public\'.'),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  return $form;
}

function patterns_client_form_incoming_configure_submit($form, &$form_state) {
  variable_set('patterns_client_allow_publish', $form_state['values']['allow'] ? TRUE : FALSE);
  drupal_set_message(t('The changes have been saved.'));
}

function patterns_client_form_outgoing_configure_submit($form, &$form_state) {
  variable_set('patterns_client_auto_publish', $form_state['values']['auto'] ? TRUE : FALSE);
  variable_set('patterns_client_server', $form_state['values']['address']);
  drupal_set_message(t('The changes have been saved.'));
}

function patterns_client_config() {
  $build['main'] = array(
    '#title' => t('Patterns Server'),
    '#type' => 'fieldset',
  );
  $build['main'][] = drupal_get_form('patterns_client_form_outgoing_configure');
  if (variable_get('patterns_client_auto_publish', FALSE)) {
    $url = variable_get('patterns_client_server', PATTERNS_CLIENT_SERVER);
    
    $credentials = patterns_client_get_full_credentials($url);
    
    if ($credentials) {

      //drupal_set_message(t('You are correctly registered on server @url.', array('@url' => $url)));
      
      $build['credentials'] = array(
      	'#title' => 'Your current credentials',
      	'#type' => 'fieldset',
        '#description' => t('You can edit this data go directly in the Patterns Server web site. Then use the "link to an existing username" form.')
      );
      
      $info['Server'][] = t('Server');
  	  $info['Server'][] = $url;
  	  $info['User'][] = t('User');
      $info['User'][] = $credentials['user'];
      $info['Token'][] = t('Token');
      $info['Token'][] = $credentials['token'];
      $info['Email'][] = t('Email');
      $info['Email'][] = $credentials['email'];
    
      $build['credentials'][] = array(
    		'#markup' => theme('table', array('rows' => $info, 'attributes' => array('class' => 'patterns-list'))),
    	);
      
    	$build['credentials']['modify'] = array(
      	'#title' => 'Below modify',
      );
      
    }
    else {
       drupal_set_message(t('You are not registered on server @url.', array('@url' => $url)), 'error');
    }
    
    // REGISTER NEW
    $build['register'] = array(
  		'#title' => $credentials ? t('Register a new username (overwrites your current credentials)') : t('Register a new username'),
  		'#type' => 'fieldset',
		);
  	$build['register'][] = drupal_get_form('patterns_client_register_remoteuser');
   

  	// LINK TO
    $build['link'] = array(
    	'#title' => $credentials ? t('Link to an existing username (overwrites your current credentials)') : t('Link to an existing username'),
  		'#type' => 'fieldset',
      '#description' =>  t('If you do not remember your username or password visit your Patterns Server web site and try a password recovery procedure there.'), 
		); 
  	$build['link'][] = drupal_get_form('patterns_client_link_remoteuser');
    
  }
  return $build;
}

function patterns_client_link_remoteuser() {
  $form = array();
  $form['user'] = array(
    '#type' => 'textfield',
    '#title' => t('Username'),
    '#description' => t('Existing username on server'),
    //'#default_value' => variable_get('patterns_client_server', PATTERNS_CLIENT_SERVER),
    '#size' => 40,
    '#maxlength' => PATTERNS_CLIENT_USER_MAX_LENGTH,
    '#required' => TRUE,
  );
  $form['password'] = array(
    '#type' => 'password',
    '#title' => t('Password'),
    '#required' => TRUE,
  );
 
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Link'),
  );
  
  return $form;
}

function patterns_client_link_remoteuser_validate($form, &$form_state) {
	if (!variable_get('patterns_client_server', FALSE)) {
		drupal_set_message(t('No server found to link to an existing user'), 'error');
		return FALSE;
	}
	return TRUE;
}

function patterns_client_link_remoteuser_submit($form, &$form_state) {
	$url = variable_get('patterns_client_server', PATTERNS_CLIENT_SERVER);
	$user = $form_state['values']['user'];
	$pwd = $form_state['values']['password'];
	patterns_client_link_user_on_server($url, $user, $pwd);
}

function patterns_client_register_remoteuser() {
  $form = array();
  $form['user'] = array(
    '#type' => 'textfield',
    '#title' => t('Username'),
    '#description' => t('New username to register on server'),
    //'#default_value' => variable_get('patterns_client_server', PATTERNS_CLIENT_SERVER),
    '#size' => 40,
    '#maxlength' => PATTERNS_CLIENT_USER_MAX_LENGTH,
    '#required' => TRUE,
  );
  $form['password'] = array(
    '#type' => 'password_confirm',
  	'#description' => t('Must be at least 6 characters/digits long'),
  	'#required' => TRUE,
  );
  $form['email'] = array(
    '#type' => 'textfield',
    '#title' => t('Email'),
  	'#required' => TRUE,
  );
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Register'),
  );
  
  return $form;
}

function patterns_client_register_remoteuser_validate($form, &$form_state) {
	if (!variable_get('patterns_client_server', FALSE)) {
		drupal_set_message(t('No server found to register a new user'), 'error');
		return FALSE;
	}
	
	if (!valid_email_address($form_state['values']['email'])) {
  	form_set_error('email', t('The email address appears to be invalid.'));
	}
	
  if (strlen($form_state['values']['password']) < 6) {
  	form_set_error('password', t('Password must be at least 6 characters/digits long'));
	}
	
	return TRUE;
}

function patterns_client_register_remoteuser_submit($form, &$form_state) {
	$url = variable_get('patterns_client_server', PATTERNS_CLIENT_SERVER);
	$user = $form_state['values']['user'];
	$pwd = $form_state['values']['password'];
	$email = $form_state['values']['email'];
	patterns_client_register_user_on_server($url, $user, $pwd, $email);
}
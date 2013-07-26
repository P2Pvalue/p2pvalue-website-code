D2D
===

## Intro ##

For a general introduction to Drupal-to-Drupal refer to the README.md file in the top directory.

## D2D in action ##

### Managing instances ###
The _Instances_-tab gives an overview of all the instances available in the database. The first instance listed is the instance corresponding to the own instance. To add an instance to the database, the _Instances_-tab provides a form with an address and several other fields. One can either manually fill out all these fields or only provide an address and then auto-complete the rest of the form. After adding an instance to the database, that instance is listed in the table at the top of the _Instances_-tab. Click "configure" to further set up an instance, i.e. to edit its description, associate a public key with that instance, send a friendship request etc.. The configuration of an instance can be accessed using several sub tabs. The _Details_-sub tab allows the description of an instance to be changed. In the _Public Key_-sub tab, the public key associated with an instance can be set. To associate a public key with an instance, click on _receive public key_. The newly received public key will be listed as a possible candidate for the public key. After checking the authenticity of the public key, a candidate key can be chosen as public key for an instance. Furthermore there's the option to _Check friends' public keys_ which invokes a secure rpc on all the friends instances checking their public keys.

The _Friendship_ sub tab is used to manage the friendship with an instance. Here you can send a friendship request, accept a pending friendship request, revoke a previously sent friendship offer, or terminate a previously established friendship. Optionally, if your D2D instance is online, you can immediatelly send this request. Otherwise, or if the other instance is not reachable the updated friendship state is sent later on.

Also, you can manually set the state of friendship. Note that this does not send this state to the other instance and should be used with care and only if you know what you are doing!

The _Groups_-sub tab allows the managed instance to be put into groups. See the _Groups & Permissions_-tab for details.

The _Permissions_-sub-tab gives an overview of the functions that can be invoked by the instance. Note that these functions can only be called while friendship is established. To change permissions of an instance, change the groups it belongs to and allow members of groups to call particular functions, see _Groups & Permissions_-tab for details.
Similarly, the _Remote Permissions_-sub tab gives an overview of the functions that can be invoked per remote on a friend instance. Again, note that the functions can only be invoked if friendship is valid. Furthermore listing permissions is only possible if allowed by the remote instance.

### Notifications ###
The _Notifications_-tab gives a short overview of "events" occuring in D2D. Such an event can be an incoming friendship request, an accepted friendship request etc. Note that whenever another instance that is not listed in your database or an instance that is listed in your database for which you did not chose a public key yet sends you a friendship requests, you are notified about it here. After inserting the instance to the database (a link for doing this comes along the notification) and associating a public key with that instance the signature of the friendship request is checked and you get a notification that the request can be accepted (in case the signature of the friendship request is valid, of course). Whenever there are unread notifications, the admin of D2D is altered using a small message while browsing any of the other D2D tabs.

### Groups & Permissions ###
The _Groups & Permissions_-tab allows groups to be created and give permissions to members of the groups. In the _Groups_-sub tab, new groups can be created. Optionally, newly added instances can be added to a group. Furthermore, a description of the group can be provided.
The _Permissions_-sub tab lists functions that can be called by friends on this instance and are defined by modules implementing `hook_d2d_secure_rpc()`.
Note that with the installation of D2D a standard group is created with every new instance being added to this group by default. Permission to basic functions coming with D2D is given per default to members of this group.

### Implementing secure remote procedures ###

Functions implementing `hook_d2d_secure_rpc()` have to return an array of methods that can be invoked. The key defines the name under which a remote function can be called, its value should be an array of key/value pairs, where the keys define `arguments`, `callback` and `description`. An example follows. Note that this and all other code snippets provided are taken from a working sub module. This sub module called `D2D Example` can be found in the `examples` sub directory. To test the example enable the module on both your and a friend instance and make sure you give the necessary rights to the corresponding functions.

```php
    /**
     * Implements hook_d2d_secure_rpc().
     */
    function d2d_example_d2d_secure_rpc() {
      $methods = array();
      $methods['d2d_example_remote_control'] = array(
        'arguments' => array('code' => 'is_string'),
        'callback' => 'd2d_example_srpc_remote_control',
        'description' => 'runs code on remote instance',
      );
      $methods['d2d_example_info'] = array(
        'arguments' => array(),
        'callback' => 'd2d_example_srpc_info',
        'description' => 'returns information about this instance',
      );
      return $methods;
    }
```

The value of the `callback`-key gives the name of a function being called internally when the specified function is called via remote.

The value belonging to the `arguments`-key has to be an array specifying the arguments being passed to the callback. The keys define the names of the arguments while the values specify functions being called for checking the corresponding argument. The function that checks the argument is called with the corresponding argument as reference argument and should return a boolean, in particular it should return `TRUE` if the argument is valid. Furthermore note that the argument gets its argument passed by reference and therefore cannot only do checks but also conversions.
An example of how a callback should look like follows.

```php
    function d2d_example_d2d_secure_rpc() {
      $methods = array();
      $methods['d2d_example_remote_control'] = array(
        'arguments' => array('code' => 'is_string'),
        'callback' => 'd2d_example_srpc_remote_control',
        'description' => 'runs code on remote instance',
      );
      $methods['d2d_example_info'] = array(
        'arguments' => array(),
        'callback' => 'd2d_example_srpc_info',
        'description' => 'returns information about this instance',
      );
      return $methods;
    }
```

The callback is given two arguments. The first argument is an array of arguments as defined in the hook and is also as checked by the functions specified in the hook. The second argument is an associative array with keys `instance`, and `ip` corresponding to an associative array giving describing the instance invoking the call, and the IP address the call came from. Please refer to the following code snippet on how to access the D2D-id resp. the IP of the caller.

```php
    function d2d_example_srpc_remote_control($arguments, $rpc_info) {
      watchdog(
        'D2D Example', 'Remote control called from \'%ip\' by instance with D2D-id \'%d2d_id\'',
        array(
          '%ip' => $rpc_info['ip'],
          '%d2d_id' => $rpc_info['instance']['d2d_id'],
        )
      );
      return eval($arguments['code']);
    }
```

A proper return value of the callback function has to be of type string. For a function to return more advanced values than just strings, these values have to be converted to a string. An example imploding an array to a string follows.

```php
    function d2d_example_srpc_info($arguments, $rpc_info) {
      $friends = d2d_api_friend_get();
      $n_friends = sizeof($friends);
      $software = $_SERVER['SERVER_SOFTWARE'];
      $phpversion = phpversion();
      $class = 'DatabaseTasks_' . Database::getConnection()->driver();
      $tasks = new $class();
      $dbname = '';
      $dbversion = '';
      $dbname = $tasks->name();
      $dbversion = Database::getConnection()->version();
      $return_array = array(
        'time' => date('l jS \of F Y h:i:s A'),
        'drupal version' => VERSION,
        'web server' => $software,
        'php version' => $phpversion,
        'database' => $dbname . $dbversion,
        'number of friends' => $n_friends,
      );
      foreach ($return_array as &$value) {
        $value = strval($value);
      }
      $imploded_return_array = d2d_implode($return_array);
      if ($imploded_return_array === FALSE) {
        return '';
      }
      return $imploded_return_array;
    }
```

To call such a secure remote procedure on a friend instance, `d2d_call_secure_rpc` has to be invoked. `d2d_call_secure_rpc` takes four arguments: the first one specifies the friend to call that method on, the second one gives the name of the remote function (as defined in the corresponding hook on the remote instance). The third argument should be an associative array defining the arguments. Note that the keys have to exactly match and be in the same order as in the corresponding definition in the hook on the remote instance. The fourth and last argument is passed by reference. On error, this variable will contain the error string. Finally, on success `d2d_call_secure_rpc` return the string being returned by the friend instance, `FALSE` otherwise. An example follows.

```php
    function d2d_example_remote_control_form() {
      $form = array();
      $friends = d2d_api_friend_get();
      if (empty($friends)) {
        drupal_set_message(t('No friends found in database'));
        return $form;
      }
      $options = array();
      $descriptions = array();
      $last_id = variable_get('d2d_example_remote_control_last_id');
      $proposed_id = null;
      foreach ($friends as $friend) {
        $options[$friend['id']] = $friend['name'];
        $descriptions[$friend['id']] = $friend['url'];
        if (is_null($proposed_id) || $friend['id'] == $last_id) {
          $proposed_id = $friend['id'];
        }
      }
      $form['friend'] = array(
        '#type' => 'radios',
        '#title' => t('Instance to run code on'),
        '#default_value' => $proposed_id, 
        '#options' => $options,
      );
      foreach ($descriptions as $id => $description) {
        $form['friend'][$id]['#description'] = $description;
      }
      $form['code'] = array(
        '#type' => 'textarea',
        '#title' => t('PHP Code to run'),
        '#description' => t('The provided code is evaluated using PHP\'s eval-function.'),
        '#rows' => 10,
        '#cols' => 60,
        '#default_value' => variable_get('d2d_example_remote_control_code', ''),
        '#required' => TRUE,
      );
      $form['submit'] = array(
        '#type' => 'submit', 
        '#value' => t('Run code'),
      );
      return $form;
    }
    
    function d2d_example_remote_control_form_submit($form, &$form_state) {
      variable_set('d2d_example_remote_control_code', $form_state['values']['code']);
      $friend_id = $form_state['values']['friend'];
      variable_set('d2d_example_remote_control_last_id', $friend_id);
      $friends = d2d_api_friend_get();
      foreach ($friends as $friend) {
        if ($friend_id === $friend['id']) {
          $res = d2d_call_secure_rpc($friend, 'd2d_example_remote_control', array('code' => $form_state['values']['code']), $error_string);
          if ($res === FALSE) {
            drupal_set_message(t('Error: @message', array('@message' => $error_string)), 'error');
          }
          else {
            drupal_set_message(t('Method returned \'@return\'.', array('@return' => $res)));
          }
          return;
        }
      }
      drupal_set_message(t('No friend selected.'), 'warning');
    }
```

### System ###

The _System_-tab lists outgoing requests, in particular outgoing friendship requests. One can manually send all outgoing requests using the corresponding button in this tab. This should only be necessary in exceptional cases as the outgoing requests are being sent periodically using Drupal's cron.

### Settings ###
The settings of D2D can be adjusted using the _Settings_-tab. The _General Settings_-sub tab gives various options to configure D2D. Description for the available options is provided with the corresponding form in the sub tab. The _Keys_-sub tab allows the public / private key pair to be changed. Note that the key can only be changed if the D2D instance is offline. Switching between online and offline mode can be done in the _Advanced_-sub tab.


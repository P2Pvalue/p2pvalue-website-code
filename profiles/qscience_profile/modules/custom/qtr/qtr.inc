<?php

/**
 * @file
 * Private QTR settings and functions.
 */

/*function _qtr_save($agent, $item, $agent_file, $item_file) {
  $ofs_AG = fopen($agent_file, "w");
  $ofs_IT = fopen($item_file, "w");
  $sorted_agent = array();
  $sorted_item = array();
//save users' rank
  //for ($i = 0; $i < count($agent); $i++) {
  foreach ($agent as $i=>$content) {
  	$sorted_agent += array ($i => $agent[$i]['reputation']);
  }
  arsort($sorted_agent);
  foreach($sorted_agent as $key => $value){
  	fwrite($ofs_AG, "$key\t" . $value . "\n");
  }
  
  //save the item ranking into a file
 // for ($a = 0; $a < count($item); $a++) {
 foreach ($item as $a=>$content) {
  	$sorted_item += array ($a => $item[$a]['quality']);
    //fwrite($ofs_IT, "$a\t" . $item[$a]['quality'] . "\n");
  }
  arsort($sorted_item);
  foreach($sorted_item as $key1 => $value1){
  	fwrite($ofs_IT, "$key1\t" . $value1 . "\n");
  }
  fclose($ofs_AG);
  fclose($ofs_IT);
}*/

/*function _qtr_init(&$agent, &$item, $agent_num, $item_num) {

    $agent = array_fill(0, $agent_num, array(
        'reputation' => 0.0, // reputation value
        'r_tmp' => 0.0,
        'collection' => array(), // ID of items the user has interacted with
        'c_time' => array(), // time when user interacted with an item
        'c_weight' => array(), // weight associated to the interaction
        'neighbor' => array(), // ID of trusted fellows of the users
        't_exp' => array() // amount of trust
            ));

    $item = array_fill(0, $item_num, array(
        'quality' => 0.0, // quality value
        'q_tmp' => 0.0,
        'reader' => array(), // ID of users who interacted with item
        'r_time' => array(), // time when an user interacted with item
        'r_weight' => array() // weight associated to the interaction
            ));
}*/

function _qtr_read(&$agent, &$item, $trust) {
    // $trust is for explicit trust.

    /* reading input file for user-item interactions */
    /* FORMAT: userID \t itemID \t interaction_type \t interaction_time
     * (eventually, \t kind of object) */
    // N.B.: the time units are decided here; set variables in lines 27 and 38
    // accordingly

    $actions = qtr_get_actions();
    foreach ($actions as $action) {
        $i = (int) $action->uid;
        $a = (int) $action->nid;
//    $rating = (string) $action->action;
    $time = (int) $action->timestamp;
    $rw = $action->weight;
    if (!isset($agent[$i])) {
      $agent[$i]=array(
    'reputation' => 0.0, // reputation value
    'r_tmp' => 0.0,
    'collection' => array(), // ID of items the user has interacted with
    'c_time' => array(), // time when user interacted with an item
    'c_weight' => array(), // weight associated to the interaction
    'neighbor' => array(), // ID of trusted fellows of the users
    't_exp' => array() // amount of trust
  );
 
    }
    if (!isset($item[$a])) {
      $item[$a] = array(
    'quality' => 0.0, // quality value
    'q_tmp' => 0.0,
    'reader' => array(), // ID of users who interacted with item
    'r_time' => array(), // time when an user interacted with item
    'r_weight' => array() // weight associated to the interaction
    );
    }
    if (!in_array($a, $agent[$i]['collection'])) { // this is to avoid duplicates; it is possible to remove this control
      $item[$a]['reader'][] = $i; 
      $agent[$i]['collection'][] = $a;
      $item[$a]['r_time'][] = $time; 
      $agent[$i]['c_time'][] = $time;
      $item[$a]['r_weight'][] = $rw;
      $agent[$i]['c_weight'][] = $rw;
    }
    /* now if $trust is not NULL, reading input file for trust relationships */
    /* FORMAT: userID \t ID_of_user_trusted_by_i \t amount_of_trust */
    if ($trust !== NULL) {
        $results = db_query("SELECT * FROM {qtr_trust}");
        foreach ($results as $result) {
            $i = (int) $result->uid1;
            $j = (int) $result->uid2;
            $tt = (double) $result->trust;
            $agent[$i]['neighbor'][] = $j;
            $agent[$i]['t_exp'][] = $tt;
        }
    }
}
}

function _qtr_decay($tau) {
    $tau0 = variable_get('qtr_tau0', 50)*24*3600;
    switch (variable_get('qtr_decay', 0)) {
        case 1:
            return pow(1. + ((double) $tau) / $tau0, -1);
        case 2:
            return exp(-((double) $tau) / $tau0);
        case 3:
            return $tau <= $tau0 ? 1.0 : 0.0;
        default:
            return 1.0;
    }
}

function _qtr_calculate(&$agent, &$item, $trust, $start) {

  // initializing the error an the average values; computing average trust //
  $err = count($agent) * count($item); $avg_q = 0; $avg_r = 0; $avg_t = 0; $nn = 0;
  if ($trust !== NULL) {
    //for ($i = 0; $i < count($agent); $i++) {
    foreach ($agent as $i => $content) {
      for ($j = 0; $j < count($agent[$i]['neighbor']); $j++) {
        $avg_t += $agent[$i]['t_exp'][$j];
        $nn++;
      }
    }
    $avg_t /= $nn;
  }

  // setting the initial values of reputation and quality //
 // for ($a = 0; $a < count($item); $a++) {
 foreach ($item as $a => $content) {
    $item[$a]['q_tmp'] = 1. / sqrt(count($item));
  }
 // for ($i = 0; $i < count($agent); $i++) {
 foreach ($agent as $i => $content) {
    $agent[$i]['r_tmp'] = 1. / sqrt(count($agent));
  }

  // iterations //
  $flag = 0;
  while ($err > variable_get('qtr_delta', 0.00000000001)) {
    $err = 0;
    $norm = 0;
   // for ($a = 0; $a < count($item); $a++) {
   foreach ($item as $a => $content) {
      $item[$a]['quality'] = 0;
      for ($r = 0; $r < count($item[$a]['reader']); $r++) {
        $item[$a]['quality'] += $item[$a]['r_weight'][$r] * ($agent[$item[$a]['reader'][$r]]['r_tmp'] - variable_get('qtr_resc_r', 0) * $avg_r) * _qtr_decay($start - $item[$a]['r_time'][$r]) /
          pow(count($item[$a]['reader']), variable_get('qtr_renorm_q', 0));
      }
      $norm += pow($item[$a]['quality'], 2);
    }
    // normalization of newly obtained qualities and updating of error
  //  for ($a = 0; $a < count($item); $a++) {
  foreach ($item as $a => $content) {
      $item[$a]['quality'] /= sqrt($norm);
      $err += abs($item[$a]['quality'] - $item[$a]['q_tmp']);
    }
    // cycle on users to determine reputations
    $norm = 0;
  //  for ($i = 0; $i < count($agent); $i++) {
  foreach ($agent as $i => $content) {
      $agent[$i]['reputation'] = 0;
      for ($a = 0; $a < count($agent[$i]['collection']); $a++) {
        $agent[$i]['reputation'] += $agent[$i]['c_weight'][$a] * ($item[$agent[$i]['collection'][$a]]['q_tmp'] - variable_get('qtr_resc_q', 0) * $avg_q) *
          _qtr_decay($start - $agent[$i]['c_time'][$a]) / pow(count($agent[$i]['collection']), variable_get('qtr_renorm_r', 0));
      }
      if ($trust !== NULL) {
        for ($j = 0; $j < count($agent[$i]['neighbor']); $j++) {
          $agent[$i]['reputation'] += ($agent[$agent[$i]['neighbor'][$j]]['r_tmp'] - variable_get('qtr_resc_t', 0) * $avg_r) * ($agent[$i]['t_exp'][$j] - variable_get('qtr_resc_t', 0) * $avg_t) /
            pow($agent[$i]['reputation'], 2);
        }
      }
      $norm += pow($agent[$i]['reputation'], 2);
    }
   // for ($i = 0; $i < count($agent); $i++) {
   foreach ($agent as $i => $content) {
      $agent[$i]['reputation'] /= sqrt($norm);
      $err += abs($agent[$i]['reputation'] - $agent[$i]['r_tmp']);
    }

    // updating scores and averages //
    $avg_q = 0; $avg_r = 0;
   // for ($a = 0; $a < count($item); $a++) {
   foreach ($item as $a => $content) {
      $item[$a]['q_tmp'] = $item[$a]['quality'];
      $avg_q += $item[$a]['quality'] / count($item);
    }
  //  for ($i = 0; $i < count($agent); $i++) {
  foreach ($agent as $i => $content) {
      $agent[$i]['r_tmp'] = $agent[$i]['reputation'];
      $avg_r = $agent[$i]['reputation'] / count($agent);
    }
    // displaying the current iteration step
    $flag++;
   // drupal_set_message($flag . " " . $err);
  }
  //print_r($agent);
  // TODO save the values to the database
 // qtr_write_values('qtr_reputation', $agent);
 // qtr_write_values('qtr_quality', $item);
   drupal_set_message(t('QTR run successfully!'));

}


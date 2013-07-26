<?php

/**
 * @file
 * 
 * Api to write and access QTR data and the database
 */
// WRITE

/**
 * Writes a a new record in the QTR table
 * 
 * @param {mixed} $uid The user id from the users table
 * @param {mixed} $nid The item id
 * @param {mixed} $action The type of action
 * @param {mixed} $time Optional. The timestamp associated with 
 *   the action. Defaults, now()
 */
function qtr_write_action($uid, $nid, $aid, $time = REQUEST_TIME) {
  //check $uid
  if (!user_load($uid)) {
    return FALSE;
  }
  //check $nid
  if (!node_load($nid)) {
    return FALSE;
  }
  //check $aid
  if (!qtr_get_actiontype($aid)) {
    return FALSE;
  }
  $query = db_select('qtr_actions', 'q');

  $result = $query->fields('q')->condition('uid', $uid)->condition('nid', $nid)->condition('action_type', $aid)->execute()->fetchCol();
  if (!$result) {
    db_insert('qtr_actions')->fields(array(
          'uid' => $uid,
          'nid' => $nid,
          'action_type' => $aid,
          'timestamp' => $time,


          ))->execute();
  }

}

/**
 * Writes the values of the computation of QTR for 
 * 
 * @param {mixed} $table The table to insert the date
 * @param {mixed} $values the array of values to insert
 */
function qtr_write_values($table, $values) {
  // TODO
  if ($table == "qtr_reputation") {
    $sorted_values = array();
    foreach ($values as $i => $value) {
      $sorted_values += array(
          $i => $values[$i]['reputation']
          );
    }
    arsort($sorted_values);
  }
  switch ($table) {
    case "qtr_reputation" :
      $time =  strtotime('now');
    $latesttime = db_query('select timestamp from {qtr_reputation} order by timestamp desc limit 1')->fetchCol();
    if ($latesttime) {
      $monthnow = (int)date('m', $time);
      $monthlast = (int)date('m', $latesttime[0]); 
      if ($monthnow <= $monthlast) {//overwrite the reputation 
        $del = db_delete('qtr_reputation')->condition('timestamp', $latesttime)->execute();
      }
    }

    $i = 1;
    $count = count($sorted_values);
    if ($count) {
      foreach ($sorted_values as $id => $value) {
        $sql = "";
        db_insert('qtr_reputation')->fields(array(
              'uid' => $id,
              'rank' => $i,
              'reputation' => $value,
              'percentile' => $i/$count,
              'timestamp' => $time,
              ))->execute();
        $i++;
      }
    }
    break;
    case "qtr_quality" :
      $del = db_delete('qtr_quality')->execute();
    if (count($values))
      foreach ($values as $id => $value) {
        db_insert('qtr_quality')->fields(array(
              'nid' => $id,
              'quality' => $value['quality']
              ))->execute();
      }
    break;

  }
}

// GET ACTIONS

/**
 * Retrieves sets of actions from the QTR table
 *
 * FALSE if 
 */
function qtr_get_actions() {

  //use join search from qtr_action and qtr_action_types to get action and weight values directly 
  $actions = db_query("SELECT a.uid, a.nid, at.action, a.timestamp, at.weight 
      FROM {qtr_actions} AS a LEFT JOIN {qtr_action_types} AS at ON at.id=a.action_type 
      INNER JOIN {node} AS n ON n.nid=a.nid where n.type IN (select item_type from {qtr_item_types})")->fetchALL();
  return $actions;

}


/* GET VALUES
 * @param $table
 * @param $field The field name of the value to get
 * @param $id 
 */

function qtr_get_value($table, $field, $id) {
  switch ($table) {
    case "qtr_reputation" :
      $latesttime = db_query('select timestamp from {qtr_reputation} order by timestamp desc limit 1')->fetchCol();
    $query = db_select('qtr_reputation', 'q');
    $result = $query->fields('q', array($field))->condition('uid', $id)->condition('timestamp', $latesttime[0])->execute()->fetchCol();
    return $result;
    break;
    case "qtr_quality" :
      $query = db_select('qtr_quality', 'q');
    $result = $query->fields('q', array($field))->condition('nid', $id)->execute()->fetchCol();
    return $result;
    break;
    break;

  }
}

// QTR ADD NODE TYPE

/**
 * 
 * add item types to qtr_item_types table
 * @param {mixed} $types The node types
 */
function qtr_update_itemtype($types = array()) {
  //delete current item types
  $del = db_delete('qtr_item_types')->execute();
  //add new item types
  foreach ($types as $type) {
    $type += array(
        'reference_table' => 'node',


        );
    db_insert('qtr_item_types')->fields(array(
          'item_type' => $type['item_type'],
          'reference_table' => $type['reference_table']
          ))->execute();
  }
}

/**
 * 
 * get item types from qtr_item_types table
 */
function qtr_get_itemtype() {
  $query = db_select('qtr_item_types', 'q');
  $result = $query->fields('q')->execute()->fetchAll();
  if (!empty ($result)) {
    return $result;
  }
  else {
    return FALSE;
  }
}

// QTR ADD ACTION TYPE

/**
 * 
 * Enter description here ...
 * @param {mixed} $type The action type to create
 * @param {mixed} $weight The action type to create
 * @param {mixed} $description The description associated to the action
 */
function qtr_add_actiontype($type = NULL, $weight = 0, $description = NULL) {
  if (is_null($type))
    return;
  else {
    db_insert('qtr_action_types')->fields(array(
          'action' => $type,
          'weight' => $weight,
          'description' => $description
          ))->execute();
  }

}

/**
 * 
 * get action types from qtr_action_types table
 */
function qtr_get_actiontype($aid = NULL) {

  $query = db_select('qtr_action_types', 'q');
  if ($aid == NULL) {
    $result = $query->fields('q')
      ->execute()
      ->fetchAll();
  }
  else {
    $result = $query->fields('q')
      ->condition('id', $aid)
      ->execute()
      ->fetchCol();
  }

  if (!empty($result)) {
    return $result;
  }
  else {
    return FALSE;
  }
}

/**
 * 
 * update action weight
 */
function qtr_update_actionweight($action, $weight) {
  db_query("UPDATE {qtr_action_types} SET weight = :weight WHERE action = :action", array(
        'weight' => $weight,
        'action' => $action
        ));

}

/**
 * 
 * get top items
 * @param {mixed} $num The number of returned items
 * 
 */
function qtr_get_topquality($num) {

  $showlist = array();
  $count = 0;
  $types = array();
  foreach (node_type_get_types() as $type => $type_obj) {
    if (variable_get('popular_block_' . $type, 0)) {
      $types[] = $type;
    }
  }
  if (count($types) > 0) {
    $types = implode("','", $types);
    $types = "'" . $types . "'";
    $sql = 'SELECT n.nid,n.title FROM {qtr_quality} AS q INNER JOIN {node} as n ON q.nid=n.nid and n.type IN (' . $types . ')  ORDER BY q.quality DESC LIMIT ' . $num;
    $result = db_query($sql)->fetchAll();
    return $result;
  }
  else{
    return FALSE;
  }
}
/**
 * 
 * get top items by action type
 * @param {mixed} $num The number of returned items
 * @param {mixed} $actiontype 
 * @param {mixed} $time how many days to statistic the result 
 */
function qtr_get_topitems($num, $actiontype, $time) {
  $start=strtotime('now')-$time*24*3600;
  $showlist = array();
  $count = 0;
  $types = array();
  foreach (node_type_get_types() as $type => $type_obj) {
    if (variable_get('popular_block_' . $type, 0)) {
      $types[] = $type;
    }
  }
  if (count($types) > 0) {
    $types = implode("','", $types);
    $types = "'" . $types . "'";
    //   $sql = 'SELECT n.nid,n.title,count(q.nid) as count FROM {node} AS n LEFT JOIN {qtr_actions} as q ON (q.nid=n.nid and q.action_type='.$actiontype.' AND q.timestamp>'.$start.') where n.type IN (' . $types . ') GROUP BY n.nid ORDER BY count DESC LIMIT ' . $num;
    $sql = 'SELECT q.nid,n.title,count(q.nid) as count FROM {qtr_actions} as q RIGHT JOIN {node} AS n ON (q.nid=n.nid and n.type IN (' . $types . ') AND q.action_type=' . $actiontype . ' AND q.timestamp>' . $start . ') GROUP BY n.nid ORDER BY count DESC LIMIT ' . $num;        
    $result = db_query($sql)->fetchAll();
    return $result;
  }
  else{
    return FALSE;
  }
}

/**
 * Implements hook_exit().
 *
 * This is where actions are gathered on page accesses.
 */
function qtr_exit() {
  global $user;
  $uid = $user->uid;
  if ($uid==0)
    return;

  drupal_bootstrap(DRUPAL_BOOTSTRAP_VARIABLES);
  drupal_bootstrap(DRUPAL_BOOTSTRAP_SESSION);

  // For anonymous users unicode.inc will not have been loaded.
  include_once DRUPAL_ROOT . '/includes/unicode.inc';
  // Log this page access.
  $nid = '';
  $aid = '';
  if (arg(0) == 'node' && is_numeric(arg(1)) && arg(2) == NULL) {
    $nid = arg(1);
    $aid = 3;
  } 
  elseif (arg(0) == 'paper' && arg(1) == 'download' && is_numeric(arg(2))) {
      $nid = arg(2);
      $aid = 2;
  } 
  elseif (arg(0) == 'plus1' && arg(1) == 'vote' && arg(2) == 'node' && is_numeric(arg(3))) {
        $nid = arg(3);
        $aid = 1;
  }

  qtr_write_action($uid, $nid, $aid);
}

/**
 * cout the user's reputation history as a chart'
 *
 */
function qtr_chart_reputation($uid) {
  $user=user_load($uid);

  $query = db_select('qtr_reputation', 'q');
  $result = $query->fields('q', array('percentile', 'timestamp'))->condition('uid', $uid)->orderBy('timestamp', 'asc')->range(0, 12)->execute()->fetchAll();
  $percentile = array();
  $chxl='0:|';
  for ($i = 11; $i >= 0; $i--) {
    $month = date('m-y', strtotime("-" . $i . " months"));
    $percentile[$month] = 0;
    $chxl .= $month . '|';
  }
  $chxl .= '2:|Percentile/Month-Year';
  $max = 0;
  foreach ($result as $repu) {
    if ($repu->percentile) {
      $percentile[date('m-y', $repu->timestamp)] = 1-$repu->percentile;
      if (1-$repu->percentile > $max)
        $max = 1-$repu->percentile;
    }

  }
  $chxr = '1,0,' . ($max+0.1)*100;
  $chd = 't:';
  foreach ($percentile as $key => $value) {
    $chd .= $value . ',';
  }
  $chd = trim($chd, ',');



  //
  $data = array();
  $data['cht'] = 'lc';//chart type
  $data['chtt'] = t($user->name . '\'s reputation percentile (how many users have lower reputation than ' . $user->name . ')');//title
      $data['chs'] = '600x200';
      $data['chm'] = 'c,FF0000,0,-1,5';
      $data['chg'] = '-1,-1,1,1';
      $data['chxt'] = 'x,y,t';
      $data['chds'] = '0,' . ($max+0.1);
      $data['chd'] = $chd;
      $data['chxl'] = trim($chxl, '|');
      $data['chxp'] = '2,-1';
      $data['chxr'] = $chxr;
      //$data['cht']='';


      $attributes['id'] = 'chart-reputation-history';
      $attributes['class'][] = 'chart';
      $url=url('https://chart.googleapis.com/chart', array('query' => $data, 'external' => TRUE));
      return theme('image', array('path' => $url, 'attributes' => $attributes, 'alt' => 'Percentile of ' . $user->name . '\'s reputation that higher than others'));

}


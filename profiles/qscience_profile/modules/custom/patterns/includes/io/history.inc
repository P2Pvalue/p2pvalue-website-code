<?php

/**
 * 
 * Adds a history header in the info section of the Pattern.
 * 
 * @param array $pattern  Pattern in which the new header will be appended
 * @param string $action  Action that has produced this pattern, currenlty 'merge' or 'pull'
 * @param string $host  URL of the instance that performed the action
 * @param string $d2d_id  D2D ID of the instance that performed the action
 * @param array $ancestors  Array of UUUIDs to be appended
 * 
 * @return Pattern including the new header
 */
function patterns_history_add_header($pattern, $action, $host, $d2d_id, $ancestors) {
  $pattern['info']['history'][$action] = array();
  $pattern['info']['history'][$action]['host'] = $host;
  $pattern['info']['history'][$action]['d2d_id'] = $d2d_id;
  $pattern['info']['history'][$action]['date'] = date_timestamp_get(date_create());
  foreach ($ancestors as $uuuid) {
    $pattern['info']['history'][$action]['uuuids'][] = $uuuid;
  }
  
  
  return $pattern;
}
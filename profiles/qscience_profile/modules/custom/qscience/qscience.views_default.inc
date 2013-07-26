<?php


function qscience_views_default_views() {
  $v = array();

  include(dirname(__FILE__) . '/views_config/papers_backlink.inc');
  $v[$view->name] = $view;

  include(dirname(__FILE__) . '/views_config/reference_backlink.inc');
  $v[$view->name] = $view;

  return $v;
}

function qscience_block_info() {
  $info = array(
    'papers_block_3-block' => array(
      'info' => t('Papers Block 2'),
      'status' => TRUE,
      'region' => 'content',
      'visibility' => BLOCK_VISIBILITY_LISTED,
      'pages' => 'node/*',
    ),
    'authors_block_3-block' => array(
      'info' => t('Authors Block 2'),
      'status' => TRUE,
      'region' => 'content',
      'visibility' => BLOCK_VISIBILITY_LISTED,
      'pages' => 'node/*',
    )
  );

  return $info;
}

<?php

/**
 * @file
 * This config will create the References block to a given Paper.
 */

$view = new view();
$view->name = 'papers_links_to_this_paper';
$view->description = '';
$view->tag = 'default';
$view->base_table = 'node';
$view->human_name = 'Citations links to this paper';
$view->core = 7;
$view->api_version = '3.0';
$view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */

/* Display: Master */
$handler = $view->new_display('default', 'Master', 'default');
$handler->display->display_options['title'] = 'Cited by';
$handler->display->display_options['use_more_always'] = FALSE;
$handler->display->display_options['access']['type'] = 'perm';
$handler->display->display_options['cache']['type'] = 'none';
$handler->display->display_options['query']['type'] = 'views_query';
$handler->display->display_options['exposed_form']['type'] = 'basic';
$handler->display->display_options['pager']['type'] = 'full';
$handler->display->display_options['style_plugin'] = 'default';
$handler->display->display_options['row_plugin'] = 'fields';
/* No results behavior: Global: Text area */
$handler->display->display_options['empty']['area']['id'] = 'area';
$handler->display->display_options['empty']['area']['table'] = 'views';
$handler->display->display_options['empty']['area']['field'] = 'area';
$handler->display->display_options['empty']['area']['label'] = 'no result';
$handler->display->display_options['empty']['area']['empty'] = TRUE;
$handler->display->display_options['empty']['area']['content'] = 'no result';
$handler->display->display_options['empty']['area']['format'] = 'filtered_html';
/* Field: Content: Title */
$handler->display->display_options['fields']['title']['id'] = 'title';
$handler->display->display_options['fields']['title']['table'] = 'node';
$handler->display->display_options['fields']['title']['field'] = 'title';
$handler->display->display_options['fields']['title']['label'] = '';
$handler->display->display_options['fields']['title']['alter']['word_boundary'] = FALSE;
$handler->display->display_options['fields']['title']['alter']['ellipsis'] = FALSE;
/* Sort criterion: Content: Post date */
$handler->display->display_options['sorts']['created']['id'] = 'created';
$handler->display->display_options['sorts']['created']['table'] = 'node';
$handler->display->display_options['sorts']['created']['field'] = 'created';
$handler->display->display_options['sorts']['created']['order'] = 'DESC';
/* Contextual filter: Content: Citations (citation_reference) */
$handler->display->display_options['arguments']['citation_reference_target_id']['id'] = 'citation_reference_target_id';
$handler->display->display_options['arguments']['citation_reference_target_id']['table'] = 'field_data_citation_reference';
$handler->display->display_options['arguments']['citation_reference_target_id']['field'] = 'citation_reference_target_id';
$handler->display->display_options['arguments']['citation_reference_target_id']['default_action'] = 'default';
$handler->display->display_options['arguments']['citation_reference_target_id']['default_argument_type'] = 'node';
$handler->display->display_options['arguments']['citation_reference_target_id']['summary']['number_of_records'] = '0';
$handler->display->display_options['arguments']['citation_reference_target_id']['summary']['format'] = 'default_summary';
$handler->display->display_options['arguments']['citation_reference_target_id']['summary_options']['items_per_page'] = '25';
/* Filter criterion: Content: Published */
$handler->display->display_options['filters']['status']['id'] = 'status';
$handler->display->display_options['filters']['status']['table'] = 'node';
$handler->display->display_options['filters']['status']['field'] = 'status';
$handler->display->display_options['filters']['status']['value'] = 1;
$handler->display->display_options['filters']['status']['group'] = 1;
$handler->display->display_options['filters']['status']['expose']['operator'] = FALSE;
/* Filter criterion: Content: Type */
$handler->display->display_options['filters']['type']['id'] = 'type';
$handler->display->display_options['filters']['type']['table'] = 'node';
$handler->display->display_options['filters']['type']['field'] = 'type';
$handler->display->display_options['filters']['type']['value'] = array(
    'paper' => 'paper',
  );

/* Display: EVA Field */
$handler = $view->new_display('entity_view', 'EVA Field', 'entity_view_1');
$handler->display->display_options['defaults']['hide_admin_links'] = FALSE;
$handler->display->display_options['entity_type'] = 'node';
$handler->display->display_options['bundles'] = array(
    0 => 'paper',
  );
$handler->display->display_options['show_title'] = 1;


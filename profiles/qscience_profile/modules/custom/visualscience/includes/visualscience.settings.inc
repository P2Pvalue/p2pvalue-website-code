<?php

/**
* @file
* Some general settings, such as the Help Page and a function to define the permissions.
* TODO: replace both functions.
*/


/**
 * Implements hook_help()
 * TODO: Write a user friendly help
 */
function visualscience_help($path, $arg) {
  switch ($path) {
    case 'admin/help#visualscience':
    $output = '<h1>
    <a href="#visualscience" class="anchor" name="visualscience"></a>VisualScience</h1>

    <p>A module providing an easy-to-use intuitive search interface for your Drupal Database. With just a single search box and advanced autocomplete you can get the targeted list of entities (users or nodes) within several seconds. You can also save your searches to have quick access to commonly used lists. <strong>No configuration needed!</strong></p>

    <p>Features include:</p>

    <ul>
    <li>  Search</li>
    <li>  Smart Autocomplete (query building tool)</li>
    <li>  Save/Load Searches</li>
    <li>  Multiple Visualisation Mechamisms &amp; Extendability</li>
    <li>  Multiple Searches on the same page (Window-based system)</li>
    </ul><h2>Search</h2>

    <p>Use Refine Search for filtering the list according to the given criteria.</p>';
    return t($output);
  }
}

/**
 * Implementes hook_permission()
 * 
 */
function visualscience_permission() {
  return array(
    'access visualscience' => array('title' => t('Access VisualScience page')), 
    'administer visualscience' => array('title' => t('Administer VisualScience settings')),
    );
}
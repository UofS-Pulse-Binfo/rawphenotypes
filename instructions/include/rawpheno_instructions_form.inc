<?php

/**
 * @file
 * implements hook form in phenotypes/instructions page
 */
 
/**
 * menu callback for rawpheno instructions
 *
 * @param type $form
 * @param type $form_state
 * @return type
 */
function rawpheno_instructions($form, &$form_state) {
  //apply template
  $form['instructions'] = array(
    '#theme' => 'instructions_template',
  );
  
  //search field with autocomplete feature
  $form['instructions']['search'] = array(
    '#title' => '',
    '#type' => 'textfield',
    '#maxlength' => 65,
    '#size' => 50,
    '#default_value' => t('Search Trait'),
    '#autocomplete_path' => 'autocomplete',
  );
  
  //attach required css and js
  $form['#attached']['css'] = array(drupal_get_path('module', 'rawpheno') . '/instructions/css/rawpheno_instructions_style.css');
  $form['#attached']['js'] = array(drupal_get_path('module', 'rawpheno') . '/instructions/js/rawpheno_instructions_script.js');
  
  //attach jquery ui libraries
  $form['#attached']['library'][] = array('system', 'ui.tabs');
   
  return $form;
}

/**
 * instructions hook theme
 *
 * @param $existing
 * @param $type
 * @oaran $theme
 * @param $path
 * @return array
 */
function rawpheno_theme($existing, $type, $theme, $path) {
  return array( 
    'instructions_template' => array(
      'render element' => 'form',
      'template' => 'rawpheno_instructions',
      'path' => $path . '/instructions/theme',
    ),
  );
}

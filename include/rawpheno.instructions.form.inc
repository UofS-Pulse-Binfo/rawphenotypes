<?php

/**
 * Page callback: display standard phenotyping instructions page.
 */
function rawpheno_instructions($form, &$form_state) {
  // Navigation button. Related page of instructions page is upload page.
  $form['page_button'] = array(
    '#type' => 'markup',
    '#markup' => t('Upload Data <span>&#10095;</span>'),
  );
  
  // Search field with autocomplete feature.
  $form['txt_search'] = array(
    '#title' => '',
    '#type' => 'textfield',
    '#maxlength' => 65,
    '#size' => 65,
    '#default_value' => t('Search Trait'),
    '#autocomplete_path' => 'autocomplete',
  );
  
  $form['btn_search'] = array(
    '#type' => 'markup',
    '#markup' => '<input type="button" id="btn_submit" name="btn_search" class="form-submit" value=" ">',
  );

  // Hidden field containing url to json.
  $form['json_url'] = array(
    '#type' => 'hidden',
    '#value' => $GLOBALS['base_url'] . '/autocomplete',
    '#attributes' => array('id' => array('traits-json'))
  );  
  
  // Attach CSS and JavaScript.
  $path = drupal_get_path('module', 'rawpheno') . '/theme/';
  $form['#attached']['css'] = array($path . 'css/rawpheno.instructions.style.css');
  $form['#attached']['js']  = array($path . 'js/rawpheno.instructions.script.js');
  
  // Attach JQuery UI library.
  $form['#attached']['library'][] = array('system', 'ui.tabs');
 
  return $form;
}
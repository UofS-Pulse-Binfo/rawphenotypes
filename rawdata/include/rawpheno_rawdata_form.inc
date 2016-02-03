<?php

/**
 * @file
 * Implements hook_form().
 */
 
/**
 * Function callback for rawpheno rawdata.
 *
 * @param type $form
 * @param type $form_state
 * @return type
 */
function rawpheno_rawdata($form, &$form_state) {
  //Container for svg canvas.
  $form['rawdata'] = array(
    '#markup' => '<div id="rawdata-container">
                    <svg class="data-chart"></svg>
                  </div>',    
  );
  
  //Hidden field containing url to json.
  $form['json_url'] = array(
    '#type' => 'hidden',
    '#value' => $GLOBALS['base_url'] . '/rawdata',
    '#attributes' => array('id' => array('rawdata-json'))
  );
  
  //Attach CSS and JavaScript.
  $form['#attached']['css'] = array(drupal_get_path('module', 'rawpheno') . '/rawdata/css/rawpheno_rawdata_style.css');
  $form['#attached']['js'] = array(drupal_get_path('module', 'rawpheno') . '/rawdata/js/rawpheno_rawdata_script.js');
  
  //Attach d3 JS library.
  $form['#attached']['libraries_load'][] = array('d3');
  
  return $form;
}